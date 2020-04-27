#define FOSC 16000000 // System Clock speed
#define BAUD 9600 // Desired baud rate
#define MYUBRR (FOSC/16/BAUD-1) // Actual baud rate

// hardware function prototypes
void USART_Init(unsigned int ubrr); // USART serial setup (half-duplex async mode)
void USART_Transmit(unsigned char data); // Transmit a char in serial
unsigned char USART_Recieve(void); // Recieve a char from serial
void EEPROM_Write(unsigned int uiAddress, unsigned char uiData); // Write char to specified address
unsigned char EEPROM_Read(unsigned int uiAddress);  // Read char from specified address

// control table function prototypes
unsigned int modelNumber(unsigned int, bool);
unsigned int versionNumber(unsigned int, bool);
unsigned int ID(unsigned int, bool);
unsigned int baudRate(unsigned int, bool);
unsigned int returnDelayTime(unsigned int, bool);
unsigned int cwAngleLimit(unsigned int, bool);
unsigned int ccwAngleLimit(unsigned int, bool);
unsigned int temperatureLimit(unsigned int, bool);
unsigned int minVoltageLimit(unsigned int, bool);
unsigned int maxVoltageLimit(unsigned int, bool);
unsigned int maxTorque(unsigned int, bool);
unsigned int statusReturnLevel(unsigned int, bool);
unsigned int alarmLED(unsigned int, bool);
unsigned int shutdown(unsigned int, bool);
unsigned int torqueEnable(unsigned int, bool);
unsigned int LED(unsigned int, bool);
unsigned int cwComplianceMargin(unsigned int, bool);
unsigned int ccwComplianceMargin(unsigned int, bool);
unsigned int cwComplianceSlope(unsigned int, bool);
unsigned int ccwComplianceSlope(unsigned int, bool);
unsigned int goalPosition(unsigned int, bool);
unsigned int movingSpeed(unsigned int, bool);
unsigned int torqueLimit(unsigned int, bool);
unsigned int presentPosition(unsigned int, bool);
unsigned int presentSpeed(unsigned int, bool);
unsigned int presentLoad(unsigned int, bool);
unsigned int presentVoltage(unsigned int, bool);
unsigned int presentTemperature(unsigned int, bool);
unsigned int registered(unsigned int, bool);
unsigned int moving(unsigned int, bool);
unsigned int lock(unsigned int, bool);
unsigned int punch(unsigned int, bool);

// main function prototypes


int main(void)
   {
     //Initialise arduino ( so millis, delay work ).
     init();
#if defined(USBCON)
	USB.attach();
#endif

    // control table function * array setup
    unsigned int (*controlTable[60])(unsigned int, bool);
    controlTable[0] = modelNumber;
    controlTable[2] = versionNumber;
    controlTable[3] = ID;
    controlTable[4] = baudRate;
    controlTable[5] = returnDelayTime;
    controlTable[6] = cwAngleLimit;
    controlTable[8] = ccwAngleLimit;
    controlTable[11] = temperatureLimit;
    controlTable[12] = minVoltageLimit;
    controlTable[13] = maxVoltageLimit;
    controlTable[14] = maxTorque;
    controlTable[16] = statusReturnLevel;
    controlTable[17] = alarmLED;
    controlTable[18] = shutdown;
    controlTable[24] = torqueEnable;
    controlTable[25] = LED;
    controlTable[26] = cwComplianceMargin;
    controlTable[27] = ccwComplianceMargin;
    controlTable[28] = cwComplianceSlope;
    controlTable[29] = ccwComplianceSlope;
    controlTable[30] = goalPosition;
    controlTable[32] = movingSpeed;
    controlTable[34] = torqueLimit;
    controlTable[36] = presentPosition;
    controlTable[38] = presentSpeed;
    controlTable[40] = presentLoad;
    controlTable[42] = presentVoltage;
    controlTable[43] = presentTemperature;
    controlTable[44] = registered;
    controlTable[46] = moving;
    controlTable[47] = lock;
    controlTable[48] = punch;

    // USART serial setup (Half-duplex async mode)
    USART_Init(controlTable[4](0, false));

     //Start loop
     while(true)
     {

       if (serialEventRun) serialEventRun();
     }
     return 0;
   }

