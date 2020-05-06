#define FOSC 16000000 // System Clock speed
#define BAUD 9600 // Desired baud rate
#define MYUBRR (FOSC/16/BAUD-1) // Actual baud rate

#include <util/delay.h>

// hardware function prototypes
void USART_Init(unsigned int ubrr); // USART serial setup (half-duplex async mode)
void USART_Transmit(unsigned char data); // Transmit a char in serial
unsigned char USART_Recieve(bool& succeeded); // Recieve a char from serial
void EEPROM_Write(unsigned int uiAddress, unsigned char uiData); // Write char to specified address
unsigned char EEPROM_Read(unsigned int uiAddress);  // Read char from specified address
void startTimer(void); // Function to set up the timer registers for byte-to-byte timeout

// EEPROM control table function prototypes
unsigned int modelNumber(unsigned int model, bool write);
unsigned int versionNumber(unsigned int version, bool write);
unsigned int ID(unsigned int ID, bool write);
unsigned int baudRate(unsigned int baudRate, bool write);
unsigned int returnDelayTime(unsigned int rdt, bool write);
unsigned int cwAngleLimit(unsigned int cwal, bool write);
unsigned int ccwAngleLimit(unsigned int ccwal, bool write);
unsigned int temperatureLimit(unsigned int temp, bool write);
unsigned int minVoltageLimit(unsigned int minvolt, bool write);
unsigned int maxVoltageLimit(unsigned int maxvolt, bool write);
unsigned int maxTorque(unsigned int maxt, bool write);
unsigned int statusReturnLevel(unsigned int srl, bool write);
unsigned int alarmLED(unsigned int alarm, bool write);
unsigned int shutdown(unsigned int shutdown, bool write);

// main function prototypes
unsigned char* instPacketReciever(bool& succeeded); // Waits for USART recieve signal and collects the instruction packet when transmission has begun
void statPacketTransmitter(unsigned char* statPacket); // Transmits the status packet back to the controller

