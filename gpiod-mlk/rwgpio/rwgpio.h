// example devPath: "/dev/gpiochip0"
unsigned char writeToPin(const char *devPath, unsigned int pinNumber, unsigned char value);
unsigned char readFromPin(const char *devPath, unsigned int pinNumber, unsigned char *receiveBuffer);
