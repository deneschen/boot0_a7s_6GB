#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sunxi_fip.h>

#define FIP_SIZE 512U
#define FIP_HEADER_SIZE 16U
#define FIP_ENTRY_SIZE 40U
#define TEST_FDT_SIZE 64U

struct copy_call {
	unsigned int destination;
	const void *source;
	size_t size;
};

struct copy_log {
	struct copy_call calls[4];
	unsigned int count;
	unsigned int fail_on_call;
};

struct read_call {
	unsigned int start_sector;
	unsigned int sector_count;
	void *destination;
};

struct fake_disk {
	unsigned char *data;
	size_t size;
	struct read_call calls[4];
	unsigned int count;
	unsigned int fail_on_call;
};

static const unsigned char scp_bl2_uuid[16] = {
	0x97, 0x66, 0xfd, 0x3d, 0x89, 0xbe, 0xe8, 0x49,
	0xae, 0x5d, 0x78, 0xa1, 0x40, 0x60, 0x82, 0x13,
};

static const unsigned char bl31_uuid[16] = {
	0x47, 0xd4, 0x08, 0x6d, 0x4c, 0xfe, 0x98, 0x46,
	0x9b, 0x95, 0x29, 0x50, 0xcb, 0xbd, 0x5a, 0x00,
};

static const unsigned char bl33_uuid[16] = {
	0xd6, 0xd0, 0xee, 0xa7, 0xfc, 0xea, 0xd5, 0x4b,
	0x97, 0x82, 0x99, 0x34, 0xf2, 0x34, 0xb6, 0xe4,
};

static const unsigned char hw_config_uuid[16] = {
	0x08, 0xb8, 0xf1, 0xd9, 0xc9, 0xcf, 0x93, 0x49,
	0xa9, 0x62, 0x6f, 0xbc, 0x6b, 0x72, 0x65, 0xcc,
};

static void put_le32(unsigned char *dst, unsigned int value)
{
	dst[0] = value;
	dst[1] = value >> 8;
	dst[2] = value >> 16;
	dst[3] = value >> 24;
}

static void put_le64(unsigned char *dst, unsigned long long value)
{
	unsigned int i;

	for (i = 0; i < 8; i++)
		dst[i] = value >> (i * 8);
}

static void put_be32(unsigned char *dst, unsigned int value)
{
	dst[0] = value >> 24;
	dst[1] = value >> 16;
	dst[2] = value >> 8;
	dst[3] = value;
}

static void put_valid_fdt(unsigned char *fdt, size_t size)
{
	memset(fdt, 0, size);
	put_be32(fdt, 0xd00dfeedU);
	put_be32(fdt + 4, (unsigned int)size);
	put_be32(fdt + 8, 56);
	put_be32(fdt + 12, 60);
	put_be32(fdt + 16, 40);
	put_be32(fdt + 20, 17);
	put_be32(fdt + 24, 16);
	put_be32(fdt + 32, 1);
	put_be32(fdt + 36, 4);
	put_be32(fdt + 56, 9); /* FDT_END */
}

static void put_entry(unsigned char *entry, const unsigned char uuid[16],
		      unsigned long long offset, unsigned long long size)
{
	memcpy(entry, uuid, 16);
	put_le64(entry + 16, offset);
	put_le64(entry + 24, size);
}

static void make_valid_fip(unsigned char image[FIP_SIZE])
{
	memset(image, 0, FIP_SIZE);
	put_le32(image, 0xaa640001U);
	put_le32(image + 4, 0x12345678U);
	put_entry(image + FIP_HEADER_SIZE, scp_bl2_uuid, 256, 4);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE, bl31_uuid, 320, 5);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 2, bl33_uuid, 384, 6);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 3,
		  hw_config_uuid, 448, TEST_FDT_SIZE);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 16, FIP_SIZE);
	put_valid_fdt(image + 448, TEST_FDT_SIZE);
}

