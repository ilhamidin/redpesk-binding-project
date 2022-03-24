#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rwgpio/rwgpio.h"
#include "rwserial/rwserial.h"

#define RECEIVE_BUFFER_MAX_LEN 257
#define sizeofCmd(a) (sizeof a / sizeof *a)

static const char devGpioPath[] = "/dev/gpiochip0";
static const char devSerialPath[] = "/dev/ttyAMA0";

static const char AT_CMD_MODEM_LED_ON[] = "at+kgpio=8,1\r";
static const char AT_CMD_MODEM_LED_OFF[] = "at+kgpio=8,0\r";

char receiveBuffer[RECEIVE_BUFFER_MAX_LEN];

int main(int argc, char ** argv)
{
    int inputLength = 0;
    char val;
    int keeploop = 1;

    //Define response var
    serialOperationStatus operationStatus;
    //Define  command data
    serialCommandData commandData;
    commandData.maxExpectedRespSize = RECEIVE_BUFFER_MAX_LEN;
    commandData.startOffset = 0;
               
    while (keeploop)
    {
        printf( "\nEnter a value.\n0 Turn On the Modem.\n1 Turn ON modem LEd.\n2 Turn Off Modem LEd.\n Others Quit:");
        //scanf("%c", &val);
        scanf("%c", &val);
        printf("\nchoose %c", (char)val);
        switch ((char)val)
        {
            case '0':
                if ( ! writeToPin(devGpioPath, 6, 1) )
                {
                    printf("\nTESTLIB Write 1 to GPIO 6 Failed. ");
                }
                else if ( ! writeToPin(devGpioPath, 21, 1) )
                {
                    printf("\nTESTLIB Write 1 to GPIO 21 Failed. ");
                }
                else if ( ! writeToPin(devGpioPath, 21, 0) )
                {
                    printf("\nTESTLIB Write 0 to GPIO 21 Failed. ");
                }
                else
                {
                    printf("\nTESTLIB OKAY. Modem is Turned on. ");
                }
                break;
            case '1':
                commandData.commandBuffer = AT_CMD_MODEM_LED_ON;
                commandData.commandLength = sizeofCmd(AT_CMD_MODEM_LED_ON);
                operationStatus =  sendAndReceive(devSerialPath, &commandData, receiveBuffer);

                if (! operationStatus.operationIsSuccess )
                {
                     printf("\nTESTLIB Turn On LED Modem Failed. ");
                     printf("\nTESTLIB Received error code %d . ", operationStatus.errorCode); //Error code can be nagative?
                }
                else{
                    printf("\nTESTLIB OKAY. Turn On LED Modem Done. ");
                    printf("\nTESTLIB Received %i bytes from the serial port, data is: ", operationStatus.numDataRead);
                    for (int i=0; i<operationStatus.numDataRead; i++)
                    {
                        printf("%c", receiveBuffer[i]);
                    }
                }
                break;
            case '2':
                commandData.commandBuffer = AT_CMD_MODEM_LED_OFF;
                commandData.commandLength = sizeofCmd(AT_CMD_MODEM_LED_OFF);
                operationStatus =  sendAndReceive(devSerialPath, &commandData, receiveBuffer);

                if (! operationStatus.operationIsSuccess )
                {
                     printf("\nTESTLIB Turn Off LED Modem Failed. ");
                     printf("\nTESTLIB Received error code %d . ", operationStatus.errorCode); //Error code can be nagative?
                }
                else{
                    printf("\nTESTLIB OKAY. Turn Off LED Modem Done. ");
                    printf("\nTESTLIB Received %i bytes from the serial port, data is: ", operationStatus.numDataRead);
                    for (int j=0; j<operationStatus.numDataRead; j++)
                    {
                        printf("%c", receiveBuffer[j]);
                    }
                }
                break;
                
            default:
            keeploop = 0;
            break;
        }
    }
    printf("\nTESTLIB Quit now\n");

   return 0; 
}