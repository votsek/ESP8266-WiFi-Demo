
//
//  miniservsmartsf
//
//  My short demo program to serve a line or two to the Raspberry Pi after receiving and parsing
//  a GET from the client. This uses the ESP8266 and open source drivers from Sparkfun
//  to connect via WiFi.
//

//  HW serial is used for debug output to the Serial Monitor
#include <SoftwareSerial.h>

//  SparkFun driver and client/server methods
#include <SparkFunESP8266WiFi.h>

//  Define DEBUG to enable outpur t the Serial Monitor. If running headless, DEBUG
//  should not be defined.
#define DEBUG

//
//  Network constants
//

const char mySSID[] = "EDNET";      //  WiFi network name
const char myPSK[] = "project2019"; //  Network password

//    HTML header for reply to the client

const String htmlBegin =   "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "Connection: close\r\n\r\n"
                            "<DOCTYPE HTML>\r\n"
                            "<html>\r\n";

//    HTML body

String htmlBody;

//    HTML terminator

const String htmlTerm =     "</html>\n";


//
//  Variables
//
//    Declared where used to minimize use of dynamic memory

//
//  WiFi server on port 80

ESP8266Server server = ESP8266Server (80);

void setup()
{
  //
  //  Set up the network connection for the server
  //
  //  Hardware serial is used to provide the serial monitor for debug
  //
  #ifdef DEBUG
  Serial.begin (9600);
  delay(100);
  #endif

  //  Wait for user to take action
  serialTrigger ("Press any key to begin");

  //  Initialize the WiFi shield
  initializeESP8266 ();

  //  Connect to the shield in station mode
  connectstaESP8266 ();

  #ifdef DEBUG
  //  Print local connection data
  displayConnectInfo ();
  #endif

  //  Start the server
  server.begin ();
  #ifdef DEBUG
  Serial.println("Server started.");
  #endif

  

}

void loop()
{ //
  //  Instantiate the client object with a 500 ms connection timeout
  ESP8266Client client = server.available(500);
  
  //
  //  Funky test using the client object. It works since the server
  //  available method sets it to 0 on failure.
  if(client)
  {
    #ifdef DEBUG
    Serial.println("Client connected");
    #endif
    
    // flush any pending input
    client.flush();

    // check that client is still connected
    while (client.connected())
    {
      //
      // Read data and skip header, retry 2 times
      int retLen=0;
      char myBuff[200];
      int myBuffLen=200;  //  Simple GET with one parameter that I tried
                          //  required 117 bytes, including the /r/n characters.
      retLen = readData(client, myBuff, myBuffLen, 2);
      
      //
      // if any data, display and send a reply
      if (retLen > 0)
      {
        myBuff[retLen]=0x0;            // terminate
        #ifdef DEBUG
        Serial.print(F("FROM: "));     // to screen
        Serial.print(myBuff);          // to screen
        #endif
      

        //Build reply string

        client.print(htmlBegin);
        String htmlBody;
        //  Send a friendly message to the client running on the Pi
        htmlBody += "Hello, Pi";
        htmlBody += "<br>\n";
        htmlBody += htmlTerm;
        client.print(htmlBody);
    
        //  Give the client time to receive the data
        delay(1);
   
        //  Close the connection:
        client.stop();
        #ifdef DEBUG
        Serial.println(F("Client disconnected"));
        #endif
      }
      
    }

  }
  
}


void initializeESP8266()
{ //
  // esp8266.begin() verifies that the ESP8266 is operational
  // and sets it up for the rest of the sketch.
  // It returns either true or false -- indicating whether
  // communication was successul or not.
  //
  
  if (esp8266.begin() != true)
  {
    //  Error communicating with the shield
    #ifdef DEBUG
    Serial.println(F("Error communicating with the WiFi shield"));
    #endif
    errorExit();
  }
  #ifdef DEBUG
  Serial.println(F("The WiFi shield is present"));
  #endif

}


void connectstaESP8266()
{
  //
  //  The ESP8266 can be set to one of three modes, but
  //  this server will be set to Station mode only
  //
  
  //  Check for shield already in station mode

  if (esp8266.getMode() != ESP8266_MODE_STA)
  { //
    // It is not in station mode, so put it there
    
    if (esp8266.setMode(ESP8266_MODE_STA) < 0)
    {
      #ifdef DEBUG
      Serial.println(F("Error setting mode."));
      #endif
      errorExit();
    }
  }

  //  Mode successfully set to station
  #ifdef DEBUG
  Serial.println("Mode set to station");
  #endif

  //  Connect if not already connected
  if (esp8266.status() <= 0)
  
  {
    #ifdef DEBUG
    Serial.print("Connecting to ");
    Serial.println(mySSID);
    #endif

    //  Connect

    //  If error, print message and exit
    if (esp8266.connect(mySSID, myPSK) < 0)
    {
      #ifdef DEBUG
      Serial.println(F("Error connecting"));
      #endif
      errorExit();
    }
  }
}


