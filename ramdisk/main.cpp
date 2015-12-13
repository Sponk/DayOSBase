#include <ustl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dayos.h>
#include <driver.h>

using namespace std;

#define DISK_SIZE 20*1024*1024 // 20 MB

int main()
{
	int retval = vfs_create_device("/dayos/dev/ram", VFS_MODE_RW, VFS_BLOCK_DEVICE);
	if (retval == SIGNAL_FAIL)
	{
		printf("[ RAM ] Could not create device!\n");
		return 1;
	}
	
	message_t msg;
	struct vfs_request* rq = (struct vfs_request*) &msg.message;
	char* data = (char*) malloc(DISK_SIZE);
	
	printf("[ RAM ] RW RAM disk at 0x%x\n", data);	
	printf("[ RAM ] Started RAM disk device.\n");
	
	while(1)
	{
		while (receive_message(&msg, MESSAGE_ANY) != MESSAGE_RECEIVED)
				sleep(10);

		switch(msg.signal)
		{
			case DEVICE_READ: {
				//printf("Device read: %d offset = %d\n", msg.size, rq->offset);
				//write_message_stream(data+rq->offset, msg.size, msg.sender);

				write_message_stream(data+rq->offset, (msg.size + rq->offset < DISK_SIZE) ? msg.size : DISK_SIZE - rq->offset, msg.sender);
			}
			break;

			case DEVICE_WRITE: {
				read_message_stream(data+rq->offset, (msg.size + rq->offset < DISK_SIZE) ? msg.size : DISK_SIZE - rq->offset, msg.sender);
			}
			break;
			
			default:
				printf("[ RAM ] Unknown signal %d from %d\n", msg.signal, msg.sender);
		}
	}

	return 0;
}
