#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

#define EXT2_SIGNATURE 0xEF53

typedef enum
{
	EXT2_VALID_FS = 1,
	EXT2_ERROR_FS = 2
}EXT2_STATES;

typedef enum
{
	EXT2_IGNORE_ERROR = 1,
	EXT2_REMOUNT_RO = 2,
	EXT2_KERNEL_PANIC = 3 // Utterly useless for usermode driver...
}EXT2_ERRORS;

struct ext2_super_block
{
	uint32_t inodes_number;
	uint32_t blocks_number;
	uint32_t superuser_reserved_number;
	uint32_t unallocated_blocks_number;
	uint32_t unallocated_inodes_number;
	uint32_t superblock_id;
	uint32_t log2_blocksize; // log2(blocksize) - 10  blocksize = log2_blocksize << 1024
	uint32_t log2_fragmentsize; // log2(fragmentsize) - 10  fragmentsize = log2_fragmentsize << 1024
	uint32_t blocks_in_group;
	uint32_t fragments_in_group;
	uint32_t inodes_in_group;
	uint32_t last_mounted;
	uint32_t last_written;
	uint16_t mounts_since_fsck;
	uint16_t mounts_before_fsck;
	uint16_t ext2_signature;
	uint16_t fs_state;
	uint16_t error_info;
	uint16_t version_minor;
	uint32_t time_since_fsck;
	uint32_t time_before_fsck;
	uint32_t os_id;
	uint32_t version_major;
	uint16_t su_id;
	uint16_t su_group_id;
	
	// Extended field
	uint32_t first_free_inode;
	uint16_t inode_size;
	uint16_t backup_block;
	uint32_t optional_features;
	uint32_t required_features;
	uint32_t ro_features;
	uint8_t fs_id[16];
	uint8_t name[16];
	uint8_t last_mounted_at[64];
	uint32_t compression;
	uint8_t blocks_for_files;
	uint8_t blocks_for_directories;
	uint16_t unused;
	uint8_t journal_id[16];
	uint32_t journal_inode;
	uint32_t journal_device;
	uint32_t orphan_list_addr;
};

struct ext2_block_group
{
	uint32_t block_usage_bitmap_addr;
	uint32_t inode_usage_bitmap_addr;
	uint32_t inode_table_addr;
	uint16_t unallocated_blocks_number;
	uint16_t unallocated_inodes_number;
	uint16_t directories_number;
	uint16_t padding;
	uint32_t reserved[3];
};

struct ext2_inode
{
	uint16_t mode;
	uint16_t uid;
	uint32_t size;
	uint32_t atime;
	uint32_t ctime;
	uint32_t mtime;
	uint32_t dtime;
	uint16_t gid;
	uint16_t links_number;
	uint32_t blocks_number;
	uint32_t osd1;
	uint32_t block[15];
	uint32_t generation;
	uint32_t file_acl;
	uint32_t dir_acl;
	uint32_t faddr;
	uint32_t osd2[3];
};

struct ext2_directory_entry
{
	uint32_t inode;
	uint16_t size;
	uint8_t name_length_lower;
	uint8_t name_length_upper;
	uint8_t name[256];	
};

const char* operating_systems[] = {"Linux", "GNU HURD", "MASIX", "FreeBSD", "Other", "DayOS"};

#define DAYOS_ID 5

#endif
