/*
 * The Virtual File System of the DayOS
 * Copyright (C) 2015  Yannick Pflanzer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#ifdef __dayos__
#include <ustl.h>
#include <stdio.h>
#define stringstream istringstream
#define std ustl
#else
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cstdio>
#endif

#include <stddef.h>
#include <vfs.h>

enum NODE_TYPE
{
	NODE_EMPTY = 0,
	NODE_DIR,
	NODE_DEVICE,
	NODE_MOUNT
};

class FSNode
{
	std::string name;
	VFS_OPEN_MODES mode;
	pid_t uid;
	pid_t guid;

public:
	FSNode(){};
	FSNode(const char* name, VFS_OPEN_MODES mode) : mode(mode)
	{
		this->name.assign(name);
	}

	virtual ~FSNode() {}

	const char* getName() { return name.c_str(); }
	VFS_OPEN_MODES getMode() { return mode; }
	virtual std::string toString();
	virtual NODE_TYPE getType() { return NODE_EMPTY; }

	pid_t getUID() { return uid; }
	pid_t getGUID() { return guid; }

	virtual void clear() {}
};

class FSDir : public FSNode
{
	void clear(FSNode* node);

public:
	FSDir(const char* name, VFS_OPEN_MODES mode) : FSNode(name, mode) {}
	virtual std::string toString();
	// std::map<std::string, FSNode*> children;
	std::vector<FSNode*> children;

	virtual NODE_TYPE getType() { return NODE_DIR; }

	void clear()
	{
		for (int i = 0; i < children.size(); i++)
		{
			FSNode* n = children[i];
			// n.second->clear();
			n->clear();
			delete n;
		}

		children.clear();
	}

	FSNode* findChild(std::string name)
	{
		FSNode* n;
		for (int i = 0; i < children.size(); i++)
		{
			n = children[i];
			if (name == n->getName())
				return n;
		}
		return NULL;
	}
};

class FSDevice : public FSNode
{
	DEVICE_TYPES type;
	std::string mountpoint;
	pid_t driver;

public:
	FSDevice(const char* name, VFS_OPEN_MODES mode, DEVICE_TYPES type,
			 pid_t pid)
		: FSNode(name, mode), type(type), driver(pid)
	{
	}
	DEVICE_TYPES getDeviceType() { return type; }
	const char* getMountpoint() { return mountpoint.c_str(); }
	pid_t getDriverPID() { return driver; }
	void setMountpoint(const char* str) { mountpoint = str; }
	virtual NODE_TYPE getType() { return NODE_DEVICE; }
};

class FSMount : public FSNode
{
	FSDevice* device;
	pid_t fs_driver;
	std::string fullpath;

public:
	FSMount(const char* path, const char* name, VFS_OPEN_MODES mode,
			FSDevice* dev, pid_t drv)
		: FSNode(name, mode), device(dev), fs_driver(drv)
	{
		fullpath.assign(path);
	}

	FSDevice* getDevice() { return device; }
	pid_t getFilesystemDriver() { return fs_driver; }
	virtual NODE_TYPE getType() { return NODE_MOUNT; }
	const char* getPath() { return fullpath.c_str(); }
};

class FileSystem
{
	FSDir root;
	FSNode* findNode(char* path, const char* name, FSNode* cur);
	FSNode* findNode(std::stringstream& path, const char* name, FSNode* cur);

public:
	FileSystem();
	~FileSystem();

	std::string toString();

	FSNode* findNode(const char* path);
	bool addNode(const char* path, FSNode* node);
	bool mkdev(const char* path, VFS_OPEN_MODES mode, DEVICE_TYPES type,
			   pid_t pid);
	bool mount(const char* from, const char* to, VFS_OPEN_MODES mode,
			   pid_t pid);
	bool mount(const char* to, VFS_OPEN_MODES mode, pid_t pid);

	static std::string getFilename(std::string& str, char delim);
	void clear();

	FSDir* getRoot() { return &root; }
};

#endif // FILESYSTEM_H
