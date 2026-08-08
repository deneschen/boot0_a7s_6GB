#ifndef _SUNXI_FIP_H_
#define _SUNXI_FIP_H_

#include <linux/types.h>

#define SUNXI_FIP_MAX_SIZE 0x00200000U

#define SUNXI_FIP_BL31_BASE 0x48000000U
#define SUNXI_FIP_BL31_MAX_SIZE 0x00100000U
#define SUNXI_FIP_BL33_BASE 0x4a000000U
#define SUNXI_FIP_BL33_MAX_SIZE 0x00180000U
#define SUNXI_FIP_SCP_BL2_BASE 0x00044000U
#define SUNXI_FIP_SCP_BL2_MAX_SIZE 0x00028000U
#define SUNXI_FIP_HW_CONFIG_BASE 0x48100000U
#define SUNXI_FIP_HW_CONFIG_MAX_SIZE 0x00100000U

struct sunxi_fip_image {
	size_t offset;
	size_t size;
};

struct sunxi_fip_layout {
	struct sunxi_fip_image scp_bl2;
	struct sunxi_fip_image bl31;
	struct sunxi_fip_image bl33;
	struct sunxi_fip_image hw_config;
};

typedef int (*sunxi_fip_copy_fn)(u32 destination, const void *source,
				 size_t size, void *context);
/* Return 1 on success, any other value on failure. */
typedef int (*sunxi_fip_read_fn)(u32 start_sector, u32 sector_count,
				 void *destination, void *context);

int sunxi_fip_parse(const void *image, size_t image_size,
		    struct sunxi_fip_layout *layout);
int sunxi_fip_copy_images(const void *image, size_t image_size,
			  sunxi_fip_copy_fn copy, void *context);
int sunxi_fip_read_image(u32 start_sector, void *buffer, size_t capacity,
			 sunxi_fip_read_fn read, void *context,
			 size_t *fip_size);
int sunxi_fip_read_redundant(u32 primary_sector, u32 backup_sector,
			     void *buffer, size_t capacity,
			     sunxi_fip_read_fn read, void *context,
			     size_t *fip_size, u32 *used_sector);
int sunxi_fip_load_redundant(u32 primary_sector, u32 backup_sector,
			     void *buffer, size_t capacity,
			     sunxi_fip_read_fn read, void *read_context,
			     sunxi_fip_copy_fn copy, void *copy_context,
			     size_t *fip_size, u32 *used_sector);

#endif
