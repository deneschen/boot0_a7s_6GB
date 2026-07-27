#include <sunxi_fip.h>

#define FIP_TOC_HEADER_NAME 0xaa640001U
#define FIP_TOC_SERIAL_NUMBER 0x12345678U
#define FIP_HEADER_SIZE 16U
#define FIP_ENTRY_SIZE 40U
#define FIP_SECTOR_SIZE 512U
#define FIP_TOC_SECTORS 3U
#define FIP_METADATA_SIZE (FIP_TOC_SECTORS * FIP_SECTOR_SIZE)
#define FIP_MAX_TOC_ENTRIES \
	((FIP_METADATA_SIZE - FIP_HEADER_SIZE) / FIP_ENTRY_SIZE)

#define FIP_FOUND_SCP_BL2 0x1U
#define FIP_FOUND_BL31 0x2U
#define FIP_FOUND_BL33 0x4U
#define FIP_FOUND_REQUIRED (FIP_FOUND_SCP_BL2 | FIP_FOUND_BL31 | \
			    FIP_FOUND_BL33)

static const u8 scp_bl2_uuid[16] = {
	0x97, 0x66, 0xfd, 0x3d, 0x89, 0xbe, 0xe8, 0x49,
	0xae, 0x5d, 0x78, 0xa1, 0x40, 0x60, 0x82, 0x13,
};

static const u8 bl31_uuid[16] = {
	0x47, 0xd4, 0x08, 0x6d, 0x4c, 0xfe, 0x98, 0x46,
	0x9b, 0x95, 0x29, 0x50, 0xcb, 0xbd, 0x5a, 0x00,
};

static const u8 bl33_uuid[16] = {
	0xd6, 0xd0, 0xee, 0xa7, 0xfc, 0xea, 0xd5, 0x4b,
	0x97, 0x82, 0x99, 0x34, 0xf2, 0x34, 0xb6, 0xe4,
};

static u32 get_le32(const u8 *src)
{
	return (u32)src[0] | ((u32)src[1] << 8) |
	       ((u32)src[2] << 16) | ((u32)src[3] << 24);
}

static u64 get_le64(const u8 *src)
{
	u64 value = 0;
	unsigned int i;

	for (i = 0; i < 8; i++)
		value |= (u64)src[i] << (i * 8);
	return value;
}

static int uuid_equal(const u8 *left, const u8 *right)
{
	unsigned int i;

	for (i = 0; i < 16; i++) {
		if (left[i] != right[i])
			return 0;
	}
	return 1;
}

static int uuid_is_null(const u8 *uuid)
{
	static const u8 null_uuid[16];

	return uuid_equal(uuid, null_uuid);
}

static void set_image(struct sunxi_fip_image *image, const u8 *entry)
{
	image->offset = (size_t)get_le64(entry + 16);
	image->size = (size_t)get_le64(entry + 24);
}

static int entry_in_buffer(const u8 *entry, size_t image_size)
{
	u64 offset = get_le64(entry + 16);
	u64 size = get_le64(entry + 24);
	u64 buffer_size = image_size;

	return size != 0 && offset <= buffer_size && size <= buffer_size - offset;
}

static int entry_overlaps_previous(const u8 *bytes, const u8 *entry,
				   unsigned int entry_count)
{
	u64 offset = get_le64(entry + 16);
	u64 end = offset + get_le64(entry + 24);
	unsigned int i;

	for (i = 0; i < entry_count; i++) {
		const u8 *previous = bytes + FIP_HEADER_SIZE + i * FIP_ENTRY_SIZE;
		u64 previous_offset = get_le64(previous + 16);
		u64 previous_end = previous_offset + get_le64(previous + 24);

		if (offset < previous_end && previous_offset < end)
			return 1;
	}
	return 0;
}

static int entry_uuid_seen(const u8 *bytes, const u8 *entry,
			   unsigned int entry_count)
{
	unsigned int i;

	for (i = 0; i < entry_count; i++) {
		const u8 *previous = bytes + FIP_HEADER_SIZE + i * FIP_ENTRY_SIZE;

		if (uuid_equal(previous, entry))
			return 1;
	}
	return 0;
}

