#ifndef OFX_SERIAL_H
#define OFX_SERIAL_H

#include "ofmain.h"
#include <stdio.h>
#include <Windows.h>

bool WriteComPort(DCB& dcb, HANDLE& hPort, string data);
bool OpenComPort(string PortSpecifier, DCB& dcb, HANDLE& hPort);
void CloseComPort(HANDLE hPort);

#endif