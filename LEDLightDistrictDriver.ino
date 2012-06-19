// This demo does web requests to a fixed IP address, using a fixed gateway.
// 2010-11-27 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

int SDI = 2; //Yellow wire (not the red 5V wire!)
int CKI = 4; //Green wire

#define SCREEN_WIDTH 8
#define SCREEN_HEIGHT 5
#define STRIP_LENGTH SCREEN_WIDTH * SCREEN_HEIGHT

#define REQUEST_RATE 5000 // milliseconds

byte colorSequence[ STRIP_LENGTH * 3 ] ;


int receivedMessageCount = 0 ;


void setup () {
  Serial.begin(115200);
   

   
  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  digitalWrite(CKI, LOW);
  
      for( int i = 0 ; i < STRIP_LENGTH ; i++ )
      {
       colorSequence[  3 * i ] = 0 ; 
       colorSequence[  3 * i + 1 ] = 0  ;
       colorSequence[  3 * i + 2 ] = 0  ;
      }
      
      
   for( int i = 0 ; i < STRIP_LENGTH ; i++ )
    send_colour( colorSequence[  3 * i ], colorSequence[  3 * i + 1 ], colorSequence[  3 * i + 2 ] ) ; 
  end_frame() ;
}


void loop () 
  {
   readSerial() ;
  }


void readSerial()
  {
     if( Serial.available() )
    {

         colorSequence[ 0 ] = 0 ;
          colorSequence[ 1 ] = 255 ;
          colorSequence[ 2 ] = 0 ; 
     
     int c = Serial.available() ;


     if( c & 1 ) 
        colorSequence[ 3 ] = 50 ;     
        
     if( c & 2 ) 
        colorSequence[ 4 ] = 50 ;     

     if( c & 4 ) 
        colorSequence[ 5 ] = 50 ;
    } 
  
  if( Serial.available() >= 4 )
    {
    while( Serial.available() >= 4 )
      {
       readPutpixelFromSerial() ;
      }
    updateScreen() ;
   } 
  }


int readPutpixelFromSerial()
  {
   byte i = Serial.read() ;
   byte r = Serial.read() ;
   byte g = Serial.read() ;
   byte b = Serial.read() ;
       
   if( i > 39 || i < 0 )
     {
      colorSequence[ 0 ] = 0 ;
      colorSequence[ 1 ] = 0 ;
      colorSequence[ 2 ] = 255 ;          
      return -1 ; 
     }
   
   
   if( i != 0 )
     {
       
      colorSequence[  3 * i ] = r ; 
      colorSequence[  3 * i + 1 ] = g  ;
      colorSequence[  3 * i + 2 ] = b  ;
     }
     
   receivedMessageCount++ ;
   
   if( receivedMessageCount == 10 )
     {
      receivedMessageCount = 0 ;
     
      Serial.write( 'a' ) ; 
     }
   
   return (int) i ;
  }


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
       
       // start of benchmark code
       for( int i = 0 ;  i < 256 ; i++ )
                send_colour( 0, 10, 0 ) ;         
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

void send_colour(int red, int green, int blue){
   long clr=green * 0x010000L + red * 0x000100L+ blue*0x000001L;
   send_colour(clr);
}

void send_colour(long this_led_color){
    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
      
      if( Serial.available() >= 4 && readPutpixelFromSerial() <= putPixelLoopIndex )  
            rewriteScreen = true ;
           
      
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

