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