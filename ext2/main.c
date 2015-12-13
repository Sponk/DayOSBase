#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dayos.h>
#include "ext2.h"

struct ext2_fs
{
	char mountpoint[256];
	FILE* device;
	
	uint32_t blocksize;
	uint32_t fragmentsize;
	
	struct ext2_super_block superblock;
	struct ext2_fs* next;
};

uint32_t block2byte(struct ext2_fs* fs, uint32_t block)
{
	return fs->blocksize * block;
}

void fetch_block_group(struct ext2_fs* fs, struct ext2_block_group* bgroup, uint32_t num)
{
	// 1
	size_t nodeBlock = 1024 + num * sizeof(struct ext2_block_group);
	fseek(fs->device, nodeBlock, SEEK_SET);
	size_t sz = fread(bgroup, sizeof(struct ext2_block_group), 1, fs->device);
	
	printf("Got: %d with size %d\n", nodeBlock, sizeof(struct ext2_block_group));
	
	printf("inode_table_addr = %d\n", bgroup->inode_table_addr);
	printf("block_usage_bitmap_addr = %d\n", bgroup->block_usage_bitmap_addr);
	printf("inode_usage_bitmap_addr = %d\n", bgroup->inode_usage_bitmap_addr);
	
	printf("Number of unallocated inodes: %d\n", bgroup->unallocated_inodes_number);
	printf("Number of unallocated blocks: %d\n", bgroup->unallocated_blocks_number);
}

void fetch_inode_from_bgroup(struct ext2_fs* fs, struct ext2_block_group* bgroup, struct ext2_inode* inode, uint32_t idx)
{
	size_t nodeBlock = block2byte(fs, bgroup->inode_table_addr) + idx * 128;
	fseek(fs->device, nodeBlock, SEEK_SET);
	size_t sz = fread(inode, sizeof(struct ext2_inode), 1, fs->device);
	
	printf("Got: %d with size %d\n", nodeBlock, sizeof(struct ext2_inode));
	
	printf("mode = 0x%x\n", inode->mode);
	printf("size = %d\n", inode->size);
	printf("uid = %d\n", inode->uid);
}

void fetch_inode(struct ext2_fs* fs, struct ext2_inode* inode, uint32_t idx)
{
	uint32_t bgroup = (idx - 1) / fs->superblock.inodes_in_group;
	uint32_t id = (idx - 1) % fs->superblock.inodes_in_group;
	uint32_t block = (id * fs->superblock.inode_size) / fs->blocksize;
	
	struct ext2_block_group blockgroup;
	fetch_block_group(fs, &blockgroup, bgroup);

	printf("Looking for inode %d in bgroup %d with index %d in block %d offset %d\n", idx, bgroup, id, block, blockgroup.inode_table_addr + block);
	
	fseek(fs->device, block2byte(fs, blockgroup.inode_table_addr), SEEK_SET);
	size_t sz = fread(inode, sizeof(struct ext2_inode), 1, fs->device);
		
	printf("Got: %d with size %d\n", block2byte(fs, blockgroup.inode_table_addr), sizeof(struct ext2_inode));
	
	printf("mode = 0x%x\n", inode->mode);
	printf("size = %d\n", inode->size);
	printf("uid = %d\n", inode->uid);
	printf("blocks = %d\n", inode->block[0]);
}

struct ext2_fs* mount(const char* device, const char* mountpoint)
{
	// Just for testing
	FILE* fd0 = fopen(device, "rw");
	char buffer[512];
	fseek(fd0, 1024, SEEK_SET);
	fread(buffer, 512, 1, fd0);

	printf("%d\n", sizeof(struct ext2_super_block));
	struct ext2_super_block* super_block = (struct ext2_super_block*) buffer;

	if(super_block->ext2_signature != EXT2_SIGNATURE)
	{
		printf("[ EXT2 ] The medium has no valid EXT2 file system!\n");
		return NULL;
	}
	
	if(super_block->fs_state == EXT2_ERROR_FS)
	{
		printf("[ EXT2 ] The filesystem at '%s' is damaged with error %d. Please run fsck to fix that!\n\n", device, super_block->fs_state);
		return NULL;
	}

	struct ext2_fs* fs = malloc(sizeof(struct ext2_fs));
	
	// 1024
	fs->blocksize = 1024 << super_block->log2_blocksize;
	
	if(super_block->log2_fragmentsize >= 0)
		fs->fragmentsize = 1024 << super_block->log2_fragmentsize;
	else
		fs->fragmentsize = 1024 >> -super_block->log2_fragmentsize;
	
	printf("Read super block: 0x%x\n", super_block->ext2_signature);
	printf("Number of inodes: %d\n", super_block->inodes_number);
	printf("Number of blocks: %d\n", super_block->blocks_number);
	printf("Number of unallocated inodes: %d\n", super_block->unallocated_inodes_number);
	printf("Number of unallocated blocks: %d\n", super_block->unallocated_blocks_number);
	printf("Number of blocksize: %d\n", fs->blocksize);
	printf("Number of fragmentsize: %d\n", fs->fragmentsize);
	printf("File system was created by: %s\n", operating_systems[super_block->os_id]);
	printf("Volume name: %s\n", super_block->name);
	printf("inode size: %d\n", super_block->inode_size);
	
	fs->superblock = *super_block;
	fs->device = fd0;
	return fs;
}

void list_directories(struct ext2_fs* fs, struct ext2_inode* inode)
{
	struct ext2_directory_entry dir;

	printf("block: %d\n", inode->block[0]);
	return;
	
	fseek(fs->device, block2byte(fs, inode->block[0]), SEEK_SET);
	size_t sz = fread(&dir, sizeof(struct ext2_directory_entry), 1, fs->device);
	
	printf("Found dir: %s\n", dir.name);
}

int main()
{
	sleep(1000);

	struct ext2_fs* fs = mount("/dayos/dev/fdc", "/devices/a");
	
	if(!fs)
	{
		printf("[ EXT2 ] Killing driver.\n");
		while(1) sleep(10000);
		return 0;
	}
	
	//struct ext2_block_group blockgroup;
	//fetch_block_group(fs, &blockgroup, 0);
	
	struct ext2_inode inode;
	fetch_inode(fs, &inode, 2);
	
	//list_directories(fs, &inode);
	
	message_t msg;
	struct vfs_request* request = (struct vfs_request*) &msg.message; 
	while (1)
	{
		while (receive_message(&msg, MESSAGE_ANY) != MESSAGE_RECEIVED)
			sleep(100);

		switch (msg.signal)
		{

		}
	}

	for (;;);
}