int main(void)
   {
     //Initialise arduino ( so millis, delay work ).
     init();
#if defined(USBCON)
	USB.attach();
#endif

    // control table EEPROM function * array setup
    unsigned int (*controlTableEEPROM[24])(unsigned int, bool);
    controlTableEEPROM[0] = modelNumber;
    controlTableEEPROM[2] = versionNumber;
    controlTableEEPROM[3] = ID;
    controlTableEEPROM[4] = baudRate;
    controlTableEEPROM[5] = returnDelayTime;
    controlTableEEPROM[6] = cwAngleLimit;
    controlTableEEPROM[8] = ccwAngleLimit;
    controlTableEEPROM[11] = temperatureLimit;
    controlTableEEPROM[12] = minVoltageLimit;
    controlTableEEPROM[13] = maxVoltageLimit;
    controlTableEEPROM[14] = maxTorque;
    controlTableEEPROM[16] = statusReturnLevel;
    controlTableEEPROM[17] = alarmLED;
    controlTableEEPROM[18] = shutdown;

    // control Table RAM char array
    unsigned char controlTableROM[32];
    controlTableROM[0] = 0;
    controlTableROM[1] = 0;
    controlTableROM[2] = 4;
    controlTableROM[3] = 4;
    controlTableROM[4] = 64;
    controlTableROM[5] = 64;
    controlTableROM[6] = 0;
    controlTableROM[7] = 0;
    controlTableROM[8] = 0;
    controlTableROM[10] = controlTableEEPROM[14](0, false);
    controlTableROM[11] = 0;
    controlTableROM[12] = 0;
    controlTableROM[13] = 0;
    controlTableROM[14] = 0;
    controlTableROM[15] = 0;
    controlTableROM[16] = 0;
    controlTableROM[17] = 0;
    controlTableROM[18] = 0;
    controlTableROM[19] = 0;
    controlTableROM[20] = 0;
    controlTableROM[21] = 7;
    controlTableROM[22] = 0;
    controlTableROM[23] = 0;
    controlTableROM[24] = 32;

    // USART serial setup (Half-duplex async mode), call the control table function for it to initialize
    controlTableEEPROM[4](0, false);

    // other variables
    unsigned char* instPacket;
    bool succeeded;

    while(true) {

      // Wait to recieve the next instruction packet
      while (!( UCSR0A & (1<<RXC0)));

      // Get the instruction packet
      instPacket = instPacketReciever(succeeded);

      // If the reading had a byte-to-byte timeout, skip the rest on this loop iteration and wait for the next packet
      if (!succeeded)
      {
        delete instPacket;
        continue;
      }
       
      // Init status packet array
      unsigned char* statPacket = new unsigned char[100];
      statPacket[0] = 0xFF;
      statPacket[1] = 0xFF;
      statPacket[2] = controlTableEEPROM[3](0, false);

      // Init other variables
      unsigned char chksum;
      unsigned char j = 5;


      // This is an invalid Dynamixel Protocol 1 packet, as it's missing the header
      if (!((instPacket[0] == 255) && (instPacket[1] == 255)))
      {
        delete instPacket;
        delete statPacket;
        continue;
      }

      // This packet can be ignored, as its ID does not match ours or the global ID, so send a status packet with the correct error code
      if ((instPacket[2] != EEPROM_Read(3)) || (instPacket[2] != 0xFE))
      {
        // Set length to two
        statPacket[3] = 0x02;

        // Set instruction Error
        statPacket[4] = 0x40;

        // Construct and set Checksum
        statPacket[5] = statPacket[2] + statPacket[3] + statPacket[4];

        // Send Status Packet
        statPacketTransmitter(statPacket);

        delete instPacket;
        delete statPacket;
        continue;
      }

      // Generate checksum for compare
      chksum = instPacket[2] + instPacket[3] + instPacket[4];
      for (unsigned char i = instPacket[3] - 2; i > 0; --i)
      {
        chksum += instPacket[j];
        ++j;
      }

      // If the checksums don't match, the packet is invalid or has been corrupted, and we cannot use it
      if (chksum != instPacket[j])
      {
        // Set length to two
        statPacket[3] = 0x02;

        // Set checksum Error
        statPacket[4] = 0x10;

        // Construct and set Checksum
        statPacket[5] = statPacket[2] + statPacket[3] + statPacket[4];

        // Send Status Packet
        statPacketTransmitter(statPacket);

        delete instPacket;
        delete statPacket;
        continue;
      }

      // After all error checking has passed, we can use a switch statement to determine the instruction to execute
      switch (instPacket[4])
      {
        // Ping instruction
        case 0x01:
          
          // Set length to two
          statPacket[3] = 0x02;

          // Set error code to 0
          statPacket[4] = 0;

          // Construct and set Checksum
          statPacket[5] = statPacket[2] + statPacket[3] + statPacket[4];

          break;
        
        // Read Instruction
        case 0x02:

          // Set status packet length to 2 plus the number of bytes to be read
          statPacket[3] = 2 + instPacket[6];

          // Set error packet to zero
          statPacket[4] = 0x00;

          // Read the number of bytes specified in the instruction packet, and add the result to the status Packet
          for (unsigned char i = instPacket[5], j = 5; i < instPacket[5] + (instPacket[6] - 1); ++i, ++j)
          {
            // If the starting address is in ROM, then access the ROM control table with (address - 24)
            if (instPacket[5] >= 24)
            {
              statPacket[j] = controlTableROM[i - 24];
            }
            else // Else we can access the EEPROM directly
            {
              statPacket[j] = EEPROM_Read(i);
            }
          }

          // Generate and add checksum
          chksum = statPacket[2] + statPacket[3] + statPacket[4];
          for (unsigned char i = statPacket[3] - 2, j = 5; i > 0; --i, ++j)
          {
            chksum += statPacket[j];
          }
          statPacket[3 + statPacket[3]];
          
          break;
        
        // Write Instruction
        case 0x03:

          // Set packet length to 2
          statPacket[3] = 0x02;

          // Set error code to 0
          statPacket[4] = 0x00;

          // Write the number of bytes specified in the instruction packet starting at the specified address
          for (unsigned char i = instPacket[5], j = 5; i < instPacket[5] + (instPacket[3] - 3); ++i, ++j)
          {
            // If the starting address is in ROM, then access the ROM control table with (address - 24)
            if (instPacket[5] >= 24)
            {
              controlTableROM[i] = instPacket[j];
            }
            else // Else we can access the EEPROM directly
            {
              EEPROM_Write(i, instPacket[j]);

              // If the location we're writing to is the baudRate, we need to re-initialize the USART chip
              if (i == 0x04)
              {
                long baud = 0;
                // Switch statement to determine the 'actual' baud rate
                switch (instPacket[j])
                {
                  case 1:
                    baud = 1000000;
                    break;
                  
                  case 3:
                    baud = 500000;
                    break;

                  case 4:
                    baud = 400000;
                    break;

                  case 7:
                    baud = 250000;
                    break;

                  case 9:
                    baud = 200000;
                    break;

                  case 16:
                    baud = 115200;
                    break;

                  case 34:
                    baud = 57600;
                    break;

                  case 103:
                    baud = 19200;
                    break;

                  case 207:
                    baud = 9600;
                    break;
                  
                  default:
                    baud = 9600;
                    break;
                }

                // Re-initialize USART chip for (potentially) new baud rate
                USART_Init(FOSC/16/baud-1);                
              }
            }
          }

          // Generate and add checksum
          chksum = statPacket[2] + statPacket[3] + statPacket[4];
          for (unsigned char i = statPacket[3] - 2, j = 5; i > 0; --i, ++j)
          {
            chksum += statPacket[j];
          }
          statPacket[3 + statPacket[3]];
          
          break;

        // Reg Write Instruction
        case 0x04:
          break;

        // Action Instruction
        case 0x05:
          break;

        // Factory Reset Instruction
        case 0x06:
          break;

        // Sync Write Instruction
        case 0x83:
          break;
        
        // Instruction not defined
        default:
          break;
      }

      // If the ID on the instruction packet is 0xFE (254), then don't transmit a status packet
      if (instPacket[2] == 0xFE)
      {
        delete statPacket;
        delete instPacket;
        continue;
      }

      // If we've made it here, we've experience no errors, and can safely transmit the status packet
      statPacketTransmitter(statPacket);

      // Delete Allocated Memory
      delete statPacket;
      delete instPacket;

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

unsigned char USART_Recieve(bool& succeeded)
{
  // Start timer
  startTimer();

  // Wait for recieve signal
  while (!( UCSR0A & (1<<RXC0)))
  { 
    // If the timer overflow flag is set, set succeeded to false, and return 0
    if ((TIFR1 & (1<<OCF1A)) != 0)
    {
      // Reset Timer
		  TCNT1 = 0; 
		  TIFR1 |= (1<<OCF1A);

      succeeded = false;
      return 0;
    }
  }

  succeeded = true;

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

void startTimer(void)
{
  // Set prescaler as 1024, and begin timer
  TCCR1B = (1<<CS10) | (1<<CS12);
  // Count a 10HZ signal
  OCR1A = 1562;
  // set TCNT1 to 0
  TCNT1 = 0;
}

unsigned char* instPacketReciever(bool& succeeded)
{
  // set up return pointer
  unsigned char* returnPacket = new unsigned char[100];
  unsigned char j = 5;

  // read in known components and check if they failed
  returnPacket[0] = USART_Recieve(succeeded);
  if (!succeeded)
  {
    return returnPacket;
  }
  returnPacket[1] = USART_Recieve(succeeded);
  if (!succeeded)
  {
    return returnPacket;
  }
  returnPacket[2] = USART_Recieve(succeeded);
  if (!succeeded)
  {
    return returnPacket;
  }
  returnPacket[3] = USART_Recieve(succeeded);
  if (!succeeded)
  {
    return returnPacket;
  }
  returnPacket[4] = USART_Recieve(succeeded);
  if (!succeeded)
  {
    return returnPacket;
  }

  // Read in instruction parameters (# of parameters = length - 2)
  for (unsigned char i = returnPacket[3] - 2; i > 0 ; --i)
  {
    returnPacket[j] = USART_Recieve(succeeded);
    
    if (!succeeded)
    {
      return returnPacket;
    }

    ++j;
  }

  // read in checksum
  returnPacket[j] = USART_Recieve(succeeded);
  if (!succeeded)
  {
    return returnPacket;
  }

  // return packet
  return returnPacket;

}

// REWRITE to use the length of the status packet instead of the size of the array
void statPacketTransmitter(unsigned char* statPacket)
{
  unsigned char length = sizeof(statPacket)/sizeof(statPacket[0]);
  for (unsigned char i = 0; i < length; ++i)
  {
    USART_Transmit(statPacket[i]);
  }
}

unsigned int modelNumber(unsigned int model, bool write)
{
  // Since this is read-only ignore the write tag and access the EEPROM
  unsigned char firstByte = EEPROM_Read(0);
  unsigned char secondByte = EEPROM_Read(1);

  return (unsigned int) (8<<secondByte) | (firstByte);
  
}
unsigned int versionNumber(unsigned int version, bool write)
{
  // Since this is a read-only entry, Ignore the write tag and access the EEPROM
  return (unsigned int) EEPROM_Read(2);
}
unsigned int ID(unsigned int ID, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    ID = ID & 255;
    EEPROM_Write(3, ID);
  }

  // Read the specified EEPROM address
  return (unsigned int) EEPROM_Read(3);

}
unsigned int baudRate(unsigned int baudRate, bool write)
{
  long baud = 0;

  // Check write flag
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    baudRate = baudRate & 255;
    EEPROM_Write(4, baudRate);
  }

  // Read EEPROM
  unsigned char returnBaud = EEPROM_Read(4);

  // Switch statement to determine the 'actual' baud rate
  switch (returnBaud)
  {
    case 1:
      baud = 1000000;
      break;
    
    case 3:
      baud = 500000;
      break;

    case 4:
      baud = 400000;
      break;

    case 7:
      baud = 250000;
      break;

    case 9:
      baud = 200000;
      break;

    case 16:
      baud = 115200;
      break;

    case 34:
      baud = 57600;
      break;

    case 103:
      baud = 19200;
      break;

    case 207:
      baud = 9600;
      break;
    
    default:
      baud = 9600;
      break;
  }

  // Re-initialize USART chip for (potentially) new baud rate
  USART_Init(FOSC/16/baud-1);

  return returnBaud;
}
unsigned int returnDelayTime(unsigned int rdt, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    rdt = rdt & 255;
    EEPROM_Write(5, rdt);
  }

  // Read the specified EEPROM address
  return (unsigned int) EEPROM_Read(5);
}
unsigned int cwAngleLimit(unsigned int cwal, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // This parameter is 2 bytes, need two Write operations
    EEPROM_Write(6, cwal & 255); // lower byte first, and with 0xFF to make sure only the lower byte is populated
    EEPROM_Write(7, cwal>>8); // then upper byte
  }

  // read both bytes, combine and return
  unsigned char firstByte = EEPROM_Read(6);
  unsigned char secondByte = EEPROM_Read(7);

  return (unsigned int) (8<<secondByte) | (firstByte);
  
}
unsigned int ccwAngleLimit(unsigned int ccwal, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // This parameter is 2 bytes, need two Write operations
    EEPROM_Write(8, ccwal & 255); // lower byte first, and with 0xFF to make sure only the lower byte is populated
    EEPROM_Write(9, ccwal>>8); // then upper byte
  }

  // read both bytes, combine and return
  unsigned char firstByte = EEPROM_Read(8);
  unsigned char secondByte = EEPROM_Read(9);

  return (unsigned int) (8<<secondByte) | (firstByte);
}
unsigned int temperatureLimit(unsigned int temp, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    temp = temp & 255;
    EEPROM_Write(5, temp);
  }

  // Read the specified EEPROM address
  return (unsigned int) EEPROM_Read(11);
}
unsigned int minVoltageLimit(unsigned int minvolt, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    minvolt = minvolt & 255;
    EEPROM_Write(5, minvolt);
  }

  // Read the specified EEPROM address
  return (unsigned int) EEPROM_Read(12);
}
unsigned int maxVoltageLimit(unsigned int maxvolt, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    maxvolt = maxvolt & 255;
    EEPROM_Write(5, maxvolt);
  }

  // Read the specified EEPROM address
  return (unsigned int) EEPROM_Read(13);
}
unsigned int maxTorque(unsigned int maxt, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // This parameter is 2 bytes, need two Write operations
    EEPROM_Write(14, maxt & 255); // lower byte first, and with 0xFF to make sure only the lower byte is populated
    EEPROM_Write(15, maxt>>8); // then upper byte
  }

  // read both bytes, combine and return
  unsigned char firstByte = EEPROM_Read(14);
  unsigned char secondByte = EEPROM_Read(15);

  return (unsigned int) (8<<secondByte) | (firstByte);
}
unsigned int statusReturnLevel(unsigned int srl, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    srl = srl & 255;
    EEPROM_Write(5, srl);
  }

  // Read the specified EEPROM address
  return (unsigned int) EEPROM_Read(16);
}
unsigned int alarmLED(unsigned int alarm, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    alarm = alarm & 255;
    EEPROM_Write(5, alarm);
  }

  // Read the specified EEPROM address
  return (unsigned int) EEPROM_Read(17);
}
unsigned int shutdown(unsigned int shutdown, bool write)
{
  // Check write flag, write to the EEPROM if true
  if (write)
  {
    // Clean unsigned int to guarantee fit into char
    shutdown = shutdown & 255;
    EEPROM_Write(5, shutdown);
  }

  // Read the specified EEPROM address
  return (unsigned int) EEPROM_Read(18);
}