static unsigned char *make_sized_fip(size_t scp_size, size_t bl31_size,
				    size_t bl33_size, size_t hw_config_size,
				    size_t *image_size)
{
	size_t scp_offset = 512;
	size_t bl31_offset = (scp_offset + scp_size + 511) & ~(size_t)511;
	size_t bl33_offset = (bl31_offset + bl31_size + 511) & ~(size_t)511;
	size_t hw_config_offset = (bl33_offset + bl33_size + 511) & ~(size_t)511;
	size_t fip_size = (hw_config_offset + hw_config_size + 511) & ~(size_t)511;
	unsigned char *image = calloc(1, fip_size);

	if (!image) {
		fprintf(stderr, "cannot allocate sized FIP\n");
		exit(EXIT_FAILURE);
	}
	put_le32(image, 0xaa640001U);
	put_le32(image + 4, 0x12345678U);
	put_entry(image + FIP_HEADER_SIZE, scp_bl2_uuid, scp_offset, scp_size);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE,
		  bl31_uuid, bl31_offset, bl31_size);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 2,
		  bl33_uuid, bl33_offset, bl33_size);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 3,
		  hw_config_uuid, hw_config_offset, hw_config_size);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 16, fip_size);
	if (hw_config_size >= TEST_FDT_SIZE)
		put_valid_fdt(image + hw_config_offset, hw_config_size);
	*image_size = fip_size;
	return image;
}

static void require_image(const char *name, const struct sunxi_fip_image *image,
			  size_t offset, size_t size)
{
	if (image->offset != offset || image->size != size) {
		fprintf(stderr, "%s: got offset=%zu size=%zu, expected %zu/%zu\n",
			name, image->offset, image->size, offset, size);
		exit(EXIT_FAILURE);
	}
}

static void require_rejected(const char *name, const unsigned char *image,
			     size_t image_size)
{
	struct sunxi_fip_layout layout;

	if (sunxi_fip_parse(image, image_size, &layout) == 0) {
		fprintf(stderr, "%s: malformed FIP was accepted\n", name);
		exit(EXIT_FAILURE);
	}
}

