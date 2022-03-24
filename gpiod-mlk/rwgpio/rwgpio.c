#include <linux/gpio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <syslog.h> //Write log to linux's syslog

#include "rwgpio.h"

#define LOG_NAME "rwgpio_c"

#define openGpioChipReadOnly(a) openGpioChip(a, O_RDONLY) 
#define openGpioChipWriteOnly(a) openGpioChip(a, O_WRONLY)
#define openGpioChipReadWrite(a) openGpioChip(a, O_RDWR)

/* //Log To stdout
#define printLog1(a) {printf("\n");printf(TAG_LOG_NAME);printf(": ");printf(a);}
#define printLog2(a,b) {printf("\n");printf(TAG_LOG_NAME);printf(": ");printf(a,b);}
#define printLog3(a,b,c) {printf("\n");printf(TAG_LOG_NAME);printf(": ");printf(a,b,c);}
*/

//log to syslog
//command = journalctl | grep rwgpio_c
#define printLog1(a) syslog(LOG_DEBUG, a)
#define printLog2(a,b) syslog(LOG_DEBUG, a, b)
#define printLog3(a,b,c) syslog(LOG_DEBUG, a, b, c)

#define True 1
#define False 0


static int openGpioChip(const char * devPath, int oflag)
{  
    int fd = -1;
    fd = open(devPath, oflag);
    return (fd);
}

unsigned char writeToPin(const char *devPath, unsigned int pinNumber, unsigned char value)
{
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    data.values[0] = value;

    req.lineoffsets[0] = pinNumber;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;


    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog (LOG_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    int fd = openGpioChipWriteOnly(devPath);
    if (fd >= 0)
    {
        //Open Chip Success
        //write to it then close the chip
        if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) == -1)
        {
            printLog2("Unable to line handle from ioctl : %s", strerror(errno));
            close(fd);
            closelog();
            return (False);
        }
        else if (ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) == -1)
        {
            printLog2("Unable to set line value using ioctl : %s", strerror(errno));
            close(req.fd);
            close(fd);
            closelog();
            return (False);
        }

        close(req.fd);
        close(fd);
        closelog();
        return (True);
    }

    printLog3("Unabled to open %s: %s", devPath, strerror(errno));
    closelog();
    return (False);
}

unsigned char readFromPin(const char *devPath, unsigned int pinNumber, unsigned char *receiveBuffer)
{
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    req.lineoffsets[0] = pinNumber;
    req.lines = 1;
    req.flags = GPIOHANDLE_REQUEST_INPUT;

    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog (LOG_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    int fd = openGpioChipReadOnly(devPath);
    if (fd >= 0)
    {
        //Open Chip Success
        //write to it then close the chip
        if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) == -1)
        {
            printLog2("Unable to line handle from ioctl : %s", strerror(errno));
            close (fd);
            closelog();
            return False;
        }
        else if (ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) == -1)
        {
            printLog2("Unable to set line value using ioctl : %s", strerror(errno));
            close (req.fd);
            close (fd);
            closelog();
            return False;
        }
        else
        {
            *(receiveBuffer) = data.values[0];
            close(req.fd);
            close(fd);
            closelog();
            return True;
        }
        
    }

    printLog3("Unabled to open %s: %s", devPath, strerror(errno));
    closelog();
    return False;
}

