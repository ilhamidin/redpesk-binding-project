//To run this lib, user must be part of dialout group:
//sudo adduser $USER dialout
//You must log out and back in before these group changes come into effect.

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <syslog.h> //Write log to linux's syslog

#include "rwserial.h"

#define LOG_NAME "rwserial_c"

#define openSerialDeviceReadOnly(a) openSerialDevice(a, O_RDONLY) 
#define openSerialDeviceWriteOnly(a) openSerialDevice(a, O_WRONLY)
#define openSerialDeviceReadWrite(a) openSerialDevice(a, O_RDWR)

/* //Log To stdout
#define printLog1(a) {printf("\n");printf(TAG_LOG_NAME);printf(": ");printf(a);}
#define printLog2(a,b) {printf("\n");printf(TAG_LOG_NAME);printf(": ");printf(a,b);}
#define printLog3(a,b,c) {printf("\n");printf(TAG_LOG_NAME);printf(": ");printf(a,b,c);}
*/

//log to syslog
//command = journalctl | grep rwserial_c
#define printLog1(a) syslog(LOG_DEBUG, a)
#define printLog2(a,b) syslog(LOG_DEBUG, a, b)
#define printLog3(a,b,c) syslog(LOG_DEBUG, a, b, c)

#define True 1
#define False 0
#define OPERATION_FAILED 0
#define OPERATION_SUCCESS 1

static int openSerialDevice(const char * devPath, int oflag)
{
    int fd = -1;
    fd = open(devPath, oflag);
    return (fd);
}

