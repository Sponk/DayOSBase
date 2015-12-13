#include <stdio.h>
#include <string.h>
#include <dayos.h>
#include <driver.h>

#define SRA 0x3F0
#define SRB 0x3F1
#define DOR 0x3F2
#define TDR 0x3F3
#define MSR 0x3F4
#define DRSR 0x3F4
#define FIFO 0x3F5
#define DIR 0x3F7
#define CCR 0x3F7

#define DOR_INIT_BITMASK 0xC

#define CMD_VERSION 0x10
#define CMD_CONFIGURE 0x13
#define CMD_LOCK 0x94
#define CMD_UNLOCK 0x14
#define CMD_SPECIFY 0x3
#define CMD_CALIBRATE 0x7
#define CMD_SENSE 0x8
#define CMD_READ 0x46
#define CMD_SEEK 0xF

#define MSR_DATAREG 0x80
#define MSR_DIO_TO_CPU 0x40

#define DOR_RESET 4
#define DOR_DMA 8

#define ERR_TIMEOUT 1
#define NO_ERROR 0

#define ERR_NOT_SUPPORTED 2

unsigned int errno;

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

void WriteTransfer()
{
	outb(0x0A, 0x06); // Channel 2 Mask
	outb(0x0B, 0x5A); // DMA-Write
	// single transfer, address increment, autoinit, write, channel2
	// 01011010

	/*outb(0x81, 0x1000 >> 16);
	outb(0x04, (0x1000 & 0xffff) & 0xFF);
	outb(0x04, (0x1000 & 0xffff) >> 8);*/

	outb(0x0A, 0x02); // Channel 2 Unmask
}

void ReadTransfer()
{
	outb(0x0A, 0x06); // Channel 2 Mask
	outb(0x0B, 0x56); // DMA-Read

	/*outb(0x81, 0x1000 >> 16);
	outb(0x04, (0x1000 & 0xffff) & 0xFF);
	outb(0x04, (0x1000 & 0xffff) >> 8);*/

	outb(0x0A, 0x02); // Channel 2 Unmask
}

int WaitIRQ()
{
	message_t msg;
	msg.signal = 0;

	for (int i = 0; i < 50; i++)
	{
		if(receive_message(&msg, MESSAGE_ANY) == MESSAGE_RECEIVED && msg.signal == 0x26)
			return NO_ERROR;

		sleep(100);
	}

	return ERR_TIMEOUT;
}

int SendCommand(uint8_t cmd)
{
	for (uint32_t timeout = 0; timeout < 100; timeout++)
	{
		if ((inb(MSR) & 0xC0) == 0x80)
		{
			outb(FIFO, cmd);
			inb(MSR);
			return NO_ERROR;
		}
		sleep(10);
	}
	return ERR_TIMEOUT;
}

uint8_t ReadData()
{
	for (uint32_t timeout = 0; timeout < 100; timeout++)
	{
		if ((inb(MSR) & 0xD0) == 0xD0)
		{
			errno = NO_ERROR;
			return inb(FIFO);
		}
		sleep(10);
	}
	errno = ERR_TIMEOUT;
	return 0;
}

void InitDMA()
{

	// ISA DMA stuff
	outb(0x0a, 0x06); // mask DMA channel 2 and 0 (assuming 0 is already masked)
	outb(0x0c, 0xFF); // reset the master flip-flop
	outb(0x04, 0);	// address to 0 (low byte)
	outb(0x04, 0x10); // address to 0x10 (high byte)
	outb(0x0c, 0xFF); // reset the master flip-flop (again!!!)
	outb(0x05, 0xFF); // count to 0x23ff (low byte)
	outb(0x05, 0x23); // count to 0x23ff (high byte),
	outb(0x81, 0); // external page register to 0 for total address of 00 10 00
	outb(0x0a, 0x02); // unmask DMA channel 2
}


void Select(int driveNumber)
{
    uint8_t dor = inb(DOR);        
    outb(DOR, (dor & (~0x03)) | driveNumber);
}

void MotorOn(int driveNumber)
{
    uint8_t status = inb(0x3F2);
    outb(0x3F2, (status & 0x0F) | (0x10 << driveNumber));
    sleep(500);
}

void MotorOff(int driveNumber)
{
    uint8_t status = inb(0x3F2);
    outb(0x3F2, status & ~(0x10 << driveNumber));
}

void Sense(unsigned char* status, unsigned char* cylinder)
{
    SendCommand(CMD_SENSE);
    *status = ReadData();
    *cylinder = ReadData();
}   

