#include <stdio.h>
#include <windows.h>
#include <pt_ioctl.c>
#include "parallelPort.h"

#define IN_MASK		0x01
#define OUT_MASK	0x02
#define ALL_UP		0xFF
#define ALL_DOWN	0x00

unsigned int getParallelAddress(char *portName) 
{
	unsigned int far *ptraddr; /* Pointer to location of Port Addresses */
	unsigned int address; /* Address of Port */
	int a;

	if (!strcmp(portName, "LPT1")) {
		a = 0;
	} else if (!strcmp(portName, "LPT2")) { 
		a = 1;
	} else if (!strcmp(portName, "LPT3")) {
		a = 2;
	} else {
		printf("Please enter a printer address LPT1-3");
	}

	ptraddr=(unsigned int far *)0x00000408;
	*ptraddr += a;
	address = *ptraddr;
	printf("Address assigned to %s is 0x%Xh\n",portName, address);
}

void sample(void)
{
	unsigned char value;
	
	OpenPortTalk();
	outportb(0x378, 0xFF);
	value = inportb(0x378);
	printf("Value returned = 0x%02X \n",value);
	outp(0x378, 0xAA);
	value = inp(0x378);
	printf("Value returned = 0x%02X \n",value);
	ClosePortTalk();
}

bool isInputHigh(unsigned int address)
{
	unsigned char val = inp(address);
	unsigned char check = val & IN_MASK;
	return (check >= IN_MASK);
}

void toggleOut(unsigned int address)
{
	unsigned char val = inp(address); 
	val ^= OUT_MASK;
	outp(address, val);
}

void setOutOff(unsigned int address)
{
	unsigned char val = inp(address);
	val &= (~OUT_MASK);
	outp(address,val);
}

