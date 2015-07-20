//The sleeping with narcoleptic and XBEE do not work very well. Narcoleptic appeared to do nothing.

#include <Narcoleptic.h>

#include <Phant.h>
#include <SoftwareSerial.h>
#include <CapacitiveSensor.h>

/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10M between send pin and receive pin
 * Resistor effects sensitivity, experiment with values, 50K - 50M. Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 */


CapacitiveSensor   cs_4_5 = CapacitiveSensor(4,5);        // 10M resistor between pins 4 & 5, pin 5 is sensor pin, add a wire and or foil if desired

#define XBEE_SLEEP_PIN 12
#define COMMAND_TIMEOUT 10000 // ms
////////////////////////
// WiFi Network Stuff //
////////////////////////
// Your WiFi network's SSID (name):
String WIFI_SSID = "CMBozeman Guest";
// Your WiFi network's encryption setting
// Set the "encrypt" variable to one of these four characters:
// OPEN = 0, WPA_TKIP = 1, WPA2_AES = 2, WEP = 3
enum encryption{NO_SECURITY, WPA_TKIP, WPA2_AES, WEP};
encryption WIFI_EE = WPA2_AES;
// Your WiFi network's passphrase (if necessary). If your network
// is open, make this a blank string (passphrase = "";)
String WIFI_PSK = "CMBGuest0987";

/////////////////
// Phant Stuff //
/////////////////
// destIP will go into the XBee's configuration setting, so
// it'll automatically connect to the correct server.
String destIP = "54.86.132.254"; // data.sparkfun.com's IP address
// Initialize the phant object:
// Phant phant(server, publicKey, privateKey);
Phant phant("data.sparkfun.com", "aGX3bzQLmQiRObamEjy3", "KEmwqzKp2KUr7eVmGkAz");
// Phant field string defintions. Make sure these match the
// fields you've defined in your data stream:

const String homebrew = "homebrew";
const String industrial = "industrial";
const String capacitive = "capacitive";

////////////////
// XBee Stuff //
////////////////
const byte XB_RX = 2; // XBee's RX (Din) pin
const byte XB_TX = 3; // XBee's TX (Dout) pin
// We'll use "xB" from here-on to send and receive data to it:
SoftwareSerial xB(XB_RX, XB_TX); 
const int XBEE_BAUD = 9600; // Your XBee's baud (9600 is default)

/////////////////////////////
// Sensors/Input Pin Stuff //
/////////////////////////////
const int homePin = A0; // Photocell input
const int industPin = A1;  // TMP36 indust sensor input

// opVoltage - Useful for converting ADC reading to voltage:
const float opVoltage = 4.7; 
float industVal;
int homeVal, coVal;
long capacVal;

int testVal = 1;

/////////////////////////
// Update Rate Control //
/////////////////////////
// Phant limits you to 10 seconds between posts. Use this variable
// to limit the update rate (in milliseconds):
const unsigned long UPDATE_RATE = 10000; // 21,600,000ms = 6 Hours

unsigned long lastUpdate = 0; // Keep track of last update time

///////////
// Setup //
///////////
// In setup() we configure our INPUT PINS, start the XBee and
// SERIAL ports, and CONNECT TO THE WIFI NETWORK.
void setup()
{
  
  
  // Set up sensor pins:
  pinMode(homePin, INPUT);
  pinMode(industPin, INPUT);
  pinMode(XBEE_SLEEP_PIN, OUTPUT);
  

  cs_4_5.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example

  // Set up serial ports:
  Serial.begin(9600);
  Serial.println("test");
  // Make sure the XBEE BAUD RATE matches its pre-set value
  // (defaults to 9600).
  xB.begin(XBEE_BAUD);
  
  
  // Set up WiFi networkm
  Serial.println("Testing network");
  digitalWrite(XBEE_SLEEP_PIN, LOW);

  //Serial.println("plz send help");
  // connectWiFi will atindustt(attempt) to connect to the given SSID, using
  // encryption mode "encrypt", and the passphrase string given.
  connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);
  // Once connected, print out our IP address for a sanity check:
  Serial.println("Connected!");
  Serial.print("IP Address: "); printIP(); Serial.println(); 

  // setupHTTP() will set up the destination address, port, and
  // make sure we're in TCP mode:  connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);
  //connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);
  //connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);

  setupHTTP(destIP);
  delay(2000);
  
  digitalWrite(XBEE_SLEEP_PIN, LOW);
  setupPins();
  //digitalWrite(XBEE_SLEEP_PIN, HIGH);
  // Once everything's set up, send a data stream to make sure
  // everything check's out:
 // Serial.print("Sending update...");
 // if (sendData())
 //   Serial.println("SUCCESS!");
 // else
 //   Serial.println("Failed :(");
}