void USART_Init(unsigned int ubrr)
{
  // Set baud rate
  UBRR0H = (unsigned char) ubrr>>8;
  UBRR0L = (unsigned char) ubrr;
  // enable tx and rx
  UCSR0B = (1<<RXEN0) | (1<<TXEN0);
  // set frame format (1 stop bit, 8 data bits)
  UCSR0C = 3<<UCSZ00;
}

void USART_Transmit(unsigned char data)
{
  // Wait for empty transmit buffer
  while (!( UCSR0A & (1<<UDRE0)));

  UDR0 = data;
}

unsigned char USART_Recieve(void)
{
  // Wait for recieve signal
  while (!( UCSR0A & (1<<RXC0)));

  // Get and return recieved data from buffer
  return UDR0;
}

void EEPROM_Write(unsigned int uiAddress, unsigned char uiData)
{
  // Wait for last write operation to complete
  while (EECR & (1<<EEPE));

  // Set up address and data registers
  EEAR = uiAddress;
  EEDR = uiData;

  // Write logical 1 to EEMPE
  EECR |= (1<<EEMPE);
  // Start EEPROM write by setting EEPE
  EECR |= (1<<EEPE);
}

unsigned char EEPROM_Read(unsigned int uiAddress)
{
  // Wait for last read/write operation to complete
  while (EECR & (1<<EEPE));

  // Set up address register
  EEAR = uiAddress;
  // Start EEPROM read by setting EEPE
  EECR |= (1<<EERE);
  // Return data register
  return EEDR;
}
unsigned int modelNumber(unsigned int, bool)
{

}
unsigned int versionNumber(unsigned int, bool)
{
  
}
unsigned int ID(unsigned int, bool)
{
  
}
unsigned int baudRate(unsigned int, bool)
{
  
}
unsigned int returnDelayTime(unsigned int, bool)
{
  
}
unsigned int cwAngleLimit(unsigned int, bool)
{
  
}
unsigned int ccwAngleLimit(unsigned int, bool)
{
  
}
unsigned int temperatureLimit(unsigned int, bool)
{
  
}
unsigned int minVoltageLimit(unsigned int, bool)
{
  
}
unsigned int maxVoltageLimit(unsigned int, bool)
{
  
}
unsigned int maxTorque(unsigned int, bool)
{
  
}
unsigned int statusReturnLevel(unsigned int, bool)
{
  
}
unsigned int alarmLED(unsigned int, bool)
{
  
}
unsigned int shutdown(unsigned int, bool)
{
  
}
unsigned int torqueEnable(unsigned int, bool)
{
  
}
unsigned int LED(unsigned int, bool)
{
  
}
unsigned int cwComplianceMargin(unsigned int, bool)
{
  
}
unsigned int ccwComplianceMargin(unsigned int, bool)
{
  
}
unsigned int cwComplianceSlope(unsigned int, bool)
{
  
}
unsigned int ccwComplianceSlope(unsigned int, bool)
{
  
}
unsigned int goalPosition(unsigned int, bool)
{
  
}
unsigned int movingSpeed(unsigned int, bool)
{
  
}
unsigned int torqueLimit(unsigned int, bool)
{
  
}
unsigned int presentPosition(unsigned int, bool)
{
  
}
unsigned int presentSpeed(unsigned int, bool)
{
  
}
unsigned int presentLoad(unsigned int, bool)
{
  
}
unsigned int presentVoltage(unsigned int, bool)
{
  
}
unsigned int presentTemperature(unsigned int, bool)
{
  
}
unsigned int registered(unsigned int, bool)
{
  
}
unsigned int moving(unsigned int, bool)
{
  
}
unsigned int lock(unsigned int, bool)
{
  
}
unsigned int punch(unsigned int, bool)
{
  
}