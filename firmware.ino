#define FOSC 16000000 // System Clock speed
#define BAUD 9600 // Desired baud rate
#define MYUBRR (FOSC/16/BAUD-1) // Actual baud rate

void USART_Init(unsigned int ubrr); // USART serial setup (half-duplex async mode)
void USART_Transmit(unsigned char data); // Transmit a char in serial
unsigned char USART_Recieve(void); // Recieve a char from serial

int main(void)
   {
     //Initialise arduino ( so millis, delay work ).
     init();
#if defined(USBCON)
	USB.attach();
#endif

    // USART serial setup (Half-duplex async mode)
    USART_Init(MYUBRR);
    unsigned char echo;

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