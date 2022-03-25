#ifndef _HL7692_DRIVER_H_
#define _HL7692_DRIVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* include serial lib */
#include "rwserial.h"
/* for sleep */
#include <unistd.h>

extern int initHL7692();

extern int turnOnLed();

extern int turnOffLed();

extern int configureNetwork();

extern int connect();

extern int disconnect();

extern int sendRecvApdu(int sessionId, const char* pDataSend, char** ppDataReceived);

#endif  // _HL7692_DRIVER_H_