#include "ofxSerial.h"

bool WriteComPort(DCB& dcb, HANDLE& hPort, string data)
{
	DWORD byteswritten;
	DWORD bytesToWrite = data.length();

	bool retVal = WriteFile(hPort, data.c_str(), bytesToWrite, &byteswritten, NULL);
	
	return retVal;
}

bool OpenComPort(string PortSpecifier, DCB& dcb, HANDLE& hPort)
{
	hPort = CreateFile2(PortSpecifier.c_str(), GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	
	if (!GetCommState(hPort,&dcb))
		return false;
	
	dcb.BaudRate = CBR_9600; //9600 Baud
	dcb.ByteSize = 8; //8 data bits
	dcb.Parity = NOPARITY; //no parity
	dcb.StopBits = ONESTOPBIT; //1 stop
	
	if (!SetCommState(hPort,&dcb))
		return false;

	return true;
}

void CloseComPort(HANDLE hPort)
{
	CloseHandle(hPort); //close the handle
}