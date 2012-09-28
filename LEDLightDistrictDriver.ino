// This demo does web requests to a fixed IP address, using a fixed gateway.
// 2010-11-27 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

int SDI = 2; //Yellow wire (not the red 5V wire!)
int CKI = 4; //Green wire

#define SCREEN_WIDTH 12
#define SCREEN_HEIGHT 10
#define STRIP_LENGTH SCREEN_WIDTH * SCREEN_HEIGHT

#define REQUEST_RATE 5000 // milliseconds


byte colorSequence[ STRIP_LENGTH * 3 ] ; // 


int receivedMessageCount = 0 ; // every 10 received messages the drivers sendsan acknowledgement

// int nextEmptyBufferTime ; 

void setup () {
  
  // set up serial port
  Serial.begin(115200);
  
  while( Serial.available() ) Serial.read() ;  // clear any information that happens to be in the buffer
  end_frame() ; // end the frame so that 
   
   // configure spi pins
  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  digitalWrite(CKI, LOW);
  
  // put a black screen in the color sequence
      for( int i = 0 ; i < STRIP_LENGTH ; i++ )
      {
       colorSequence[  3 * i ] = 0 ; 
       colorSequence[  3 * i + 1 ] = 0  ;
       colorSequence[  3 * i + 2 ] = 0  ;
      }
     
    // send the black color sequence to screen, making the screen black
   for( int i = 0 ; i < STRIP_LENGTH ; i++ )
    send_colour( colorSequence[  3 * i ], colorSequence[  3 * i + 1 ], colorSequence[  3 * i + 2 ] ) ; 

  end_frame() ;    // end the frame by waiting briefly. This ensures the next frame can be sent correctly
  // nextEmptyBufferTime = millis() + 2000 ;

}


void loop () 
  {  
   readSerial() ;
  }


void readSerial()
  {
   // whenever one or more put pixel messages are on serial, read them and update the screen
    
   if( Serial.available() ) // only read and update the screen if there is data on the serial port
    {
     int c = Serial.available() ;
    } 
  
  if( Serial.available() >= 4 ) // only if more than 4 bytes (irgb) are available, can you write a pixel
    {
    while( Serial.available() >= 4 ) // and while complete put pixel messages are available, read pixels from the serial.
      {
       readPutpixelFromSerial() ;
      }

    updateScreen() ; 
   } 
  }


int readPutpixelFromSerial()
  {
   byte i = Serial.read() ; // i is the index in the led string
   byte r = Serial.read() ; // r is the red, g is green b = blue
   byte g = Serial.read() ;
   byte b = Serial.read() ;
   
   // nextEmptyBufferTime = millis() + 2000 ;
       
   colorSequence[  3 * i ]     = r ; // every 3 bytes of the color sequence describe a pixel in the led string. 
   colorSequence[  3 * i + 1 ] = g ; // The first is red, the second green third blue
   colorSequence[  3 * i + 2 ] = b ; // so i * 3 identifies the offset of the pixel, 
                                     // which is incidentally also red, green is at i + 1 and blue is at i + 2
         
   receivedMessageCount++ ;          // every 10 messages processesed the arduino sends an 'a' so the pixel sender 
                                     // knows how fast the arduino is processing the pixels (and can stop sending 
                                     // and not overflow the 64B buffer on the arduino)
   
   if( receivedMessageCount == 10 )
     {
      receivedMessageCount = 0 ;
     
      Serial.write( 'a' ) ; 
     }
   
   return (int) i ; // the index of the pixel read is returned, so updateScreen() knows whether it needs to update again, or whether it can stop (see updateScreen() )
  }

/*
Here the led string is updated with new colors. But due to the small buffer on the serial port we can't wait too long. Writing a frame takes longer than it takes the typical
animation to fill the buffer with put pixel messages. Thus we have to read new put pixel messages during the led string update. But if the put pixel 
command writes to a section of the color sequence that has already be been sent to the string, the screen must be updated all over again.

If the screen has to be rewritten rewriteScreen must be set to true. If the index of the pixel read by readPutpixelFromSerial() is lower than the 
putPixelLoopIndex, that keeps track of how many pixels have already been sent int the led string, rewriteScreen must be set to true.

Because we to empty the buffer as often as possible the serialport buffer is read every time a bit is sent to the string. So the 
serialport is actually read in void send_colour(long this_led_color).

*/

boolean rewriteScreen = true ; 
int putPixelLoopIndex ;
void updateScreen()
  {
   rewriteScreen = true ;
   
   while( rewriteScreen )
      {
       
       rewriteScreen = false ;
       for( putPixelLoopIndex = 0 ; putPixelLoopIndex < STRIP_LENGTH ; putPixelLoopIndex++ )
        {
         send_colour( colorSequence[  3 * putPixelLoopIndex ], colorSequence[  3 * putPixelLoopIndex + 1 ], colorSequence[  3 * putPixelLoopIndex + 2 ] ) ;         
        }
       
       // start of benchmark code: just put more pixels on the chain, to see how fast it works
       // for( int i = 0 ;  i < 256 ; i++ )
       //         send_colour( 0, 0, 10 ) ;         
       // end of benchmark code 
        
       end_frame() ; 
      }

  }


void singleblock(int n){
  for(int i=0;i<(n-1);i++) send_colour(0x000000);
  send_colour(0x00FF00);
  for(int i=0;i<(STRIP_LENGTH-n);i++) send_colour(0x000000);
  end_frame();
}

// first convert the colors into bits, then send the bits to the screen

void send_colour(int red, int green, int blue){
   long clr=green * 0x010000L + red * 0x000100L+ blue*0x000001L; // the colors aren't in ordered rgb, but grb
   send_colour(clr);
}

void send_colour(long this_led_color){
    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) { // a color is 24 bit, indexed from 0 to 23. because color_bit is an unsigned byte 0 - 1 == 255 due to integer underflow
      
      if( Serial.available() >= 4 && readPutpixelFromSerial() <= putPixelLoopIndex )  
            rewriteScreen = true ;
       
      // to get the right bit the color we use a mask
      // if shift 1 color_bit positions to the right, leaving 0s at the other positions
      // and then we use a binary and ( the & operator ).
      // C = A & B means that for each binary digit (bit) in A and B, if both A and B have 
      // a digit on 1 at that position, C also has a 1 on that position. Other wise C has  
      // that digit set to 0.
      
      // the if condition below thus non-zero only if it has a bit set to true at the same
      // position as the color_bit variable. And thus the SDI bit is set to high only if
      // the bit in this_led_color corresponding to the 1 in color_bit is true.
      
      
      //Feed color bit 23 first (red data MSB)
      digitalWrite(CKI, LOW); //Only change data when clock is low
      long mask = 1L << color_bit;
      //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.
      if(this_led_color & mask) 
        digitalWrite(SDI, HIGH);
      else
        digitalWrite(SDI, LOW);
      digitalWrite(CKI, HIGH); //Data is latched when clock goes high
    }
}

void end_frame(){
  //Pull clock low to put strip into reset/post mode
  digitalWrite(CKI, LOW);
  delay(1); //Wait for 500us to go into reset
}

