#include <stdio.h>
#include <string.h>
#include <dayos.h>
#include <stdlib.h>
#include "tlli-master/include/tlli.h"

char buffer[512];
void read_line(FILE* stream)
{
	char c;
	char* s = buffer;
	while ((c = fgetc(stream)) != '\n' && c != 0 &&
		   s < (&buffer + sizeof(buffer)))
	{
		if (c == '\b' && s > buffer)
		{
			*s = 0;
			s--;
			putch('\b');
			continue;
		}
		else if (c == '\b')
			continue;

		*s = c;
		s++;
	}

	*s = '\0';
}

int execute_program(const char* path)
{
	FILE* exec = fopen(path, "r");
	
	if(!exec)
	{
		return 1;
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
	
	return 0;
}

int useTlli = 0;

tlliValue* exitTlli(int i, tlliValue** val)
{
	useTlli = 0;

	tlliValue* rtn;
	tlliIntToValue(0, &rtn);
	return rtn;
}

tlliValue* tlli_execute(int i, tlliValue** val)
{
	tlliValue* rtn;
	char buf[512];
	tlliValueToString(val[0], &buf, 512);
	tlliIntToValue(execute_program(buf), &rtn);
	return rtn;
}

void execute_script(const char* path, tlliContext* context)
{
	FILE* script = fopen(path, "r");

	if (!script)
	{
		printf("Shellscript does not exist!\n");
		return;
	}

	char* content;
	fseek(script, 0, SEEK_END);
	size_t sz = ftell(script);
	fseek(script, 0, SEEK_SET);

	content = (char*)malloc(sz + 1);
	fread(content, sz, 1, script);
	fclose(script);

	char* p = strtok(content, "\n");
	tlliValue* retval = NULL;
	char* outputBuffer = (char*)malloc(512);

	while (p != NULL)
	{
		if (!strcmp(p, ""))
		{
			p = strtok(NULL, "\n");
			continue;
		}

		if (tlliEvaluate(context, p, &retval) != TLLI_SUCCESS)
		{
			printf("Error: %s\n    %s\n\n", tlliError(), p);
			tlliReleaseValue(&retval);
			free(content);
			return;
		}

		tlliValueToString(retval, &outputBuffer, 512);
		tlliReleaseValue(&retval);

		//printf("%s\n\n", outputBuffer);
		p = strtok(NULL, "\n");
	}

	free(content);
}

int main()
{
	tlliValue* retval = NULL;
	tlliContext* context = NULL;
	tlliInitContext(&context);
	tlliAddFunction(context, "exit", exitTlli);
	tlliAddFunction(context, "execute", tlli_execute);
	
	execute_script("/drives/roramdisk/lisp/boot.lisp", context);

	// sleep(150);
	printf("\nThe DayOS shell v0.1\n\n");
	printf("Type 'help' for a list of available commands.\n\n");

	// FILE* in = fopen("/dayos/dev/tty", "r");
	// stdin = in;

	char* outputBuffer = malloc(512);

	while (1)
	{
		printf("DayOS > ");
		read_line(stdin);

		if (!useTlli)
		{
			char program[256];
			snprintf(program, sizeof(program), "%s/%s", "/drives/roramdisk", buffer);
			
			if(!execute_program(program))
			{
				
			}
			else if (!strcmp("tlli", buffer))
			{
				useTlli = 1;
				printf("Using TLLI for LISP interface. Type (exit) to return "
					   "to the default shell.\n\n");
			}
			else if (!strcmp("help", buffer))
			{
				printf("Commands: help, tlli, readfloppy\n\n");
			}
			else if (!strcmp("exit", buffer))
			{
				return 0;
			}
			else if (!strcmp("readram", buffer))
			{
				FILE* f = fopen("/dayos/dev/ram", "r");
				unsigned int data[12];
				
				printf("\n\n");
				
				fread(data, 11, 1, f);
				for (int i = 0; i < sizeof(data); i++)
				{
					printf("%x ", data[i]);
				}
				
				fclose(f);
				printf("\n\n");
			}
			else if (!strcmp("writeram", buffer))
			{
				FILE* f = fopen("/dayos/dev/ram", "r");
				unsigned int data[12] = {0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF};
				fwrite(data, sizeof(unsigned int), 12, f);
				fputc('C', f);
				
				fclose(f);
			}
			else if (!strcmp("readfloppy", buffer))
			{
				uint32_t wantedSize = 1024;
				FILE* f = fopen("/dayos/dev/fdc", "r");
				// fseek(f, 1024, SEEK_SET);

				unsigned char* buf = malloc(wantedSize);
				uint32_t sz = fread(buf, wantedSize, 1, f);

				printf("Read %d bytes\n\n");
				for (int i = 0; i < sz; i++)
				{
					printf("%x ", buf[i]);
				}
				printf("Done.\n\n\n");

				free(buf);
				fclose(f);
			}
			else if (strlen(buffer) > 0)
			{
				printf("Unknown command!\n\n");
			}
		}
		else
		{
			if (tlliEvaluate(context, buffer, &retval) != TLLI_SUCCESS)
			{
				printf("Error: %s\n    %s\n\n", tlliError(), buffer);
				tlliReleaseValue(&retval);
				continue;
			}

			tlliValueToString(retval, &outputBuffer, 512);
			tlliReleaseValue(&retval);

			printf("%s\n\n", outputBuffer);
		}
	}

	for (;;)
		;
}
