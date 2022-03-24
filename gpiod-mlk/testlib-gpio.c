#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "rwgpio/rwgpio.h"

//COMPILE: gcc -fPIC testlib-gpio.c gpio/rwgpio.c -o testlib-gpio $(pkg-config --cflags)

typedef struct HW_STATUS {
    int status;        // 0 means OK, 1 means not OK
    int dataReturned;  // if the status is 0, we put the data we read from the sensor to here
} HWStatus;

#define HW_STATUS_OK 0
#define HW_STATUS_NOK 1

static const char devicePath[] = "/dev/gpiochip0";

HWStatus HW_HANDLER_turnLedOn() {
    HWStatus hwStatus;
    hwStatus.status = HW_STATUS_NOK;
    // Real Handling for hardware goes into here
    // For instance, we read a value from the sensor and we get 12345. So we returns 12345 from this method

    if ( writeToPin(devicePath, 16, 1) )
    {
        //Led successfully turned on
        hwStatus.status = HW_STATUS_OK;
        hwStatus.dataReturned = 12345;
    }
     
    return hwStatus;
}

HWStatus HW_HANDLER_turnLedOff() {
    HWStatus hwStatus;
    hwStatus.status = HW_STATUS_NOK;
    // Real Handling for hardware goes into here
    // For instance, we read a value from the sensor and we get 67890. So we returns 67890 from this method

    if ( writeToPin(devicePath, 16, 0) )
    {
        //Led successfully turned off
        hwStatus.status = HW_STATUS_OK;
        hwStatus.dataReturned = 67890;
    }
     
    return hwStatus;
}

int main(int argc, char ** argv)
{
    HWStatus stat;
    stat.status = HW_STATUS_NOK;

    int val;
    int keeploop = 1;
    while (keeploop)
    {
        printf( "Enter a value. 1 turnon lamp. 0 turnoff. others Quit: ");
        scanf("%d", &val);
        switch (val)
        {
            case 1:
                stat = HW_HANDLER_turnLedOn();
                if (stat.status == HW_STATUS_OK)
                {
                    printf ("\nDONE OKAY\n");
                }
                else{
                    printf ("\nNOOO. FAILED !\n");
                }
                break;
            case 0:
                stat = HW_HANDLER_turnLedOff();
                if (stat.status == HW_STATUS_OK)
                {
                    printf ("\nDONE OKAY\n");
                }
                else{
                    printf ("\nNOOO. FAILED !\n");
                }
                break;
            default:
            keeploop = 0;
            break;
        }
    }
    printf("\nQuit now\n");
   return 0; 
}
