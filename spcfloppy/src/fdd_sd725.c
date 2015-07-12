#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "sd725.h"

static HANDLE hSerial;
static char portName[256];
static char connected;

#define false 0
#define true 1

#define ARDUINO_WAIT_TIME 2000

int sd_serial(const char *com)
{
    strcpy(portName, com);
    hSerial = CreateFile(portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
	//Check if the connection was successfull
	if(hSerial==INVALID_HANDLE_VALUE)
	{
		//If not success full display an Error
		if(GetLastError()==ERROR_FILE_NOT_FOUND){
            //Print Error if neccessary
			printf("ERROR: Handle was not attached. Reason: %s not available.\n", portName);
		}
		else
		{
			printf("ERROR!!!");
		}
	}
	else
	{
		//If connected we try to set the comm parameters
		DCB dcbSerialParams = {0};
		//Try to get the current
		if (!GetCommState(hSerial, &dcbSerialParams))
		{
			//If impossible, show an error
			printf("failed to get current serial parameters!");
		}
		else
		{
			//Define serial connection parameters for the arduino board
			dcbSerialParams.BaudRate=CBR_115200;
			dcbSerialParams.ByteSize=8;
			dcbSerialParams.StopBits=ONESTOPBIT;
			dcbSerialParams.Parity=NOPARITY;

			//Set the parameters and check for their proper application
			if(!SetCommState(hSerial, &dcbSerialParams))
			{
				printf("ALERT: Could not set Serial Port parameters");
			}
			else
			{
				//If everything went fine we're connected
				connected = 1;
				//We wait 2s as the arduino board will be reseting
				Sleep(ARDUINO_WAIT_TIME);
			}
		}
	}
	FlushFileBuffers(hSerial);
	if (hSerial)
        return 1;
    else
        return -1;
}

uint8 send_command(char *str)
{
    DWORD bytesSend, errors;
    COMSTAT status;
	if(!WriteFile(hSerial, (void *)str, strlen(str), &bytesSend, 0))
	{
		//In case it don't work get comm error and return false
		ClearCommError(hSerial, &errors, &status);

		return false;
	}
	else
		return true;
}

void recv_data(char *str)
{

}

unsigned char hex2byte(char *str, int pos)
{
    char *ptr = str + pos;
    unsigned char val, p1, p2;
    p1 = *ptr;
    p2 = *(ptr+1);
    val = 16 * (p1 >= '0' && p1 <= '9' ? p1 - '0' : p1 >= 'A' && p1 <= 'F' ? p1 - 'A' + 10 : p1 >= 'a' && p1 <= 'f' ? p1 - 'a' + 10 : 0);
    val += (p2 >= '0' && p2 <= '9' ? p2 - '0' : p2 >= 'A' && p2 <= 'F' ? p2 - 'A' + 10 : p2 >= 'a' && p2 <= 'f' ? p2 - 'a' + 10 : 0);
    return val;
}

uint8 sd_init()
{
    char str[256];
    sprintf(str, "X%02x\n", SDINIT);
    send_command(str);
    recv_data(str);
    return hex2byte(str, 1);
}