static int verify_fip_file(const char *path)
{
	struct sunxi_fip_layout layout;
	unsigned char *image;
	long file_size;
	FILE *file;
	int status = EXIT_FAILURE;

	file = fopen(path, "rb");
	if (!file || fseek(file, 0, SEEK_END) != 0 ||
	    (file_size = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0) {
		fprintf(stderr, "cannot open FIP: %s\n", path);
		if (file)
			fclose(file);
		return EXIT_FAILURE;
	}

	image = malloc((size_t)file_size);
	if (!image || fread(image, 1, (size_t)file_size, file) != (size_t)file_size) {
		fprintf(stderr, "cannot read FIP: %s\n", path);
		goto out;
	}
	if (sunxi_fip_parse(image, (size_t)file_size, &layout) != 0) {
		fprintf(stderr, "fiptool image was rejected: %s\n", path);
		goto out;
	}

	printf("SCP_BL2=%zu/%zu BL31=%zu/%zu BL33=%zu/%zu HW_CONFIG=%zu/%zu\n",
	       layout.scp_bl2.offset, layout.scp_bl2.size,
	       layout.bl31.offset, layout.bl31.size,
	       layout.bl33.offset, layout.bl33.size,
	       layout.hw_config.offset, layout.hw_config.size);
	status = EXIT_SUCCESS;
out:
	free(image);
	fclose(file);
	return status;
}

static int record_copy(unsigned int destination, const void *source,
		       size_t size, void *context)
{
	struct copy_log *log = context;

	if (log->count >= 4)
		return -1;
	log->calls[log->count].destination = destination;
	log->calls[log->count].source = source;
	log->calls[log->count].size = size;
	log->count++;
	if (log->fail_on_call == log->count)
		return -1;
	return 0;
}

static void require_copy(const struct copy_log *log, unsigned int index,
			 unsigned int destination, const void *source, size_t size)
{
	const struct copy_call *call = &log->calls[index];

	if (call->destination != destination || call->source != source ||
	    call->size != size) {
		fprintf(stderr, "copy %u: got %08x/%p/%zu, expected %08x/%p/%zu\n",
			index, call->destination, call->source, call->size,
			destination, source, size);
		exit(EXIT_FAILURE);
	}
}

static int fake_read(unsigned int start_sector, unsigned int sector_count,
		     void *destination, void *context)
{
	struct fake_disk *disk = context;
	size_t offset = (size_t)start_sector * 512;
	size_t size = (size_t)sector_count * 512;

	if (disk->count >= 4 || offset > disk->size || size > disk->size - offset)
		return 0;
	disk->calls[disk->count].start_sector = start_sector;
	disk->calls[disk->count].sector_count = sector_count;
	disk->calls[disk->count].destination = destination;
	disk->count++;
	if (disk->fail_on_call == disk->count)
		return 0;
	memcpy(destination, disk->data + offset, size);
	return 1;
}

int main(int argc, char **argv)
{
	unsigned char image[FIP_SIZE];
	struct sunxi_fip_layout layout;

	if (argc == 2)
		return verify_fip_file(argv[1]);
	if (argc != 1) {
		fprintf(stderr, "usage: %s [fip.bin]\n", argv[0]);
		return EXIT_FAILURE;
	}

	make_valid_fip(image);
	if (sunxi_fip_parse(image, sizeof(image), &layout) != 0) {
		fprintf(stderr, "valid FIP was rejected\n");
		return EXIT_FAILURE;
	}

	require_image("SCP_BL2", &layout.scp_bl2, 256, 4);
	require_image("BL31", &layout.bl31, 320, 5);
	require_image("BL33", &layout.bl33, 384, 6);
	require_image("HW_CONFIG", &layout.hw_config, 448, TEST_FDT_SIZE);

	make_valid_fip(image);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4,
		  bl31_uuid, 400, 4);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 5 + 16,
		 FIP_SIZE);
	require_rejected("duplicate BL31", image, sizeof(image));

	make_valid_fip(image);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE,
		  bl31_uuid, 500, 20);
	require_rejected("BL31 outside FIP buffer", image, sizeof(image));

	make_valid_fip(image);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 16, 389);
	require_rejected("payload beyond FIP end marker", image, sizeof(image));

	make_valid_fip(image);
	put_entry(image + FIP_HEADER_SIZE, scp_bl2_uuid, 100, 4);
	require_rejected("payload overlaps FIP metadata", image, sizeof(image));

	make_valid_fip(image);
	put_entry(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE,
		  bl31_uuid, 258, 5);
	require_rejected("FIP payloads overlap", image, sizeof(image));

	make_valid_fip(image);
	put_le32(image, 0x89119800U);
	require_rejected("TOC1 magic used as FIP", image, sizeof(image));

	make_valid_fip(image);
	image[FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 2] ^= 0xff;
	require_rejected("missing BL33", image, sizeof(image));

	make_valid_fip(image);
	image[FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 3] ^= 0xff;
	if (sunxi_fip_parse(image, sizeof(image), &layout) != 0 ||
	    layout.hw_config.size != 0) {
		fprintf(stderr, "FIP without optional HW_CONFIG was rejected\n");
		return EXIT_FAILURE;
	}
	{
		struct copy_log log = { 0 };

		if (sunxi_fip_copy_images(image, sizeof(image), record_copy, &log) != 0 ||
		    log.count != 3) {
			fprintf(stderr, "FIP without HW_CONFIG was not copied safely\n");
			return EXIT_FAILURE;
		}
		require_copy(&log, 0, SUNXI_FIP_BL31_BASE, image + 320, 5);
		require_copy(&log, 1, SUNXI_FIP_BL33_BASE, image + 384, 6);
		require_copy(&log, 2, SUNXI_FIP_SCP_BL2_BASE, image + 256, 4);
	}

	make_valid_fip(image);
	image[448] = 0;
	require_rejected("invalid HW_CONFIG FDT", image, sizeof(image));

	make_valid_fip(image);
	image[FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4] = 0x5a;
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 16, 400);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 24, 1);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 5 + 16,
		 FIP_SIZE);
	if (sunxi_fip_parse(image, sizeof(image), &layout) != 0) {
		fprintf(stderr, "FIP with an unknown optional entry was rejected\n");
		return EXIT_FAILURE;
	}

	make_valid_fip(image);
	image[FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4] = 0x5a;
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 16, 400);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 24, 1);
	image[FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 5] = 0x5a;
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 5 + 16, 402);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 5 + 24, 1);
	put_le64(image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 6 + 16,
		 FIP_SIZE);
	require_rejected("duplicate unknown UUID", image, sizeof(image));

	{
		unsigned char large_image[2048] = { 0 };
		unsigned int i;

		put_le32(large_image, 0xaa640001U);
		put_le32(large_image + 4, 0x12345678U);
		put_entry(large_image + FIP_HEADER_SIZE, scp_bl2_uuid, 1536, 4);
		put_entry(large_image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE,
			  bl31_uuid, 1540, 5);
		put_entry(large_image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 2,
			  bl33_uuid, 1545, 6);
		put_entry(large_image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 3,
			  hw_config_uuid, 1552, TEST_FDT_SIZE);
		put_valid_fdt(large_image + 1552, TEST_FDT_SIZE);
		for (i = 4; i < 32; i++) {
			unsigned char *entry = large_image + FIP_HEADER_SIZE +
					       FIP_ENTRY_SIZE * i;

			entry[0] = 0xa5;
			entry[1] = i;
			put_le64(entry + 16, 1640 + (i - 4) * 2);
			put_le64(entry + 24, 1);
		}
		put_le64(large_image + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 32 + 16,
			 sizeof(large_image));
		if (sunxi_fip_parse(large_image, sizeof(large_image), &layout) != 0) {
			fprintf(stderr, "32-entry standard FIP was rejected\n");
			return EXIT_FAILURE;
		}
	}

	{
		struct copy_log log = { 0 };

		make_valid_fip(image);
		if (sunxi_fip_copy_images(image, sizeof(image), record_copy, &log) != 0) {
			fprintf(stderr, "valid FIP images were not copied\n");
			return EXIT_FAILURE;
		}
		if (log.count != 4) {
			fprintf(stderr, "got %u copy calls, expected 4\n", log.count);
			return EXIT_FAILURE;
		}
		require_copy(&log, 0, SUNXI_FIP_BL31_BASE, image + 320, 5);
		require_copy(&log, 1, SUNXI_FIP_BL33_BASE, image + 384, 6);
		require_copy(&log, 2, SUNXI_FIP_HW_CONFIG_BASE, image + 448,
			     TEST_FDT_SIZE);
		require_copy(&log, 3, SUNXI_FIP_SCP_BL2_BASE, image + 256, 4);
	}

	{
		struct copy_log log = { .fail_on_call = 2 };

		make_valid_fip(image);
		if (sunxi_fip_copy_images(image, sizeof(image), record_copy, &log) == 0) {
			fprintf(stderr, "copy callback failure was ignored\n");
			return EXIT_FAILURE;
		}
		if (log.count != 2) {
			fprintf(stderr, "copy continued after callback failure\n");
			return EXIT_FAILURE;
		}
	}

	{
		struct copy_log log = { 0 };
		size_t sized_fip_size;
		unsigned char *sized_fip =
			make_sized_fip(1, SUNXI_FIP_BL31_MAX_SIZE + 1, 1,
				       TEST_FDT_SIZE,
				       &sized_fip_size);

		if (sunxi_fip_copy_images(sized_fip, sized_fip_size,
					  record_copy, &log) == 0) {
			fprintf(stderr, "oversized BL31 was copied\n");
			free(sized_fip);
			return EXIT_FAILURE;
		}
		if (log.count != 0) {
			fprintf(stderr, "copy started before BL31 size validation\n");
			free(sized_fip);
			return EXIT_FAILURE;
		}
		free(sized_fip);
	}

	{
		struct copy_log log = { 0 };
		size_t sized_fip_size;
		unsigned char *sized_fip =
			make_sized_fip(1, 1, SUNXI_FIP_BL33_MAX_SIZE + 1,
				       TEST_FDT_SIZE,
				       &sized_fip_size);

		if (sunxi_fip_copy_images(sized_fip, sized_fip_size,
					  record_copy, &log) == 0) {
			fprintf(stderr, "oversized BL33 was copied\n");
			free(sized_fip);
			return EXIT_FAILURE;
		}
		if (log.count != 0) {
			fprintf(stderr, "copy started before BL33 size validation\n");
			free(sized_fip);
			return EXIT_FAILURE;
		}
		free(sized_fip);
	}

	{
		struct copy_log log = { 0 };
		size_t sized_fip_size;
		unsigned char *sized_fip =
			make_sized_fip(SUNXI_FIP_SCP_BL2_MAX_SIZE + 1, 1, 1,
				       TEST_FDT_SIZE,
				       &sized_fip_size);

		if (sunxi_fip_copy_images(sized_fip, sized_fip_size,
					  record_copy, &log) == 0) {
			fprintf(stderr, "oversized SCP_BL2 was copied\n");
			free(sized_fip);
			return EXIT_FAILURE;
		}
		if (log.count != 0) {
			fprintf(stderr, "copy started before SCP_BL2 size validation\n");
			free(sized_fip);
			return EXIT_FAILURE;
		}
		free(sized_fip);
	}

	{
		struct copy_log log = { 0 };
		size_t sized_fip_size;
		unsigned char *sized_fip =
			make_sized_fip(1, 1, 1,
				       SUNXI_FIP_HW_CONFIG_MAX_SIZE + 1,
				       &sized_fip_size);

		if (sunxi_fip_copy_images(sized_fip, sized_fip_size,
					  record_copy, &log) == 0 || log.count != 0) {
			fprintf(stderr, "oversized HW_CONFIG was copied\n");
			free(sized_fip);
			return EXIT_FAILURE;
		}
		free(sized_fip);
	}

	{
		struct copy_log log = { 0 };
		size_t sized_fip_size;
		unsigned char *sized_fip =
			make_sized_fip(0x1000, 0x40000, 0x80000,
				       TEST_FDT_SIZE, &sized_fip_size);

		if (sunxi_fip_copy_images(sized_fip, sized_fip_size,
					  record_copy, &log) != 0 || log.count != 4) {
			fprintf(stderr, "valid multi-image FIP was rejected\n");
			free(sized_fip);
			return EXIT_FAILURE;
		}
		free(sized_fip);
	}

	{
		unsigned char disk_data[4096] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 0;

		make_valid_fip(disk_data + 4 * 512);
		if (sunxi_fip_read_image(4, staging, sizeof(staging), fake_read,
					 &disk, &loaded_size) != 0) {
			fprintf(stderr, "valid FIP could not be read\n");
			return EXIT_FAILURE;
		}
		if (disk.count != 2 ||
		    disk.calls[0].start_sector != 4 ||
		    disk.calls[0].sector_count != 3 ||
		    disk.calls[0].destination != staging ||
		    disk.calls[1].start_sector != 4 ||
		    disk.calls[1].sector_count != 1 ||
		    disk.calls[1].destination != staging ||
		    loaded_size != FIP_SIZE ||
		    memcmp(staging, disk_data + 4 * 512, FIP_SIZE) != 0) {
			fprintf(stderr, "FIP staged-read contract was not followed\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[2048] = { 0 };
		unsigned char staging[512];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 123;

		if (sunxi_fip_read_image(0, staging, sizeof(staging), fake_read,
					 &disk, &loaded_size) == 0 ||
		    disk.count != 0 || loaded_size != 0) {
			fprintf(stderr, "undersized staging buffer was not rejected safely\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[2048] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
			.fail_on_call = 1,
		};
		size_t loaded_size = 123;

		if (sunxi_fip_read_image(0, staging, sizeof(staging), fake_read,
					 &disk, &loaded_size) == 0 ||
		    disk.count != 1 || loaded_size != 0) {
			fprintf(stderr, "initial FIP read failure was not propagated\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[2048] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
			.fail_on_call = 2,
		};
		size_t loaded_size = 123;

		make_valid_fip(disk_data);
		if (sunxi_fip_read_image(0, staging, sizeof(staging), fake_read,
					 &disk, &loaded_size) == 0 ||
		    disk.count != 2 || loaded_size != 0) {
			fprintf(stderr, "complete FIP read failure was not propagated\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[4096] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 0;

		make_valid_fip(disk_data + 512);
		put_le64(disk_data + 512 + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 16,
			 513);
		if (sunxi_fip_read_image(1, staging, sizeof(staging), fake_read,
					 &disk, &loaded_size) != 0 ||
		    disk.count != 2 || disk.calls[1].sector_count != 2 ||
		    loaded_size != 513) {
			fprintf(stderr, "unaligned FIP size was not read by sector ceiling\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[2048] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 123;

		make_valid_fip(disk_data);
		disk_data[FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 2] ^= 0xff;
		if (sunxi_fip_read_image(0, staging, sizeof(staging), fake_read,
					 &disk, &loaded_size) == 0 ||
		    disk.count != 2 || loaded_size != 0) {
			fprintf(stderr, "invalid complete FIP was accepted after reading\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[2048] = { 0 };
		unsigned char *staging = malloc(0x200200);
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 123;

		if (!staging) {
			fprintf(stderr, "cannot allocate staging buffer\n");
			return EXIT_FAILURE;
		}
		make_valid_fip(disk_data);
		put_le64(disk_data + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 16,
			 SUNXI_FIP_MAX_SIZE + 1);
		if (sunxi_fip_read_image(0, staging, 0x200200, fake_read,
					 &disk, &loaded_size) == 0 ||
		    disk.count != 1 || loaded_size != 0) {
			fprintf(stderr, "FIP exceeded the staging region\n");
			free(staging);
			return EXIT_FAILURE;
		}
		free(staging);
	}

	{
		unsigned char *disk_data = calloc(1, SUNXI_FIP_MAX_SIZE);
		unsigned char *staging = malloc(SUNXI_FIP_MAX_SIZE);
		struct fake_disk disk = {
			.data = disk_data,
			.size = SUNXI_FIP_MAX_SIZE,
		};
		size_t loaded_size = 0;

		if (!disk_data || !staging) {
			fprintf(stderr, "cannot allocate maximum FIP buffers\n");
			free(staging);
			free(disk_data);
			return EXIT_FAILURE;
		}
		make_valid_fip(disk_data);
		put_le64(disk_data + FIP_HEADER_SIZE + FIP_ENTRY_SIZE * 4 + 16,
			 SUNXI_FIP_MAX_SIZE);
		if (sunxi_fip_read_image(0, staging, SUNXI_FIP_MAX_SIZE, fake_read,
					 &disk, &loaded_size) != 0 ||
		    disk.count != 2 || disk.calls[1].sector_count != 4096 ||
		    loaded_size != SUNXI_FIP_MAX_SIZE) {
			fprintf(stderr, "maximum-sized FIP was not read\n");
			free(staging);
			free(disk_data);
			return EXIT_FAILURE;
		}
		free(staging);
		free(disk_data);
	}

	{
		unsigned char disk_data[8192] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 0;
		unsigned int used_sector = 0;

		make_valid_fip(disk_data + 512);
		if (sunxi_fip_read_redundant(1, 8, staging, sizeof(staging),
					     fake_read, &disk, &loaded_size,
					     &used_sector) != 0 ||
		    disk.count != 2 || disk.calls[0].start_sector != 1 ||
		    disk.calls[1].start_sector != 1 || loaded_size != FIP_SIZE ||
		    used_sector != 1) {
			fprintf(stderr, "valid primary FIP did not suppress backup read\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[8192] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 0;
		unsigned int used_sector = 0;

		make_valid_fip(disk_data + 8 * 512);
		if (sunxi_fip_read_redundant(1, 8, staging, sizeof(staging),
					     fake_read, &disk, &loaded_size,
					     &used_sector) != 0 ||
		    disk.count != 3 || disk.calls[0].start_sector != 1 ||
		    disk.calls[1].start_sector != 8 ||
		    disk.calls[2].start_sector != 8 || loaded_size != FIP_SIZE ||
		    used_sector != 8) {
			fprintf(stderr, "valid backup FIP was not selected\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[8192] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
			.fail_on_call = 2,
		};
		size_t loaded_size = 0;
		unsigned int used_sector = 0;

		make_valid_fip(disk_data + 1 * 512);
		make_valid_fip(disk_data + 8 * 512);
		if (sunxi_fip_read_redundant(1, 8, staging, sizeof(staging),
					     fake_read, &disk, &loaded_size,
					     &used_sector) != 0 ||
		    disk.count != 4 || disk.calls[2].start_sector != 8 ||
		    disk.calls[3].start_sector != 8 || loaded_size != FIP_SIZE ||
		    used_sector != 8) {
			fprintf(stderr, "backup did not recover a complete-read failure\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[2048] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 123;
		unsigned int used_sector = 456;

		if (sunxi_fip_read_redundant(1, 0, staging, sizeof(staging),
					     fake_read, &disk, &loaded_size,
					     &used_sector) == 0 ||
		    disk.count != 1 || loaded_size != 0 || used_sector != 0) {
			fprintf(stderr, "zero backup sector triggered a retry\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[2048] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 123;
		unsigned int used_sector = 456;

		if (sunxi_fip_read_redundant(1, 1, staging, sizeof(staging),
					     fake_read, &disk, &loaded_size,
					     &used_sector) == 0 ||
		    disk.count != 1 || loaded_size != 0 || used_sector != 0) {
			fprintf(stderr, "duplicate backup sector was read twice\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[8192] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		size_t loaded_size = 123;
		unsigned int used_sector = 456;

		if (sunxi_fip_read_redundant(1, 8, staging, sizeof(staging),
					     fake_read, &disk, &loaded_size,
					     &used_sector) == 0 ||
		    disk.count != 2 || disk.calls[0].start_sector != 1 ||
		    disk.calls[1].start_sector != 8 ||
		    loaded_size != 0 || used_sector != 0) {
			fprintf(stderr, "invalid redundant FIPs were not rejected safely\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[8192] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		struct copy_log log = { 0 };
		size_t loaded_size = 0;
		unsigned int used_sector = 0;

		make_valid_fip(disk_data + 512);
		if (sunxi_fip_load_redundant(1, 8, staging, sizeof(staging),
					     fake_read, &disk, record_copy, &log,
					     &loaded_size, &used_sector) != 0 ||
		    disk.count != 2 || log.count != 4 ||
		    loaded_size != FIP_SIZE || used_sector != 1) {
			fprintf(stderr, "valid redundant FIP was not loaded\n");
			return EXIT_FAILURE;
		}
		require_copy(&log, 0, SUNXI_FIP_BL31_BASE, staging + 320, 5);
		require_copy(&log, 1, SUNXI_FIP_BL33_BASE, staging + 384, 6);
		require_copy(&log, 2, SUNXI_FIP_HW_CONFIG_BASE, staging + 448,
			     TEST_FDT_SIZE);
		require_copy(&log, 3, SUNXI_FIP_SCP_BL2_BASE, staging + 256, 4);
	}

	{
		const unsigned int backup_sector = 3000;
		unsigned char *disk_data = calloc(1, SUNXI_FIP_MAX_SIZE);
		unsigned char *staging = malloc(SUNXI_FIP_MAX_SIZE);
		struct fake_disk disk = {
			.data = disk_data,
			.size = SUNXI_FIP_MAX_SIZE,
		};
		struct copy_log log = { 0 };
		size_t oversized_size;
		unsigned char *oversized =
			make_sized_fip(1, SUNXI_FIP_BL31_MAX_SIZE + 1, 1,
				       TEST_FDT_SIZE,
				       &oversized_size);
		size_t loaded_size = 0;
		unsigned int used_sector = 0;

		if (!disk_data || !staging) {
			fprintf(stderr, "cannot allocate failover FIP buffers\n");
			free(oversized);
			free(staging);
			free(disk_data);
			return EXIT_FAILURE;
		}
		memcpy(disk_data + 512, oversized, oversized_size);
		make_valid_fip(disk_data + backup_sector * 512);
		if (sunxi_fip_load_redundant(1, backup_sector, staging,
					     SUNXI_FIP_MAX_SIZE, fake_read, &disk,
					     record_copy, &log, &loaded_size,
					     &used_sector) != 0 ||
		    disk.count != 4 || log.count != 4 ||
		    loaded_size != FIP_SIZE || used_sector != backup_sector) {
			fprintf(stderr,
				"oversized primary FIP did not fall back to backup\n");
			free(oversized);
			free(staging);
			free(disk_data);
			return EXIT_FAILURE;
		}
		free(oversized);
		free(staging);
		free(disk_data);
	}

	{
		unsigned char disk_data[8192] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		struct copy_log log = { .fail_on_call = 2 };
		size_t loaded_size = 123;
		unsigned int used_sector = 456;

		make_valid_fip(disk_data + 512);
		make_valid_fip(disk_data + 8 * 512);
		if (sunxi_fip_load_redundant(1, 8, staging, sizeof(staging),
					     fake_read, &disk, record_copy, &log,
					     &loaded_size, &used_sector) == 0 ||
		    disk.count != 2 || log.count != 2 ||
		    loaded_size != 0 || used_sector != 0) {
			fprintf(stderr, "copy failure retried or exposed a loaded FIP\n");
			return EXIT_FAILURE;
		}
	}

	{
		unsigned char disk_data[8192] = { 0 };
		unsigned char staging[1536];
		struct fake_disk disk = {
			.data = disk_data,
			.size = sizeof(disk_data),
		};
		struct copy_log log = { 0 };
		size_t loaded_size = 123;
		unsigned int used_sector = 456;

		if (sunxi_fip_load_redundant(1, 8, staging, sizeof(staging),
					     fake_read, &disk, record_copy, &log,
					     &loaded_size, &used_sector) == 0 ||
		    disk.count != 2 || log.count != 0 ||
		    loaded_size != 0 || used_sector != 0) {
			fprintf(stderr, "invalid FIPs reached the copy callback\n");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