//////////
// Loop //
//////////
// loop() constantly checks to see if enough time has lapsed
// (controlled by UPDATE_RATE) to allow a new stream of data
// to be posted.
// Otherwise, to kill time, it'll print out the sensor values
// over the serial port.
/*void loop()
{
  Serial.println("sleep xbee");
  digitalWrite(XBEE_SLEEP_PIN, HIGH);
  delay(5000);
  Serial.println("wake up xbee");
  digitalWrite(XBEE_SLEEP_PIN, LOW);
  delay(5000);  
}*/
void loop()
{
  //Turn the XBEE ON (Wakes it up)
  delay(5000);
  digitalWrite(XBEE_SLEEP_PIN, LOW);
  //delay(5000);
  connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);
  //delay(5000);
  Serial.println("Connected!");
  Serial.print("IP Address: "); printIP(); Serial.println(); 
  setupHTTP(destIP);
   
  readSensors();
  int sendVal = 0;
  sendVal = sendData();
  while(sendVal != 1){
    sendVal = sendData();
    Serial.print("Sending update...");
    if (sendVal== 1)
      Serial.println("SUCCESS!");
    else if(sendVal == -1)
      Serial.println("Timeout");
    else
      Serial.println("Failed :(");
  }
  //separate
  Serial.print("");
  Serial.print(capacVal);                  // print sensor output 1
  Serial.print("\t ");
  Serial.print(homeVal);
  Serial.print('\t');
  Serial.print(testVal);
  //Serial.print(industVal);
  Serial.print('\n');
  //delay(5000);
  
  Serial.flush(); // Flush data so we get fresh stuff in
  //Puts the XBEE in sleep mode
  digitalWrite(XBEE_SLEEP_PIN, HIGH);
  //puts the Aruino to sleepy
  Serial.print("narco at some point maybe");
  delay(10000); //add "Narcoleptic." before this for narco
  
  
  
}
/*void loop()
{
    while(sendData() == -1) {
        delay(1000);
    }
     testVal++;
    digitalWrite(XBEE_SLEEP_PIN, HIGH);
    Narcoleptic.delay(10000); //add "Narcoleptic." before this for narco
    //delay(10000);
    digitalWrite(XBEE_SLEEP_PIN, LOW);
  
  
}*/

////////////////
// sendData() //
////////////////
// sendData() makes use of the PHANT LIBRARY to send data to the
// data.sparkfun.com server. We'll use phant.add() to add specific
// parameter and their values to the param list. Then use
// phant.post() to send that data up to the server.
int sendData()
{

  // IMPORTANT PHANT STUFF!!!
  // First we need to add fields and values to send as parameters
  // Since we just need to read values from the analog pins, this
  // can be automized with a for loop:
  //readSensors(); // Get updated values from sensors.
  //industVal = analogRead(industPin);
  //homeVal = analogRead(homePin);
  //capacVal =  (cs_4_5.capacitiveSensorRaw(50)/50);
  
  //Serial.println("Sending data: ");
  //Serial.print("industrial: ");
  //Serial.println(testVal);

  phant.add(industrial, testVal);
  testVal++;
  //phant.add(industrial, industVal);
  phant.add(homebrew, homeVal);
  phant.add(capacitive, capacVal);
  // After our PHANT.ADD's we need to PHANT.POST(). The post needs
  // to be sent out the XBee. A simple "print" of that post will
  // take care of it.
  //Serial.println(phant.post());
  
  String request = phant.post();
  flushXB();
  //Serial.println(request);
  xB.print(request);
  flushXB();
  //xB.flush(); // Flush data so we get fresh stuff in
  
  // Check the response to make sure we receive a "200 OK".
  char response[12];
  if (waitForAvailable(12) > 0)
  {
    for (int i=0; i<12; i++)
    {
      response[i] = xB.read();
    }
    if (memcmp(response, "HTTP/1.1 200", 12) == 0) {
      Serial.println("SUCCESS");
      return 1;
    }
    else
    {
      Serial.println("ERROR");
      Serial.println(response);
      return 0; // Non-200 response
    }
  }
  else {// Otherwise timeout, no response from server
    Serial.println("TIMEOUT");
    return -1;
  }
}