serialOperationStatus sendAndReceive(const char *devPath, serialCommandData *commandData, char* responseBuffer)
{
    struct termios tty, oldtty;

    serialOperationStatus status;
    status.operationIsSuccess = OPERATION_FAILED;
    status.errorCode = 0;
    status.numDataWritten = -1;
    status.numDataRead = -1;

    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog (LOG_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    int fd = openSerialDeviceReadWrite(devPath);
    if (fd >= 0)
    {
        //Read existing serial port setting (backup old setting)
        // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
        // must have been initialized with a call to tcgetattr() overwise behaviour
        // is undefined
        if (tcgetattr(fd, &oldtty) != 0)
        {
            printLog2("Error executing tcgetattr: %s", strerror(errno));
            status.errorCode = errno;
            close(fd);
            closelog();
            return (status);
        }
        else{
            //struct termios successfully initialized.
            //now we configure it to our need

            //clear struct for new port settings
            bzero(&tty, sizeof(tty));

            //The c_cflag member of the termios struct contains control parameter fields.
            //Clear parity bit (disable parity)
            tty.c_cflag &= ~PARENB;
            //Clear stop field, only one stop bit used in communication
            tty.c_cflag &= ~CSTOPB;
            //8 bits per byte
            tty.c_cflag |= CS8;
            //Disable RTS/CTS hardware flow control
            tty.c_cflag &= ~CRTSCTS;
            //Setting CLOCAL disables modem-specific signal lines such as carrier detect. It also prevents the controlling process from getting sent a SIGHUP signal when a modem disconnect is detected.
            //Setting CREAD allows us to read data.
            tty.c_cflag |= CLOCAL|CREAD;

            //Local Modes (c_lflag)
            //In canonical mode, input is processed when a new line character is received. The receiving application receives that data line-by-line.
            //Also, in canonical mode, some characters such as backspace are treated specially, and are used to edit the current line of text (erase).
            //We enable it
            tty.c_lflag &= ~ICANON;
            //If this bit is set, sent characters will be echoed back.
            tty.c_lflag &= ~ECHO; //Disable Echo
            tty.c_lflag &= ~ECHOE; //Disable Erasure
            tty.c_lflag &= ~ECHONL; //Disable New Line Echo
            //When the ISIG bit is set, INTR, QUIT and SUSP characters are interpreted.
            //We disable it
            tty.c_lflag &= ~ISIG;

            //The c_iflag member of the termios struct contains low-level settings for input processing.
            //Clearing IXOFF, IXON and IXANY disables software flow control. 
            //We Turn off sw flow ctrl
            tty.c_iflag &= ~(IXON|IXOFF|IXANY);
            //Clearing all of the following bits disables any special handling of the bytes as they are received by the serial port, before they are passed to the application. 
            //We just want the raw data, so we clear them all.
            tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
            //tty.c_iflag = IGNPAR | ICRNL;

            //The c_oflag member of the termios struct contains low-level settings for output processing.
            // Prevent special interpretation of output bytes (e.g. newline chars)
            tty.c_oflag &= ~OPOST;
            // Prevent conversion of newline to carriage return/line feed
            tty.c_oflag &= ~ONLCR;
            //tty.c_oflag = 0;

            //VMIN and VTIME (c_cc)
            //VMIN = 0, VTIME = 0 ---> No blocking, return immediately with what is available.
            //VMIN > 0, VTIME = 0 ---> This will make read() always wait for bytes (exactly how many is determined by VMIN), so read() could block indefinitely.
            //VMIN = 0, VTIME > 0 ---> This is a blocking read of any number of chars with a maximum timeout (given by VTIME). read() will block until either any amount of data is available, or the timeout occurs.
            //VMIN > 0, VTIME > 0 ---> Block until either VMIN characters have been received, or VTIME after first character has elapsed. Note that the timeout for VTIME does not begin until the first character is received.
            tty.c_cc[VMIN] = 4; tty.c_cc[VTIME] = 30;
            //tty.c_cc[VMIN] = 3; tty.c_cc[VTIME] = 0;

            tty.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
            tty.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
            tty.c_cc[VERASE]   = 0;     /* del */
            tty.c_cc[VKILL]    = 0;     /* @ */
            tty.c_cc[VEOF]     = 4;     /* Ctrl-d */
            tty.c_cc[VSWTC]    = 0;     /* '\0' */
            tty.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
            tty.c_cc[VSTOP]    = 0;     /* Ctrl-s */
            tty.c_cc[VSUSP]    = 0;     /* Ctrl-z */
            tty.c_cc[VEOL]     = 0;     /* '\0' */
            tty.c_cc[VREPRINT] = 0;     /* Ctrl-r */
            tty.c_cc[VDISCARD] = 0;     /* Ctrl-u */
            tty.c_cc[VWERASE]  = 0;     /* Ctrl-w */
            tty.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
            tty.c_cc[VEOL2]    = 0;     /* '\0' */

            //Set in/out baud rate to be 115200
            cfsetispeed(&tty, B115200);
            cfsetospeed(&tty, B115200);

            //now clean the modem line and activate the settings for the port
            tcflush(fd, TCIFLUSH);

            //After changing all settings above, we save it.
            // Save tty settings, also checking for error
            if (tcsetattr(fd, TCSANOW, &tty) != 0)
            {
                printLog2("Error Saving the serial port settings: %s\n", strerror(errno));
                status.errorCode = errno;
                close (fd);
                closelog ();
                return (status);
            }
            else{
                //Settings are successfully saved. Now we can Read/Write data.
                //We send the data first, then get the reply
                printLog2("Sending to Serial Port %d bytes: ", commandData->commandLength);
                printLog3("%*s", commandData->commandLength, commandData->commandBuffer+commandData->startOffset);
                status.numDataWritten =  write(fd, commandData->commandBuffer+commandData->startOffset, commandData->commandLength);
                tcdrain(fd);

                if (status.numDataWritten<0)
                {
                    printLog2("Write Failed: %s", strerror(errno));
                    status.errorCode = errno;
                    close(fd);
                    closelog ();
                    return (status);
                }
                printLog2("Sent %d bytes.", status.numDataWritten);

                status.numDataRead = read(fd, responseBuffer, commandData->maxExpectedRespSize);
                
                if (status.numDataRead < 0)
                {
                    printLog2("Error receiving data from serial port: %s", strerror(errno));
                    status.errorCode = errno;
                    //restore old serial setting
                    tcsetattr(fd,TCSANOW,&oldtty);
                    close(fd);
                    closelog ();
                    return (status);
                }
                else{
                    printLog2("Read %d bytes. Received message is: ", status.numDataRead);
                    for (int i=0; i<status.numDataRead; i++)
                    {
                       // printf("\n%x => %c", responseBuffer[i], responseBuffer[i]);
                        printLog3("0x%02x => %c", responseBuffer[i], responseBuffer[i]);
                    }
                    printLog1("\n");
                    status.errorCode = 0;
                    status.operationIsSuccess = OPERATION_SUCCESS;
                    //restore old serial setting
                    tcsetattr(fd,TCSANOW,&oldtty);
                    close(fd);
                    closelog ();
                    return (status);
                }

            }

        }
    }
    printLog3("Unabled to open %s: %s", devPath, strerror(errno));
    status.errorCode = errno;
    closelog ();
    return (status);
}