void Calibrate(int driveNumber)
{
    uint8_t st0, cyl0;
    
    Select(driveNumber);    
    MotorOn(driveNumber);
    
    SendCommand(CMD_CALIBRATE);
    SendCommand(driveNumber);
    
    if(WaitIRQ() == ERR_TIMEOUT)
    {
        printf("[ FLOPPY ] Fehler: Timeout beim warten auf einen Interrupt!\n");
    }
    
    //Sense(&st0, &cyl0);
    
    MotorOff(driveNumber);
}

int Seek(int driveNumber, uint8_t cylinder, uint8_t head)
{
    uint8_t st0, cyl0;

    int i = 0;
    for(i = 0; i < 5; i++)
    {
        SendCommand(CMD_SEEK);
	    SendCommand((head << 2) | driveNumber);
	    SendCommand(cylinder);

	    WaitIRQ();
	    
	    Sense(&st0, &cyl0);
		
	    if(cyl0 == cylinder)
	       return NO_ERROR;

        sleep(15);
    }

    return ERR_TIMEOUT;
}

/// FIXME: Intelligente Motorverwaltung
int ReadSector(int driveNumber, uint8_t cylinder, uint8_t head, uint8_t sector)
{
    uint8_t cyl0, status;
        
    ReadTransfer();
        
    MotorOn(driveNumber);
    
    Seek(driveNumber, cylinder, head);
    
    for (int i = 0; i<5; i++)
    {
        SendCommand(CMD_READ);
        SendCommand((head<<2) | driveNumber);
        SendCommand(cylinder);
        SendCommand(head);
        SendCommand(sector);
        
        SendCommand(2);
        SendCommand(18);
        SendCommand(27);
        SendCommand(0xFF);
        
        WaitIRQ();
        	
        uint8_t st0 = ReadData();  //st0
        ReadData();  //st1
        ReadData();  //st2
        ReadData();  //cylinder
        ReadData();  //head
        ReadData();  //sector
        ReadData();  //Sektorgr����������������e
        
		Sense(&cyl0, &status);
		
        if((st0 & 0xC0) == 0)
        {
          MotorOff(driveNumber);
          return NO_ERROR;
        }
    }
    
    MotorOff(driveNumber);
    return ERR_TIMEOUT;
}

int ReadSectorLBA(int drive, uint32_t lba)
{
    uint8_t sector = (lba % 18) + 1;
    uint8_t cylinder = (lba / 18) / 2;
    uint8_t head = (lba / 18) % 2;
    
    return ReadSector(drive ,cylinder, head, sector);
}
/************************************************************/
/* Andere Helper Funktionen                                        */
/************************************************************/

int VersionCommand()
{
    uint8_t result;
    SendCommand(CMD_VERSION);
    result = ReadData();
    
    if(errno != NO_ERROR)
    {
        printf("[ FLOPPY ] Konnte keine Daten vom Diskettencontroller empfangen!\n");
        return errno;
    }
    else if(result != 0x90)
    {
        return ERR_NOT_SUPPORTED;
    }    
    
    return NO_ERROR;
}

int ConfigureAndLock()
{
    // Configure
    int ret = SendCommand(CMD_CONFIGURE);
                
    ret = SendCommand(0);
    ret = SendCommand((1 << 6) | (0 << 5) | (0 << 4) | 7);
    ret = SendCommand(0);
    
    // FIXME: Lock
    /*ret = SendCommand(CMD_LOCK);
    
    if(ReadData() != (0x80 << 4))
    {
        printf("[ FLOPPY ] Warnung: Konnte das Laufwerk nicht sperren!\n");
    } */ 
    
    return ret;
}

void Reset()
{
    outb(DOR, 0);
    outb(DOR, 0x0C);
    
    WaitIRQ();
    
    /*sleep(10);
    outb(MSR, 0);
    outb(DOR, 0x0C);
    
    //WaitIRQ();

    /// TODO: Sense interrupts*/
    
    outb(CCR, 0);   
}    

void readSectorToBuffer(uint32_t lba, char* buffer, uint32_t* currentBlock)
{
	if (currentBlock != lba)
	{
		*currentBlock = lba;
		ReadSectorLBA(0, lba);
		memcpy(buffer, 0x1000, 512);
	}
}

#define min(a, b) ((a < b) ? a : b)