// unsigned int torqueEnable(unsigned int enable, bool write)
// {
  
// }
// unsigned int LED(unsigned int led, bool write)
// {
  
// }
// unsigned int cwComplianceMargin(unsigned int cwcompmargin, bool write)
// {
  
// }
// unsigned int ccwComplianceMargin(unsigned int ccwcompmargin, bool write)
// {
  
// }
// unsigned int cwComplianceSlope(unsigned int cwcompslope, bool write)
// {
  
// }
// unsigned int ccwComplianceSlope(unsigned int ccwcompslope, bool write)
// {
  
// }
// unsigned int goalPosition(unsigned int goal, bool write)
// {
  
// }
// unsigned int movingSpeed(unsigned int speed, bool write)
// {
  
// }
// unsigned int torqueLimit(unsigned int torqueLimit, bool write)
// {
  
// }
// unsigned int presentPosition(unsigned int pos, bool write)
// {
  
// }
// unsigned int presentSpeed(unsigned int speed, bool write)
// {
  
// }
// unsigned int presentLoad(unsigned int load, bool write)
// {
  
// }
// unsigned int presentVoltage(unsigned int voltage, bool write)
// {
  
// }
// unsigned int presentTemperature(unsigned int temp, bool write)
// {
  
// }
// unsigned int registered(unsigned int registered, bool write)
// {
  
// }
// unsigned int moving(unsigned int moving, bool write)
// {
  
// }
// unsigned int lock(unsigned int lock, bool write)
// {
  
// }
// unsigned int punch(unsigned int punch, bool write)
// {
  
// }