int sunxi_fip_parse(const void *image, size_t image_size,
		    struct sunxi_fip_layout *layout)
{
	const u8 *bytes = image;
	size_t entry_offset;
	unsigned int entry_count;
	unsigned int found = 0;
	u64 payload_start = ~0ULL;
	u64 payload_end = 0;

	if (!bytes || !layout || image_size < FIP_HEADER_SIZE)
		return -1;
	if (get_le32(bytes) != FIP_TOC_HEADER_NAME ||
	    get_le32(bytes + 4) != FIP_TOC_SERIAL_NUMBER)
		return -1;

	layout->scp_bl2.offset = 0;
	layout->scp_bl2.size = 0;
	layout->bl31.offset = 0;
	layout->bl31.size = 0;
	layout->bl33.offset = 0;
	layout->bl33.size = 0;

	entry_offset = FIP_HEADER_SIZE;
	for (entry_count = 0; entry_count < FIP_MAX_TOC_ENTRIES; entry_count++) {
		const u8 *entry;

		if (entry_offset > image_size ||
		    image_size - entry_offset < FIP_ENTRY_SIZE)
			return -1;
		entry = bytes + entry_offset;
		if (uuid_is_null(entry)) {
			u64 fip_size = get_le64(entry + 16);
			u64 metadata_end = entry_offset + FIP_ENTRY_SIZE;

			if (get_le64(entry + 24) != 0 ||
			    get_le64(entry + 32) != 0 ||
			    fip_size > image_size || fip_size < metadata_end ||
			    payload_start < metadata_end || payload_end > fip_size)
				return -1;
			return found == FIP_FOUND_REQUIRED ? 0 : -1;
		}
		if (entry_uuid_seen(bytes, entry, entry_count))
			return -1;
		if (!entry_in_buffer(entry, image_size))
			return -1;
		if (entry_overlaps_previous(bytes, entry, entry_count))
			return -1;
		if (get_le64(entry + 16) < payload_start)
			payload_start = get_le64(entry + 16);
		if (get_le64(entry + 16) + get_le64(entry + 24) > payload_end)
			payload_end = get_le64(entry + 16) + get_le64(entry + 24);

		if (uuid_equal(entry, scp_bl2_uuid)) {
			set_image(&layout->scp_bl2, entry);
			found |= FIP_FOUND_SCP_BL2;
		} else if (uuid_equal(entry, bl31_uuid)) {
			set_image(&layout->bl31, entry);
			found |= FIP_FOUND_BL31;
		} else if (uuid_equal(entry, bl33_uuid)) {
			set_image(&layout->bl33, entry);
			found |= FIP_FOUND_BL33;
		}

		entry_offset += FIP_ENTRY_SIZE;
	}

	return -1;
}

static int layout_is_loadable(const struct sunxi_fip_layout *layout)
{
	return layout->bl31.size <= SUNXI_FIP_BL31_MAX_SIZE &&
	       layout->bl33.size <= SUNXI_FIP_BL33_MAX_SIZE &&
	       layout->scp_bl2.size <= SUNXI_FIP_SCP_BL2_MAX_SIZE;
}

int sunxi_fip_copy_images(const void *image, size_t image_size,
			  sunxi_fip_copy_fn copy, void *context)
{
	const u8 *bytes = image;
	struct sunxi_fip_layout layout;

	if (!copy || sunxi_fip_parse(image, image_size, &layout) != 0 ||
	    !layout_is_loadable(&layout))
		return -1;
	if (copy(SUNXI_FIP_BL31_BASE, bytes + layout.bl31.offset,
		 layout.bl31.size, context) != 0)
		return -1;
	if (copy(SUNXI_FIP_BL33_BASE, bytes + layout.bl33.offset,
		 layout.bl33.size, context) != 0)
		return -1;
	if (copy(SUNXI_FIP_SCP_BL2_BASE, bytes + layout.scp_bl2.offset,
		 layout.scp_bl2.size, context) != 0)
		return -1;
	return 0;
}

