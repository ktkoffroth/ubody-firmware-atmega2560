
 int main(void)
   {
     //Initialise arduino ( so millis, delay work ).
     init();
#if defined(USBCON)
	USB.attach();
#endif

    // Setup code

     //Start loop
     while(true)
     {

       if (serialEventRun) serialEventRun();
     }
     return 0;
   }