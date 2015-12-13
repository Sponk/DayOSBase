#include <stdio.h>
#include <string.h>
#include <dayos.h>
#include <stdlib.h>
#include "fat.h"

typedef enum
{
	FAT12,
	FAT16,
	FAT32,
	ExFAT
}FS_TYPE;

typedef struct FILESYSTEM
{
	FatBootSector boot;
	char mountpoint[256];
	FILE* device;
	FS_TYPE type;
	
	uint32_t total_sectors;
	uint32_t fat_size;
	uint32_t root_dir_sectors;
	uint32_t first_data_sector;
	uint32_t first_fat_sector;
	uint32_t data_sectors;
	uint32_t total_clusters;
	
	struct FILESYSTEM* next;
}filesystem_t;

const filesystem_t* mountpoints;

void calculate_fat_data(filesystem_t* fs)
{
	fs->total_sectors = (fs->boot.total_sectors_16 == 0) ? fs->boot.total_sectors_32 : fs->boot.total_sectors_16;
	fs->fat_size = fs->boot.table_size_16; // FIXME: HACK! Does not work for FAT32!
	fs->root_dir_sectors = ((fs->boot.root_entry_count * 32) + (fs->boot.bytes_per_sector - 1)) / fs->boot.bytes_per_sector;
	fs->first_data_sector = fs->boot.reserved_sector_count + (fs->boot.table_count * fs->fat_size) + fs->root_dir_sectors;
	
	fs->first_fat_sector = fs->boot.reserved_sector_count;

	fs->data_sectors = fs->total_sectors - (fs->boot.reserved_sector_count + (fs->boot.table_count * fs->fat_size) + fs->root_dir_sectors);
	fs->total_clusters = fs->data_sectors / fs->boot.sectors_per_cluster;
	
	if(fs->total_clusters < 4085) 
	{
		fs->type = FAT12;
		printf("Found FAT12 FS\n");
	} 
	else if(fs->total_clusters < 65525) 
	{
		fs->type = FAT16;
		printf("Found FAT16 FS\n");
	} 
	else if (fs->total_clusters < 268435445)
	{
		fs->type = FAT32;
		printf("Found FAT32 FS\n");
	}
	else
	{ 
		fs->type = ExFAT;
		printf("Found ExFAT FS\n");
	}
}

filesystem_t* open_device(const char* device)
{
	filesystem_t* fs = malloc(sizeof(filesystem_t));
	
	// Just for testing
	FILE* f = fopen(device, "rw");

	FatBootSector boot;
	fread(&fs->boot, sizeof(FatBootSector), 1, f);

	printf("boot->oem_name: %s\n", fs->boot.oem_name);
	printf("boot->bytes_per_sector: %d\n", fs->boot.bytes_per_sector);
	printf("boot->table_count: %d\n", fs->boot.table_count);
	printf("boot->root_entry_count: %d\n", fs->boot.root_entry_count);
	
	fs->device = f;
	
	calculate_fat_data(fs);	
	return fs;
}

uint32_t calculate_first_sector(filesystem_t* fs, uint32_t cluster)
{
	return ((cluster - 2) * fs->boot.sectors_per_cluster) + fs->first_data_sector;
}

void print_filename(FatFile* file)
{
	switch(file->attributes)
	{
		case ATTR_DIRECTORY:
			puts("<DIR> ");
			break;
		
		default: puts("<FILE> ");
	}
	
	for(int i = 0; i < 8; i++)
		putch(file->filename[i]);
	
	for(int i = 0; i < 3; i++)
		putch(file->extension[i]);

	puts("\n");
}

void list_root_dir(filesystem_t* fs)
{
	char buf[32];

	FatFile file;
	uint32_t root_sector = fs->first_data_sector - fs->root_dir_sectors;
	
	printf("root_sector: %d\n", root_sector);
	//return;
	// TODO: Handle FAT32
	
	//uint32_t sector = calculate_first_sector();

	fseek(fs->device, root_sector * fs->boot.bytes_per_sector, SEEK_SET);
	
	while(1)
	{
		fread(&file, sizeof(file), 1, fs->device);
		//fread(buf, 32, 1, fs->device);

		//for(int i = 0; i < 32; i++)
		//	printf("%x ", buf[i]);
		
		if(file.filename[0] == 0)
		{
			printf("No further entry found.\n\n");
			break;
		}
		/*else if(file.attributes == 0x10)
		{
			printf("Found directory!\n");
		}*/
		
		print_filename(&file);
		// printf("dir->filename: %s\n", file.filename);
		//printf("dir->filesize: %d\n", file.file_size);
	}
	
	printf("Done.\n\n");
	//printf("dir->filename: %s\n", file.filename);
	//printf("dir->filesize: %d\n", file.file_size);
	//free(entries);
}

int main()
{
	sleep(1000);
	
	filesystem_t* fs = open_device("/dayos/dev/fdc");
	
	list_root_dir(fs);
	
	/*FILE* f = fopen("/dayos/dev/fdc", "r");
	unsigned char buf[256];
	fseek(f, 512, SEEK_SET);
	fread(buf, 10, 1, f);
	
	for(int i = 0; i < 10; i++)
		printf("%d ", buf[i]);
	*/
	
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
