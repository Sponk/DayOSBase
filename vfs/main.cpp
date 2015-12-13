#include <ustl.h>
#include "FileSystem.h"

#include <stdio.h>
#include <string.h>
#include <dayos.h>
#include <stdlib.h>

using namespace std;

int main()
{
	FileSystem rootfs;

	rootfs.addNode("/", new FSDir("dayos", VFS_MODE_RW));
	rootfs.addNode("/", new FSDir("drives", VFS_MODE_RW));
	rootfs.addNode("/dayos", new FSDir("dev", VFS_MODE_RW));

	if (register_service("vfs") != SIGNAL_OK)
	{
		printf("[ VFS ] Could not register VFS service. Will terminate.\n");
		return -1;
	}

	/*root = (dir_t*) malloc(sizeof(dir_t));
	strcpy(root->path, "/");

	mkdir("/", "dayos");
	mkdir("/", "drives");
	mkdir("/dayos/", "dev");*/

	printf("[ VFS ] Virtual File System started.\n");

	message_t msg;
	struct vfs_request* request = (struct vfs_request*)&msg.message;
	while (1)
	{
		while (receive_message(&msg, MESSAGE_ANY) != MESSAGE_RECEIVED)
			sleep(10);

		switch (msg.signal)
		{
			case VFS_SIGNAL_CREATE_DEVICE:

				if (!rootfs.mkdev(request->path, (VFS_OPEN_MODES)request->mode,
								  (DEVICE_TYPES)request->param, msg.sender))
					msg.signal = SIGNAL_FAIL;
				else
					msg.signal = SIGNAL_OK;

				send_message(&msg, msg.sender);
				break;

			case VFS_SIGNAL_MOUNT_DEVICE:
			{
				// printf("Mounting from: %s\n", request->path);
				struct vfs_request req = *request;
				if (receive_message_timeout(&msg, msg.sender, 100, 10) !=
					MESSAGE_RECEIVED)
					break;

				if (!rootfs.mount(req.path, request->path,
								  (VFS_OPEN_MODES)req.mode, request->param))
					msg.signal = SIGNAL_FAIL;
				else
					msg.signal = SIGNAL_OK;

				send_message(&msg, msg.sender);

				// printf("Mounting to: %s %d 0x%x\n", request->path,
				// msg.signal, rootfs.findNode(request->path));
				// printf("Tree:\n%s\n", rootfs.toString().c_str());
			}
			break;

			case VFS_SIGNAL_MOUNT_RAMDISK:
			{
				if (!rootfs.mount(request->path, (VFS_OPEN_MODES)request->mode,
								  msg.sender))
					msg.signal = SIGNAL_FAIL;
				else
					msg.signal = SIGNAL_OK;

				send_message(&msg, msg.sender);

				// printf("Mounting to: %s %d 0x%x\n", request->path,
				// msg.signal, rootfs.findNode(request->path));
				// printf("WRITING to: %s %d 0x%x\n", request->path, msg.signal,
				// rootfs.findNode("/roramdisk/FUCK"));
			}
			break;

			case VFS_SIGNAL_OPEN:
			{
				// printf("READING to: %s %d 0x%x\n", "RORORO", msg.signal,
				// rootfs.findNode("/roramdisk/init.cfg"));
				// break;
				pid_t sender = msg.sender;
				FSNode* node = rootfs.findNode(request->path);
				struct vfs_file* file = (struct vfs_file*)&msg.message;

				if (!node)
				{
					msg.signal = SIGNAL_FAIL;
					send_message(&msg, msg.sender);
					break;
				}

				msg.signal = SIGNAL_OK;
				switch (node->getType())
				{
					case NODE_DEVICE:
					{
						FSDevice* dev = static_cast<FSDevice*>(node);
						file->device = dev->getDriverPID();
						file->type = dev->getDeviceType();
						file->uid = dev->getUID();
						file->guid = dev->getGUID();

						strcpy(file->path, request->path);
					}
					break;

					case NODE_MOUNT:
					{
						FSMount* mount = static_cast<FSMount*>(node);
						file->device = mount->getFilesystemDriver();
						file->type = VFS_MOUNTPOINT;

						// FIXME: Fetch from FS driver!
						file->uid = mount->getUID();
						file->guid = mount->getGUID();

						std::string path;
						path = request->path;
						path = path.substr(strlen(mount->getPath()));
						strcpy(file->path, path.c_str());
					}
					break;

					default:
						msg.signal = SIGNAL_FAIL;
				}

				send_message(&msg, sender);
			}
			default:
			{
			}
				// printf("[ VFS ] Unknown signal %d from %d\n", msg.signal,
				// msg.sender);
		}
	}

	for (;;);
}