int main()
{
	sleep(100);

	// Check if FDC exists
	/*outb(0x70, 0x10);
	uint8_t drives = inb(0x71) >> 4;

	if (drives == 0)
	{
		// No drives found
		while (1)
			sleep(1000);
		return 0;
	}

	printf("Your system has %d floppy drives.\n\n", drives);*/

	int retval = vfs_create_device("/dayos/dev/fdc", VFS_MODE_RW, VFS_BLOCK_DEVICE);
	if (retval == SIGNAL_FAIL)
	{
		printf("[ FDC ] Could not create device file!\n");
		return -1;
	}
	
	registerHandlerProcess(0x26);
	
	// Register IRQ 0x26
	InitDMA();

	Reset();

	int result = VersionCommand();
	
	if(result == ERR_NOT_SUPPORTED)
    {
        printf("[ FLOPPY ] Your drive is not supported!\n");
    }
     
    if(ConfigureAndLock() == ERR_TIMEOUT)
    {
        printf("[ FLOPPY ] Timeout while configuring drive!\n");
    }

	Calibrate(0);
	
	/*unsigned long sectornum = 0;
	unsigned char* sector = 0x1000;

	ReadSectorLBA(0, 0);
	
	int i = 0;
	int c, j;
	for (c = 0; c < 4; ++c)
	{
		for (j = 0; j < 128; ++j)
			printf("%x ", sector[i + j]);
		i += 128;
		sleep(3000);
		printf("\n\n\n\n");
		}*/

	char* buffer = malloc(512);
	uint32_t currentBlock = -1;
	
	message_t msg;
	struct vfs_request* request = (struct vfs_request*) &msg.message; 
	while (1)
	{
		while (receive_message(&msg, MESSAGE_ANY) != MESSAGE_RECEIVED)
			sleep(100);

		switch (msg.signal)
		{

		case DEVICE_READ: {

			if (request->magic != VFS_MAGIC)
			{
				printf("[ FDC ] Invalid read request!\n");
				msg.signal = SIGNAL_FAIL;
				send_message(&msg, msg.sender);
				break;
			}

			pid_t sender = msg.sender;
			
			size_t byteOffset = request->offset % 512;
			size_t block = (request->offset - byteOffset) / 512;

			size_t requestEnd = request->offset + request->size;
			size_t byteOffsetEnd = requestEnd % 512;
			size_t secondBlock = (requestEnd - byteOffsetEnd) / 512;
			
			size_t numMessages = (request->size > MESSAGE_STRING_SIZE) ? (request->size / MESSAGE_STRING_SIZE) : 1;
			
			/*if (currentBlock != block)
			{
				printf("READING BLOCK %d AND BLOCK %d WITH OFFSETS %d AND %d INTO BUFFER\n\n", block, secondBlock, byteOffset, byteOffsetEnd);
				printf("%d MESSAGES ARE NEEDED TO TRANSFER THE DATA\n", numMessages);
				currentBlock = block;
				ReadSectorLBA(0, block);
				memcpy(buffer, 0x1000, 512);
			}*/
			
			//printf("READING BLOCK %d AND BLOCK %d WITH OFFSETS %d AND %d INTO BUFFER\n\n", block, secondBlock, byteOffset, byteOffsetEnd);
			//printf("%d MESSAGES ARE NEEDED TO TRANSFER THE DATA OF SIZE %d\n", numMessages, request->size);

			readSectorToBuffer(block, buffer, &currentBlock);

			msg.signal = min(512 - byteOffset, request->size);
			memcpy(msg.message, buffer + byteOffset, msg.signal);
			send_message(&msg, sender);

			//printf("Sending message for block %d with size %d\n", block, msg.signal);			
			block++;
			
			for(int i = 0; i < numMessages-1; i++, block++)
			{
				//printf("Sending message for block %d\n", block);
				readSectorToBuffer(block, buffer, &currentBlock);

				memcpy(msg.message, buffer, MESSAGE_STRING_SIZE);
				msg.signal = MESSAGE_STRING_SIZE;
				send_message(&msg, sender);
			}

			block++;
			if(byteOffsetEnd > 0 && numMessages > 1)
			{
				//printf("Sending message for block %d\n", block);
				readSectorToBuffer(secondBlock, buffer, &currentBlock);

				memcpy(msg.message, buffer, byteOffsetEnd);
				msg.signal = byteOffsetEnd;
				send_message(&msg, sender);
			}
			
			msg.signal = SIGNAL_OK;
			send_message(&msg, sender);
		}
			
			break;

		case DEVICE_WRITE:
			break;
			
		default:
			printf(" [ FDC ] Unknown request signal %d from %d!\n\n", msg.signal, msg.sender);
		}
	}

	for (;;)
		;
}