// readSensors() will simply update a handful of global variables
// It updates industVal, homeVal, coVal, and capacVal
void readSensors()
{
  industVal = analogRead(industPin);
  homeVal = analogRead(homePin);
  capacVal =  cs_4_5.capacitiveSensor(30);
}

///////////////////////////
// XBee WiFi Setup Stuff //
///////////////////////////
// setupHTTP() sets three important parameters on the XBee:
// 1. Destination IP -- This is the IP address of the server
//    we want to send data to.
// 2. Destination Port -- We'll be sending data over port 80.
//    The standard HTTP port a server listens to.
// 3. IP protocol -- We'll be using TCP (instead of default UDP).
void setupHTTP(String address)
{
  // Enter command mode, wait till we get there.
  while (!commandMode(1))
    ;

  // Set IP (1 - TCP)
  command("ATIP1", 2); // RESP: OK
  // Set DL (destination IP address)
  command("ATDL" + address, 2); // RESP: OK
  // Set DE (0x50 - port 80)
  command("ATDE50", 2); // RESP: OK

  commandMode(0); // Exit command mode when done
}

void setupPins()
{
  // Enter command mode, wait till we get there.
  commandMode(1);
  
  command("ATD70", 2);
  command("ATSM1", 2);

  commandMode(0); // Exit command mode when done
}

///////////////
// printIP() //
///////////////
// Simple function that enters command mode, reads the IP and
// prints it to a serial terminal. Then exits command mode.
void printIP()
{
  // Wait till we get into command Mode.
  while (!commandMode(1))
    ;
  // Get rid of any data that may have already been in the
  // serial receive buffer:
  flushXB();
  // Send the ATMY command. Should at least respond with
  // "0.0.0.0\r" (7 characters):
  command("ATMY", 7);
  // While there are characters to be read, read them and throw
  // them out to the serial monitor.
  while (xB.available() > 0)
  {
    Serial.write(xB.read());
  }
  // Exit command mode:
  commandMode(0);
}

