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

#include "FileSystem.h"
#include <stdio.h>
#include <string.h>

FileSystem::FileSystem() :
root("/", VFS_MODE_RW)
{

}

FileSystem::~FileSystem()
{

}

std::string FileSystem::toString()
{
	std::ostringstream ss;
	
	ss << root.toString() << std::endl;
	
	return ss.str();
}

FSNode* FileSystem::findNode(char* path, const char* name,  FSNode* cur)
{
	if(name == NULL || strlen(name) == 0 || cur == NULL) return NULL;

	// printf("subdir: %s name: '%s' cur: '%s' type: %d (NODE_MOUNT = %d)\n", path, name, cur->getName(), cur->getType(), NODE_MOUNT);

	if(!strcmp(name, cur->getName()) || (cur->getType() == NODE_MOUNT))
	{
		return cur;
	}

	// printf("nextdir=%s\n", path);

	if(cur->getType() != NODE_DIR || path == NULL)
	{
		return NULL;
	}

	FSDir* dir = static_cast<FSDir*>(cur);
	FSNode* nextiter = dir->findChild(path);
	if(nextiter == NULL) 
	{
		return NULL;
	}
	
	return findNode(strtok(NULL, "/"), name, nextiter);
}

std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems)
{
	std::istringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) 
	{
		elems.push_back(item);
	}
	return elems;
}

std::string FileSystem::getFilename(std::string& str, char delim)
{
	char delimstr[2] = {delim, 0};
	int idx = str.find_last_of(delimstr);
	if(idx == -1)
		return "";
	
	//printf("fname: %s\n", str.substr(idx+1).c_str());
	return str.substr(idx+1);
}

FSNode* FileSystem::findNode(const char* path)
{
	if(!path || strlen(path) == 0)
		return false;
	
	//printf("Searching for: %s\n", path);
	if(!strcmp(path, "/")) return &root;

	//split(path, '/', pathlist);
	std::string spath;
	spath = path;
	
	size_t len = spath.length();
	if(len > 0 && len < spath.length() && spath.at(len-1) == '/')
		spath.erase(len-1);

	//std::stringstream strpath(spath);
	std::string name = getFilename(spath, '/');
	//printf("\n\n");
	char* p = strdup(spath.c_str());
	FSNode* node = findNode(strtok(p, "/"), name.c_str(), &root);
	free(p);

	return node;
}

bool FileSystem::addNode(const char* path, FSNode* node)
{
	if(!path || !node || strlen(path) == 0)
		return false;
	
	//printf("%s\n", path);
	std::string fullpath;
	fullpath = path;
	
	FSDir* parent = (path == "/") ? &root : dynamic_cast<FSDir*>(findNode(path));
	
	if(!parent) // FIXME: Exception!
	{
		printf("[ VFS ] Node %s not found!\n", fullpath.c_str());
		return false;
	}
	
	//if(parent->children.find(node) != parent->children.end())
	//
	
	if(parent->findChild(node->getName()))
		return false;
	
	//printf("Adding node %s to %s \n", node->getName(), parent->getName());
	parent->children.push_back(node);//.emplace(node->getName(), node);
	return true;
}

bool FileSystem::mkdev(const char* path, VFS_OPEN_MODES mode, DEVICE_TYPES type, pid_t pid)
{
	if(!path)
		return false;

	std::string fullpath;
	fullpath = path;
	
	std::string name = getFilename(fullpath, '/');
	fullpath.erase(fullpath.length() - name.length() - 1);
	
	//printf("mkdev: %s %s %d\n", fullpath.c_str(), name.c_str(), pid);
	return addNode(fullpath.c_str(), new FSDevice(name.c_str(), mode, type, pid));
}

bool FileSystem::mount(const char* from, const char* to, VFS_OPEN_MODES mode, pid_t pid)
{
	
	if(!from || !to)
		return false;
	
	FSDevice* device = dynamic_cast<FSDevice*>(findNode(from));
	if(!device)
		return false;
	
	if(findNode(to))
		return false;
	
	//printf("Mounting %s to %s\n", from, to);
	std::string fullpath;
	fullpath = to;
	
	std::string name = getFilename(fullpath, '/');
	fullpath.erase(fullpath.length() - name.length() - 1);
	
	//printf("mkdev: %s %s\n", fullpath.c_str(), name.c_str());
	return addNode(fullpath.c_str(), new FSMount(to, name.c_str(), mode, device, pid));
}

bool FileSystem::mount(const char* to, VFS_OPEN_MODES mode, pid_t pid)
{	
	if(!to)
		return false;
	
	if(findNode(to))
		return false;
	
	std::string fullpath;
	fullpath = to;
	
	std::string name = getFilename(fullpath, '/');
	fullpath.erase(fullpath.length() - name.length() - 1);

	if(fullpath.empty()) fullpath = "/";

	//printf("mount: %s name == %s\n", fullpath.c_str(), name.c_str());
	//FSMount(const char* path, const char* name, VFS_OPEN_MODES mode, FSDevice* dev, pid_t drv) : fullpath(path), FSNode(name, mode), device(dev), fs_driver(drv) {}

	//printf("Mount fullpath = %s to = %s name = %s\n", fullpath.c_str(), to, name.c_str());
	return addNode(fullpath.c_str(), new FSMount(to, name.c_str(), mode, NULL, pid));
}

void FileSystem::clear()
{
	root.clear();
}

std::string FSNode::toString()
{
	std::string str;
	str = name;
	str += " <NODE>";
	
	return str;
}

std::string FSDir::toString()
{
	std::ostringstream ss;
	ss << getName() << " <DIR>" << std::endl;
	
	for(auto n : children)
		ss << n->toString() << std::endl;
	
	return ss.str();
}