static int get_fip_size_from_toc(const u8 *bytes, size_t metadata_size,
				 size_t capacity, size_t *fip_size)
{
	size_t entry_offset = FIP_HEADER_SIZE;
	unsigned int entry_count;

	if (get_le32(bytes) != FIP_TOC_HEADER_NAME ||
	    get_le32(bytes + 4) != FIP_TOC_SERIAL_NUMBER)
		return -1;

	for (entry_count = 0; entry_count < FIP_MAX_TOC_ENTRIES; entry_count++) {
		const u8 *entry;
		u64 declared_size;
		u64 rounded_size;

		if (entry_offset > metadata_size ||
		    metadata_size - entry_offset < FIP_ENTRY_SIZE)
			return -1;
		entry = bytes + entry_offset;
		if (!uuid_is_null(entry)) {
			entry_offset += FIP_ENTRY_SIZE;
			continue;
		}

		declared_size = get_le64(entry + 16);
		rounded_size = (declared_size + FIP_SECTOR_SIZE - 1) &
			       ~(u64)(FIP_SECTOR_SIZE - 1);
		if (get_le64(entry + 24) != 0 || get_le64(entry + 32) != 0 ||
		    declared_size < entry_offset + FIP_ENTRY_SIZE ||
		    declared_size > SUNXI_FIP_MAX_SIZE ||
		    rounded_size < declared_size || rounded_size > capacity)
			return -1;
		*fip_size = (size_t)declared_size;
		return 0;
	}

	return -1;
}

int sunxi_fip_read_image(u32 start_sector, void *buffer, size_t capacity,
			 sunxi_fip_read_fn read, void *context,
			 size_t *fip_size)
{
	struct sunxi_fip_layout layout;
	size_t metadata_size = FIP_METADATA_SIZE;
	size_t declared_size;
	u32 sector_count;

	if (!fip_size)
		return -1;
	*fip_size = 0;
	if (!buffer || !read || capacity < metadata_size)
		return -1;
	if (read(start_sector, FIP_TOC_SECTORS, buffer, context) != 1)
		return -1;
	if (get_fip_size_from_toc(buffer, metadata_size, capacity,
				  &declared_size) != 0)
		return -1;

	sector_count = (declared_size + FIP_SECTOR_SIZE - 1) / FIP_SECTOR_SIZE;
	if (read(start_sector, sector_count, buffer, context) != 1)
		return -1;
	if (sunxi_fip_parse(buffer, declared_size, &layout) != 0 ||
	    !layout_is_loadable(&layout))
		return -1;

	*fip_size = declared_size;
	return 0;
}

int sunxi_fip_read_redundant(u32 primary_sector, u32 backup_sector,
			     void *buffer, size_t capacity,
			     sunxi_fip_read_fn read, void *context,
			     size_t *fip_size, u32 *used_sector)
{
	if (!fip_size || !used_sector)
		return -1;
	*fip_size = 0;
	*used_sector = 0;
	if (sunxi_fip_read_image(primary_sector, buffer, capacity, read,
				 context, fip_size) == 0) {
		*used_sector = primary_sector;
		return 0;
	}
	if (!backup_sector || backup_sector == primary_sector)
		return -1;
	if (sunxi_fip_read_image(backup_sector, buffer, capacity, read,
				 context, fip_size) != 0)
		return -1;
	*used_sector = backup_sector;
	return 0;
}

int sunxi_fip_load_redundant(u32 primary_sector, u32 backup_sector,
			     void *buffer, size_t capacity,
			     sunxi_fip_read_fn read, void *read_context,
			     sunxi_fip_copy_fn copy, void *copy_context,
			     size_t *fip_size, u32 *used_sector)
{
	if (sunxi_fip_read_redundant(primary_sector, backup_sector, buffer,
				     capacity, read, read_context,
				     fip_size, used_sector) != 0)
		return -1;
	if (sunxi_fip_copy_images(buffer, *fip_size, copy, copy_context) != 0) {
		*fip_size = 0;
		*used_sector = 0;
		return -1;
	}
	return 0;
}
