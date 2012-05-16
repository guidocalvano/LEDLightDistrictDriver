// This demo does web requests to a fixed IP address, using a fixed gateway.
// 2010-11-27 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

int SDI = 2; //Yellow wire (not the red 5V wire!)
int CKI = 4; //Green wire

#define SCREEN_WIDTH 8
#define SCREEN_HEIGHT 5
#define STRIP_LENGTH SCREEN_WIDTH * SCREEN_HEIGHT

#include <EtherCard.h>

#define REQUEST_RATE 5000 // milliseconds

byte colorSequence[ STRIP_LENGTH * 3 ] ;

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte myip[] = { 192,168,3,203 }; // nanode ip
// gateway ip address
static byte gwip[] = { 192,168,3,1 }; // gateway (router) ip
// remote website ip address and port
static byte hisip[] = { 192,168,3,1 }; // ip of the site you request
// remote website name
char website[] PROGMEM = "192.168.3.1"; // the hostname of the url of the site you request

// ~guidocalvano/
static BufferFiller bfill;  // used as cursor while filling the buffer
byte Ethernet::buffer[1000];   // a very small tcp/ip buffer is enough here
static long timer;

// called when the client request is complete
static void my_result_cb (byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.print(millis() - timer);
  Serial.println(" ms");
  Serial.println((const char*) Ethernet::buffer + off);
}

void setup () {
  Serial.begin(9600);
  Serial.println("\n[getStaticIP]");
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");

  ether.staticSetup(myip, gwip);

  ether.copyIp(ether.hisip, hisip);
  ether.printIp("Server: ", ether.hisip);

  while (ether.clientWaitingGw())
    ether.packetLoop(ether.packetReceive());
    
  Serial.println("Gateway found");
  
  timer = - REQUEST_RATE; // start timing out right away
  
  
   
  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  digitalWrite(CKI, LOW);
  
  
    
  
  
      for( int i = 0 ; i < STRIP_LENGTH ; i++ )
      {
       colorSequence[  3 * i ] = 0; 
       colorSequence[  3 * i + 1 ] = 4  ;
       colorSequence[  3 * i + 2 ] = 0  ;
      }
}
/*
char page[] PROGMEM = "HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<html>"
  "<head><title>"
    "No worries"
  "</title></head>"
  "<body>"
    "<h3>This service is currently fine</h3>"
    "<p><em>"
      "The main server is currently on-line.<br />"
    "</em></p>"
  "</body>"
"</html>"
;
*/

char page[] PROGMEM = "HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"\r\n";
char* nextLine( char* bufferOffset )
  {
   while( true ) // !( *bufferOffset == '\n' ) )
     {

      if( *bufferOffset == '\r' )
        {
         bufferOffset++ ;
         
         if( *bufferOffset == '\n' )
           {
            bufferOffset++ ;
             
            if( *bufferOffset == '\r' )
               {
                bufferOffset ++ ;
                
                if( *bufferOffset == '\n' )
                   {
                    bufferOffset++ ;
                    
                    while( *bufferOffset != '\n' ) bufferOffset++ ;
                    
                    bufferOffset++ ;
                 
                    return bufferOffset ;
                   }
               }
           }
        }
      else 
        bufferOffset++  ;
     }
     
     /*
   bufferOffset++ ;
   
   return bufferOffset ; */
  } ;


void loop () {
  
  
  word len = ether.packetReceive() ;
  word pos = ether.packetLoop(len) ;

if( pos )
  {
   // Serial.println( "start of packet\n" );
    bfill = ether.tcpOffset();
    char* dataOffset = (char*) Ethernet::buffer + pos ;
    

    
   // char* next = nextLine( nextLine( nextLine( nextLine( nextLine( nextLine( dataOffset ) ) ) ) ) ) ;
    char* next = nextLine( dataOffset ) ;
    
  //  Serial.println( "payload" ) ;
    //Serial.println( next ) ;
    
   Serial.println( next ) ;
    int offset =  next[ 0 ] ;
    int r =  next[ 1 ] ;
    int g =  next[ 2 ] ;
    int b =  next[ 3 ] ;
    
    int index = 3 * offset ;
    colorSequence[ index ] = r ;
    colorSequence[ index + 1 ] = g ;
    colorSequence[ index + 2 ] = b ;
    
    
    
/*
    Serial.print( "offset" ) ; Serial.println( (int) next[ 0 ] ) ;
    Serial.print( "r" ) ; Serial.println( (int) next[ 1 ] ) ;
    Serial.print( "g" ) ; Serial.println( (int) next[ 2 ] ) ;
    Serial.print( "b" ) ; Serial.println( (int) next[ 3 ] ) ;
    /*
    Serial.println( "end of packet\n" );
    */
    memcpy_P(ether.tcpOffset(), page, sizeof page);
    ether.httpServerReply(sizeof page - 1);
  }
  /*
  if (millis() > timer + REQUEST_RATE) {
    timer = millis();
    // Serial.println(PSTR("/~guidocalvano/"));
    ether.browseUrl(PSTR("/~guidocalvano/"), "", website, my_result_cb);
   
  }*/
  /*
  for(int i=0;i<STRIP_LENGTH;i++) singleblock(i);
  for(int i=STRIP_LENGTH;i>0;i--) singleblock(i);
  */
  
 
  for( int i = 0 ; i < STRIP_LENGTH ; i++ )
    send_colour( colorSequence[  3 * i ], colorSequence[  3 * i + 1 ], colorSequence[  3 * i + 2 ] ) ; 
  end_frame() ;
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


