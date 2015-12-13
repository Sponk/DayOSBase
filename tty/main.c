#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dayos.h>
#include <driver.h>

#define BUFFER_SIZE 512
#define TRUE 1
#define FALSE 0

unsigned char kbdgerman[128] = {
	'0',  '0', '1', '2', '3',  '4',  '5', '6', '7',  '8',
	'9',  '0', '-', '=', '\b', '\t',					  /* Tab */
	'q',  'w', 'e', 'r',								  /* 19 */
	't',  'z', 'u', 'i', 'o',  'p',  '[', ']', '\n',	  /* Enter key */
	'0',												  /* 29   - Control */
	'a',  's', 'd', 'f', 'g',  'h',  'j', 'k', 'l',  ';', /* 39 */
	'\'', '`', '0',										  /* Left shift */
	'\\', 'y', 'x', 'c', 'v',  'b',  'n',				  /* 49 */
	'm',  ',', '.', '-', '0',							  /* Right shift */
	'*',  '0',											  /* Alt */
	' ',												  /* Space bar */
	'0',												  /* Caps lock */
	'0',											/* 59 - F1 key ... > */
	'0',  '0', '0', '0', '0',  '0',  '0', '0', '0', /* < ... F10 */
	'0',											/* 69 - Num lock*/
	'0',											/* Scroll Lock */
	'0',											/* Home key */
	'0',											/* Up Arrow */
	'0',											/* Page Up */
	'-',  '0',										/* Left Arrow */
	'0',  '0',										/* Right Arrow */
	'+',  '0',										/* 79 - End key*/
	'0',											/* Down Arrow */
	'0',											/* Page Down */
	'0',											/* Insert Key */
	'0',											/* Delete Key */
	'0',  '0', '0', '0',							/* F11 Key */
	'0',											/* F12 Key */
	'0',											/*undefined */
};

unsigned char kbdgermanshift[128] = {
	'0',  '0', '!', '"', '0',  '$',  '%', '&', '/',  '(',
	')',  '=', '?', '=', '\b', '\t',					  /* Tab */
	'Q',  'W', 'E', 'R',								  /* 19 */
	'T',  'Z', 'U', 'I', 'O',  'P',  '[', ']', '\n',	  /* Enter key */
	'0',												  /* 29   - Control */
	'A',  'S', 'D', 'F', 'G',  'H',  'J', 'K', 'L',  ';', /* 39 */
	'\'', '`', '0',										  /* Left shift */
	'\\', 'Y', 'X', 'C', 'V',  'B',  'N',				  /* 49 */
	'M',  ';', ':', '_', '0',							  /* Right shift */
	'*',  '0',											  /* Alt */
	' ',												  /* Space bar */
	'0',												  /* Caps lock */
	'0',											/* 59 - F1 key ... > */
	'0',  '0', '0', '0', '0',  '0',  '0', '0', '0', /* < ... F10 */
	'0',											/* 69 - Num lock*/
	'0',											/* Scroll Lock */
	'0',											/* Home key */
	'0',											/* Up Arrow */
	'0',											/* Page Up */
	'-',  '0',										/* Left Arrow */
	'0',  '0',										/* Right Arrow */
	'+',  '0',										/* 79 - End key*/
	'0',											/* Down Arrow */
	'0',											/* Page Down */
	'0',											/* Insert Key */
	'0',											/* Delete Key */
	'0',  '0', '0', '0',							/* F11 Key */
	'0',											/* F12 Key */
	'0',											/*undefined */
};

void outb(unsigned short port, unsigned char value)
{
	asm volatile("outb %1, %0" : : "dN"(port), "a"(value));
}

/* Liest einen byte aus einem Port */

unsigned char inb(unsigned short port)
{
	unsigned char ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

void SendKBCCommand(uint8_t cmd)
{
	while ((inb(0x64) & 0x2))
		;

	outb(0x60, cmd);
}

void EmptyKBCBuffer()
{
	// Tastaturpuffer leeren
	while (inb(0x64) & 0x1)
	{
		inb(0x60);
	}
}

void GetKeyboardInfo()
{
	EmptyKBCBuffer();
	SendKBCCommand(0xF2);

	uint8_t c = inb(0x60);

	if(c == 0xFA)
		printf("[ TTY ] You have an AT keyboard (0x%x).\n", c);
	else
		printf("[ TTY ] You have an unknown keyboard (0x%x).\n", c);
}

char* buffer;
uint32_t len = 0;

void UpdateBuffer(char c)
{

	// if(c == '\b')
	//{
	// putch('\b');//printf("BACK\n");
	//}

	if (c == '\b' && len > 0)
	{
		len--;
	}

	if (len < BUFFER_SIZE && c != '\b')
	{
		buffer[len] = c;
		len++;
	}

	if (len > 0)
		putch(c);
}

void moveBufferLeft(char* s)
{
	for (int i = 0; i < len - 1; i++)
	{
		buffer[i] = buffer[i + 1];
	}

	len--;
}

int main()
{
	pid_t read_request = 0;
	message_t msg;
	buffer = (char*)malloc(BUFFER_SIZE);

	sleep(100);

	int retval =
		vfs_create_device("/dayos/dev/tty", VFS_MODE_RW, VFS_CHARACTER_DEVICE);
	if (retval == SIGNAL_FAIL)
	{
		printf("[ TTY ] Could not create device file!\n");
		while (1)
			;
		return -1;
	}

	registerHandlerProcess(0x21);

	GetKeyboardInfo();
	EmptyKBCBuffer();
	SendKBCCommand(0xF4);

	uint8_t shifted;
	char lastChar = 0;

	while (1)
	{
		msg.signal = 0;
		while (receive_message(&msg, MESSAGE_ANY) != MESSAGE_RECEIVED)
			;

		switch (msg.signal)
		{
			case 0x21:
			{
				uint8_t scancode;
				scancode = inb(0x60);

				if (scancode & 0x80)
				{
					// Release Event

					switch (scancode)
					{
						// L-Shift
						case 0xaa:
						case 0xb6:
							shifted = FALSE;
							break;
					}
				}
				else
				{
					// Press Event
					switch (scancode)
					{
						// L-Shift
						case 0x2a:
						case 0x36:
							shifted = TRUE;
							break;

						default:
							if (!shifted)
								lastChar = kbdgerman[scancode];
							else
								lastChar = kbdgermanshift[scancode];

							UpdateBuffer(lastChar);

							if (read_request)
							{
								msg.message[0] = lastChar;
								msg.signal = SIGNAL_OK;
								send_message(&msg, read_request);

								if (lastChar != '\b')
									moveBufferLeft(buffer);

								read_request = 0;
							}
					}
				}
			}
			break;

			case DEVICE_READ:
				if (read_request != 0 && msg.sender != read_request)
				{
					msg.signal = SIGNAL_FAIL;
					send_message(&msg, msg.sender);
					break;
				}

				if (len == 0)
				{
					read_request = msg.sender;
					break;
				}

				msg.message[0] = buffer[0];

				moveBufferLeft(buffer);
				msg.signal = SIGNAL_OK;
				send_message(&msg, msg.sender);
				break;

			case DEVICE_WRITE:

				break;
		}
	}

	for (;;)
		;
}