void displayConnectInfo()
{ //
  //  Diisplay the SSID, local IP and MAC of the ESP8266 shield
  //  once connected to the network
  //
  
  char connectedSSID[24];
  memset(connectedSSID, 0, 24);
  char deviceMAC[30];
  memset(deviceMAC, 0, 30);

  //
  //  Check for connection to the network access point
  int retVal = esp8266.getAP(connectedSSID);
  if (retVal > 0)
  {
    Serial.print("Connected to: ");
    Serial.println(connectedSSID);
  }

  //
  //  Return the IP address of the shield. Note: the AT command response includes the
  //  MAC. The library method needs update to return the MAC as well as the IP.
  IPAddress myIP = esp8266.localIP();
  Serial.print("My IP: "); Serial.println(myIP);

  //
  //  Use deprecated AT command to retrieve the device MAC
  retVal = esp8266.localMAC (deviceMAC);

  //  Check for error retrieving MAC
  if (retVal <= 0)
  {
    errorExit ();
  }

  Serial.print ("Device MAC: "); Serial.println (deviceMAC);

}

void serialTrigger(String message)
//
// serialTrigger prints a message, then waits for something
// to come in from the serial port.
//
{
  #ifdef DEBUG
  Serial.println();
  Serial.println(message);
  Serial.println();
  while (!Serial.available())
    ;
  while (Serial.available())
    Serial.read();
  #endif
}

void errorExit ()
//
//  Exit from a loop, print a message, and loop forever until disconnected
//  from power.
//
{
  #ifdef DEBUG
  Serial.println (F("Waiting for you to unplug me"));
  #endif
  //
  //  The only way to stop an Arduino sketch is to loop forever and wait
  //  for the user to do something drastic to stop it. This function could
  //  light an LED before looping to indicate an error.
  for (;;);
}


int readData(ESP8266Client client, char *out, int len, int retry)
//
//  charsread = readData ( client, buff, bufflen, retry)
//
//    charsread ->  int number of characters read into buff array
//    client    ->  client object
//    buff      ->  char buffer to receive characters read from the ESP8266
//    bufflen   ->  int length of the buffer
//    retry     ->  int number of times to retry the read if client is not available
//
//  This function reads the client request from the ESP8266. It can be used to
//  read the passed parameters to be evaluated by the server.
//
{
   int wait = 0;
   char c;
   int i = 0, ch_count = 0;
   char ch_len[5];
   int retry_loop = retry;

  // Try at least loop times before returning
  while (retry_loop > 0)
  {
     retry_loop--;
     delay(100);                      // wait 100ms to handle any delay between characters

    // available() will return the number of characters
    // currently in the receive buffer.
    while (client.available() )
    {
      // get character from buffer
      c = client.read();

      // parse +IPD ....  header. format is : +IPD,0,4:paul
      if (wait < 3)
      {
        if (wait == 2) ch_len[i++] = c;  // after the second comma : store character count digit(s)

        if (c == ',') wait++;            // count comma's

        else if (c == ':')               // end of header
        {
          ch_len[i] = 0x0;               // terminate buffer
          ch_count= atoi(ch_len);        // calculate length of message
          wait = 3;                      // indicate header has been parsed
          i = 0;                         // reset storage counter
        }

        continue;                        // skip rest of loop
      }

      // if no buffer length, just display character on serial/screen
      if (len == 0) Serial.write(c);
      else
      {
        //output to provided buffer
        out[i++] = c;

        // prevent buffer overrun
        if (i == len-1)
        {
          out[i] = 0x0;  // terminate
          return i;
        }
      }

      // if we got atleast ONE character, but not all increase retry_loop to keep trying to get all
      // we should not expect/hope this loopcount will be reached. This is only to prevent a dead-lock
      if (--ch_count > 0) retry_loop = 10;

      // seems we got them all, no need for waiting and retry
      else return i;
    }
  }

  out[i] = 0x0;  // terminate

  // return count of characters in buffer
  return i;
}
