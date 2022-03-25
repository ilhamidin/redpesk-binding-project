#include "HL7692-Driver.h"

/* var */
#define OK 1;
#define NOK 0;

static const char devSerialPath[] = "/dev/ttyAMA0";
static const unsigned int RECEIVE_BUFFER_MAX_LEN = 257;

static const char CMD_LED_ON[] = "AT+KGPIO=8,1\r\n";
static const char CMD_LED_OFF[] = "AT+KGPIO=8,0\r\n";
static const char CMD_LED_NETWORK_ON[] = "AT+KSYNC=2,7\r\n";
static const char CMD_LED_NETWORK_OFF[] = "AT+KSYNC=0\r\n";
/* config setting */
static const char CMD_AT_BASIC[] = "AT\r\n";
static const char CMD_ECHO_ON[] = "ATE1\r\n";
static const char CMD_ECHO_OFF[] = "ATE0\r\n";
static const char CMD_USB_TO_ETH[] = "AT+KUSBCOMP=2\r\n";
/* hardware setting */
static const char CMD_REBOOT[] = "AT+CFUN=1,1\r\n";
/* network setting */
static const char CMD_SET_APN[] = "AT+CGDCONT=1,\"IP\",\"internet\"\r\n";
static const char CMD_SET_APN_USERNAME_PASSWD[] = "AT+WPPP=1,1,\"username\",\"password\"\r\n";
static const char CMD_SET_DYNAMIC_DNS_REQ[] = "AT+XDNS=1,1\r\n";
static const char CMD_START_IP_SESSION[] = "AT+XCEDATA=1,0\r\n";
static const char CMD_STOP_IP_SESSION[] = "AT+CGACT=0,1\r\n";

#define sizeofCmd(a) (sizeof a / sizeof *a)
#define RECEIVE_BUFFER_MAX_LEN 257

char receiveBuffer[RECEIVE_BUFFER_MAX_LEN];

serialCommandData commandData;
serialOperationStatus operationStatus;

/* declare private function */
int preConfigBeforeConnect();
int configBeforeConnect();
serialOperationStatus initializeAndSendCommand(const char* commandBuffer, unsigned int startOffset, unsigned int commandLength);

/* Accessible Function */

/* This function is to
    1. check if modem is ok, with send command "AT"
    2. disable echo
*/
int initHL7692() {
    // check functionality

    /* send basic command */
    operationStatus = initializeAndSendCommand(CMD_AT_BASIC, 0, sizeofCmd(CMD_AT_BASIC));
    /* filter the ret value */
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    /* send command echo off */
    operationStatus = initializeAndSendCommand(CMD_ECHO_OFF, 0, sizeofCmd(CMD_ECHO_OFF));
    /* filter the ret value */
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    return 0;
}

int turnOnLed() {
    operationStatus = initializeAndSendCommand(CMD_LED_ON, 0, sizeofCmd(CMD_LED_ON));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    return 0;
}

int turnOffLed() {
    operationStatus = initializeAndSendCommand(CMD_LED_OFF, 0, sizeofCmd(CMD_LED_OFF));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    return 0;
}

int configureNetwork() {
    int result;
    result = preConfigBeforeConnect();
    switch (result) {
        case 0:
            break;
        case 1:
            return 1;
        case 2:
            return 2;
        case 3:
            return 3;
        default:
            return 4;
            break;
    }

    result = configBeforeConnect();
    switch (result) {
        case 0:
            break;
        case 1:
            return 1;
        case 2:
            return 2;
        case 3:
            return 3;
        default:
            return 4;
            break;
    }
    return 0;
}

int connect() {
    operationStatus = initializeAndSendCommand(CMD_START_IP_SESSION, 0, sizeofCmd(CMD_START_IP_SESSION));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    return 0;
}

int disconnect() {
    operationStatus = initializeAndSendCommand(CMD_STOP_IP_SESSION, 0, sizeofCmd(CMD_STOP_IP_SESSION));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    return 0;
}

int sendRecvApdu(int sessionId, const char* pDataSend, char** ppDataReceived) {
    /* filter if data send and data received null */
    // if(pDataSend == NULL || ppDataReceived == NULL){
    //     return ;
    // }
    /* reset received variable char array */
    memset(receiveBuffer, 0, RECEIVE_BUFFER_MAX_LEN);

    /*  */
    int len = snprintf(NULL, 0, "AT+CGLA=%d,%ld,\"%s\"\r\n", sessionId, strlen(pDataSend), pDataSend);
    char* buf = malloc(len + 1);
    snprintf(buf, len + 1, "AT+CGLA=%d,%ld,\"%s\"\r\n", sessionId, strlen(pDataSend), pDataSend);

    /* start send */
    operationStatus = initializeAndSendCommand(buf, 0, strlen(buf));
    if (!operationStatus.operationIsSuccess) {
        /* free malloc */
        free(buf);
        buf = NULL;
        return operationStatus.errorCode;
    }

    /* free malloc */
    free(buf);
    buf = NULL;

    /* */
    char* pReceiveBuffer = &receiveBuffer[0];
    char* pDataReceived = strdup(pReceiveBuffer);
    *ppDataReceived = pDataReceived;
    return 0;
}

/* private function */

/* This function is to
    1. activate left LED in modem for network status
    2. change USB connection to ETH connection (device known as "wwan0")
    3. reboot the module, in order step 2 to take effect
*/
int preConfigBeforeConnect() {
    operationStatus = initializeAndSendCommand(CMD_LED_NETWORK_ON, 0, sizeofCmd(CMD_LED_NETWORK_ON));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    operationStatus = initializeAndSendCommand(CMD_USB_TO_ETH, 0, sizeofCmd(CMD_USB_TO_ETH));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    operationStatus = initializeAndSendCommand(CMD_REBOOT, 0, sizeofCmd(CMD_REBOOT));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    return 0;
}

/* This function is to
    1. set APN for sim card
    2. set dynamic DNS request
*/
int configBeforeConnect() {
    operationStatus = initializeAndSendCommand(CMD_SET_APN, 0, sizeofCmd(CMD_SET_APN));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    operationStatus = initializeAndSendCommand(CMD_SET_DYNAMIC_DNS_REQ, 0, sizeofCmd(CMD_SET_DYNAMIC_DNS_REQ));
    if (!operationStatus.operationIsSuccess) {
        return operationStatus.errorCode;
    }

    return 0;
}

serialOperationStatus initializeAndSendCommand(const char* commandBuffer, unsigned int startOffset, unsigned int commandLength) {
    commandData.commandBuffer = commandBuffer;
    commandData.startOffset = startOffset;
    commandData.commandLength = commandLength;
    commandData.maxExpectedRespSize = RECEIVE_BUFFER_MAX_LEN;
    return (sendAndReceive(devSerialPath, &commandData, receiveBuffer));
}
