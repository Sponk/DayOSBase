#include <stdio.h>
#include <string.h>
#include <dayos.h>
#include <stdlib.h>

#define LOG(msg) printf("[ INIT ] %s\n", msg)

void execute_program(const char* path)
{
	FILE* exec = fopen(path, "r");
	
	if(!exec)
	{
		LOG("Could not open executable!");
		return;
	}
	
	char* content;
	fseek(exec, 0, SEEK_END);
	size_t sz = ftell(exec);
	fseek(exec, 0, SEEK_SET);

	content = (char*) malloc(sz);
	fread(content, sz, 1, exec);
	
	syscall1(9, content);
	
	fclose(exec);
	free(content);
}

void execute_config(const char* path)
{
	FILE* config = fopen(path, "r");
	
	if(!config)
	{
		LOG("Could not open config file!");
		while(1) sleep(1000);
	}
	
	char* content;
	fseek(config, 0, SEEK_END);
	size_t sz = ftell(config);
	fseek(config, 0, SEEK_SET);

	content = (char*) malloc(sz+1);
	fread(content, sz, 1, config);
	
	char* p = strtok(content, "\n");
	while(p != NULL)
	{
		
		if(!strcmp(p, "delay"))
		{
			sleep(250);
		}
		else if(strcmp(p, ""))
		{
			LOG(p);
			execute_program(p);
		}
		
		p = strtok(NULL, "\n");
	}
	
	fclose(config);
	free(content);
}

int main()
{
	LOG("Starting INIT system.");
	pid_t pid = 0;
	
	sleep(150);
	
	// Wait for VFS to crop up
	while((pid = get_service_pid("vfs")) == 0) sleep(50);
	
	//FILE* config = fopen(");
	
	sleep(150);
	execute_config("/drives/roramdisk/init.cfg");
	//execute_program("/drives/ramdisk/system/tty.drv");
	
	while(1)
	{
		sleep(1000);
	}

	for(;;);
}
