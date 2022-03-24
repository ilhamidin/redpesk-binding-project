typedef struct s_serialOperationStaus
{
    unsigned char operationIsSuccess; //1 if Operation is success, 0 otherwise;
    int errorCode; //if Opration is NOT success, this contains the error code
    int numDataWritten; //Number of bytes Written To serial port
    int numDataRead; //Number of bytes Read From serial port
} serialOperationStatus;

typedef struct s_serialCommand
{
    const char *commandBuffer;
    unsigned int startOffset;
    unsigned int commandLength;
    unsigned int maxExpectedRespSize; //This is the max. Will be indicated by serialOperationStatus.numDataRead,If the data read is less than the max,.
} serialCommandData;

//Returns 1 on OK and 0 on Error
serialOperationStatus sendAndReceive(const char *devPath, serialCommandData* commandData, char *responseBuffer);