//////////////////////////////
// connectWiFi(id, ee, psk) //
//////////////////////////////
// For all of your connecting-to-WiFi-networks needs, we present
// the connectWiFi() function. Supply it an SSID, encryption
// setting, and passphrase, and it'll try its darndest to connect
// to connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK); your network.
int connectWiFi(String id, byte auth, String psk)
{
  const String CMD_SSID = "ATID";
  const String CMD_ENC = "ATEE";
  const String CMD_PSK = "ATPK";
  // Check if we're connected. If so, sweet! We're done.
  // Otherwise, time to configure some settings, and print
  // some status messages:
  int status;
  //Serial.println("connectWiFi");
  while ((status = checkConnect(id)) != 0)
  {
    // Print a status message. If `status` isn't 0 (indicating
    // "connected"), then it'll be one of these 
    //  (from XBee WiFI user's manual):
    // 0x01 - WiFi transceiver initialization in progress. 
    // 0x02 - WiFi transceiver initialized, but not yet scanning 
    //        for access point. 
    // 0x13 - Disconnecting from access point. 
    // 0x23 – SSID not configured. 
    // 0x24 - Encryption key invalid (either NULL or invalid 
    //        length for WEP) 
    // 0x27 – SSID was found, but join failed. 0x40- Waiting for 
    //        WPA or WPA2 Authentication 
    // 0x41 – Module joined a network and is waiting for IP 
    //        configuration to complete, which usually means it is
    //        waiting for a DHCP provided address. 
    // 0x42 – Module is joined, IP is configured, and listening 
    //        sockets are being set up. 
    // 0xFF– Module is currently scanning for the configured SSID.
    //
    // We added 0xFE to indicate connected but SSID doesn't match
    // the provided id.
    Serial.print("Waiting to connect: ");
    Serial.println(status, HEX);

    commandMode(0);
    while(!commandMode(1)){} // Enter command mode

    // Write AH (2 - Infrastructure) -- Locked in
    command("ATAH2", 2);
    // Write CE (2 - STA) -- Locked in
    command("ATCE2", 2);  
    // Write ID (SparkFun) -- Defined as parameter
    command(CMD_SSID + id, 2);
    // Write EE (Encryption Enable) -- Defined as parameter
    command(CMD_ENC + auth, 2);
    // Write PK ("sparkfun6175") -- Defined as parameter
    command(CMD_PSK + psk, 2);
    // Write MA (0 - DHCP) -- Locked in
    command("ATMA0", 2);
    // Write IP (1 - TCP) -- Loced in
    command("ATIP1", 2);

    commandMode(0); // Exit Command Mode CN

    delay(2000);
  }
}

// Check if the XBee is connected to a WiFi network.
// This function will send the ATAI command to the XBee.
// That command will return with either a 0 (meaning connected)
// or various values indicating different levels of no-connect.
byte checkConnect(String id)
{  
  //Serial.println("checkConnect");
  char indust[2];
  commandMode(0);
  while (!commandMode(1))
    ;
  //Serial.println("in command mode");
  command("ATAI", 2);
  char c0 = xB.read();
  char c1 = xB.read();
  indust[0] = hexToInt(c0);
  indust[1] = hexToInt(c1);
  flushXB();

  if (indust[0] == 0)
  {
    command("ATID", 1);
    int i=0;
    char c=0;
    String atid;
    while ((c != 0x0D) && xB.available())
    {
      c = xB.read();
      if (c != 0x0D)
        atid += c;
    }
    if (atid == id)
      return 0;
    else
      return 0xFE;
  }
  else
  {
    if (indust[1] == 0x13)
      return indust[0];
    else
      return (indust[0]<<4) | indust[1];
  }
}

/////////////////////////////////////
// Low-level, ugly, XBee Functions //
/////////////////////////////////////
void command(String atcmd, int rsplen)
{
  flushXB();
  xB.print(atcmd);
  xB.print("\r");
  waitForAvailable(rsplen);
}

void flushXB() {
//  if(xB.available()) {
//      Serial.print("Flushing: ");
//      while(xB.available() > 0) {
//          Serial.print(xB.read());
//     }  
//      Serial.println();
//  }
  
  xB.flush();
}

int commandMode(boolean enter)
{
  flushXB();
  //Serial.println("flushed");
  if (enter)
  {
    char c;
    xB.print("+++");   // Send CMD mode string
    //Serial.println("+++");
    waitForAvailable(3);
    //Serial.println("waited");
    Serial.println(xB.available());
    if (xB.available() > 0)
    {
      c = xB.read();
      Serial.print(c);
      if (c == 'O') // That's the letter 'O', assume 'K' is next
        c = xB.read();
        Serial.print(c);
        c = xB.read();
        Serial.println(c);
        return 1; // IF we see "OK" return success
    }
    return 0; // If no (or incorrect) receive, return fail
  }
  else
  {
    command("ATCN", 2);
    return 1;
  }
}

int waitForAvailable(int qty)
{
  int timeout = COMMAND_TIMEOUT;

  while ((timeout-- > 0) && (xB.available() < qty))
    delay(1);
  return timeout;
}

byte hexToInt(char c)
{
  if (c >= 0x41) // If it's A-F
    return c - 0x37;
  else
    return c - 0x30;
}


