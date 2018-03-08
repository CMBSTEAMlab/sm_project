/*****************************************************************
Phant_XBee_WiFi.ino
Post data to SparkFun's data stream server system (phant) using
an XBee WiFi and XBee Shield.
Jim Lindblom @ SparkFun Electronics
Original Creation Date: May 20, 2014
https://learn.sparkfun.com/tutorials/online-datalogging-with-an-xbee-wifi

This sketch uses an XBee WiFi and an XBee Shield to get on
the internet and POST analogRead values to SparkFun's data
logging streams (http://data.sparkfun.com).

Hardware Hookup:
  The Arduino shield makes all of the connections you'll need
  between Arduino and XBee WiFi. If you have the shield make
  sure the SWITCH IS IN THE "DLINE" POSITION.

  I've also got four separate analog sensors (methane, co,
  temperature, and photocell) connected to pins A0-A3. Feel
  free to switch that up. You can post analog data, digital 
  data, strings, whatever you wish to the Phant server.

Requires the lovely Phant library:
  https://github.com/sparkfun/phant-arduino

Development environment specifics:
    IDE: Arduino 1.0.5
    Hardware Platform: SparkFun RedBoard
    XBee Shield & XBee WiFi (w/ trace antenna)

This code is beerware; if you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, please 
buy us a round!

Distributed as-is; no warranty is given.
*****************************************************************/
// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>
// The Phant library makes creating POSTs super-easy
#include <Phant.h>
#include <Narcoleptic.h>

// Time in ms, where we stop waiting for serial data to come in
// 2s is usually pretty good. Don't go under 1000ms (entering
// command mode takes about 1s).
#define COMMAND_TIMEOUT 5000 // ms

////////////////////////
// WiFi Network Stuff //
////////////////////////
// Your WiFi network's SSID (name):
//String WIFI_SSID = "CMBozeman Guest";
String WIFI_SSID = "FreakinSweet";
// Your WiFi network's encryption setting
// Set the "encrypt" variable to one of these four characters:
// OPEN = 0, WPA_TKIP = 1, WPA2_AES = 2, WEP = 3
enum encryption{NO_SECURITY, WPA_TKIP, WPA2_AES, WEP};
encryption WIFI_EE = WPA2_AES;
// Your WiFi network's passphrase (if necessary). If your network
// is open, make this a blank string (passphrase = "";)
//String WIFI_PSK = "CMBGuest0987";
String WIFI_PSK = "wtfd1nApf";

/////////////////
// Phant Stuff //
/////////////////
// destIP will go into the XBee's configuration setting, so
// it'll automatically connect to the correct server.
//String destIP = "54.86.132.254"; // data.sparkfun.com's IP address
String destIP = "192.168.1.122";
String destPort = "1F90";
// Initialize the phant object:
// Phant phant(server, publicKey, privateKey);
//Phant phant("data.sparkfun.com", "JxLNaEoE1XSOb1RZ4WDO", "gzJ50pDpW2T0w1yE2dg0");
Phant phant("192.168.1.122", "Z0wjAMb8YDSlNxKV6ZexuMgaMAz", "x7OjVwl9QgIvEyjMgqmyFgKegVO");
// Phant field string defintions. Make sure these match the
// fields you've defined in your data stream:
const String methaneField = "methane";
const String coField = "co";
const String tempField = "temp";
const String lightField = "light";

const String moistureField = "moisture";

////////////////
// XBee Stuff //
////////////////
const byte XB_SLEEP_PIN = 6; // XBee's DTR pin
const byte XB_RX = 8; // Arduino's RX, XBee's (Dout) pin
const byte XB_TX = 9; // Arduino's TX, XBee's (Din) pin
// We'll use "xB" from here-on to send and receive data to it:
SoftwareSerial xB(XB_RX, XB_TX); 
//#define xB Serial1
const int XBEE_BAUD = 9600; // Your XBee's baud (9600 is default)

/////////////////////////////
// Sensors/Input Pin Stuff //
/////////////////////////////
const int lightPin = A0; // Photocell input
const int tempPin = A1;  // TMP36 temp sensor input
const int coPin = A2;    // Carbon-monoxide sensor input
const int methanePin = A3; // Methane sensor input
// opVoltage - Useful for converting ADC reading to voltage:
const float opVoltage = 4.7;
float tempVal;
int lightVal, coVal, methaneVal;

int test = 0;

#define DEBUG 1

/////////////////////////
// Update Rate Control //
/////////////////////////
// Phant limits you to 10 seconds between posts. Use this variable
// to limit the update rate (in milliseconds):
const unsigned long UPDATE_RATE = 20000; // 300000ms = 5 minutes
unsigned long lastUpdate = 0; // Keep track of last update time

///////////
// Setup //
///////////
// In setup() we configure our INPUT PINS, start the XBee and
// SERIAL ports, and CONNECT TO THE WIFI NETWORK.
void setup()
{
  // Set up sensor pins:
  pinMode(lightPin, INPUT);
  pinMode(coPin, INPUT);
  pinMode(methanePin, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(XB_SLEEP_PIN, OUTPUT);

  digitalWrite(XB_SLEEP_PIN, LOW);

  // Set up serial ports:
  #ifdef DEBUG
      Serial.begin(9600);
  #endif
  
  // Make sure the XBEE BAUD RATE matches its pre-set value
  // (defaults to 9600).
  xB.begin(XBEE_BAUD);

  while(!commandMode(1)) {}
  commandOK("ATFR");
  delay(3000);

  // Set up WiFi network

  #ifdef DEBUG
  Serial.println("Testing network");
  #endif
  // connectWiFi will attempt to connect to the given SSID, using
  // encryption mode "encrypt", and the passphrase string given.
  connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);
  // Once connected, print out our IP address for a sanity check:

  #ifdef DEBUG
  Serial.println("Connected!");
  Serial.print("IP Address: "); printIP(); Serial.println(); 
  #endif

  // setupHTTP() will set up the destination address, port, and
  // make sure we're in TCP mode:
  setupHTTP(destIP);
  setXBeeSleepPinMode();
}

void setXBeeSleepPinMode() {
  
  while(!commandMode(1)) {}
  commandOK("ATD70");
  commandOK("ATD81");
  commandOK("ATD51");
  commandOK("ATSM1");
  commandMode(0);
}

//////////
// Loop //
//////////
// loop() constantly checks to see if enough time has lapsed
// (controlled by UPDATE_RATE) to allow a new stream of data
// to be posted.
// Otherwise, to kill time, it'll print out the sensor values
// over the serial port.
void loop()
{
  wakeXBee();
  
  #ifdef DEBUG
    Serial.print("Sending update...");
  #endif
  
  int attempts = 10;
  while(attempts > 0 && sendData() != 1) {
     // keep sending
      #ifdef DEBUG
      Serial.println("Failed to send data :(");
      #endif
      attempts--;
  }
  if(attempts > 0) {
    #ifdef DEBUG
      Serial.println("SUCCESS!");
    #endif
    test++;
  }

  sleepXBee();
  delay(UPDATE_RATE);
}

void wakeXBee() {
  #ifdef DEBUG
  Serial.println("Waking up XBee");

  #endif
  digitalWrite(XB_SLEEP_PIN, LOW);
  
  delay(5000);

 // connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);
 // setupHTTP(destIP);
}

void sleepXBee() {
  xB.flush();
  
  #ifdef DEBUG
  Serial.println("Sleeping XBee");
  #endif
  digitalWrite(XB_SLEEP_PIN, HIGH);
}

////////////////
// sendData() //
////////////////
// sendData() makes use of the PHANT LIBRARY to send data to the
// data.sparkfun.com server. We'll use phant.add() to add specific
// parameter and their values to the param list. Then use
// phant.post() to send that data up to the server.
int sendData()
{
  xB.flush(); // Flush data so we get fresh stuff in
  // IMPORTANT PHANT STUFF!!!
  // First we need to add fields and values to send as parameters
  // Since we just need to read values from the analog pins, this
  // can be automized with a for loop:
  readSensors(); // Get updated values from sensors.
  phant.add(moistureField, test);

  // After our PHANT.ADD's we need to PHANT.POST(). The post needs
  // to be sent out the XBee. A simple "print" of that post will
  // take care of it.
  String request = phant.post();
  Serial.println(request);

  int status;
  while ((status = checkConnect(WIFI_SSID)) != 0) {
      Serial.print("Waiting to connect: ");
      Serial.println(status);
  }
  commandMode(0);
  xB.print(request);

  // Check the response to make sure we receive a "200 OK". If 
  // we were good little programmers we'd check the content of
  // the OK response. If we were good little programmers...
  char response[12];
  if (waitForAvailable(12) > 0)
  {
    for (int i=0; i<12; i++)
    {
      response[i] = xB.read();
      delay(1);
    }
    if (memcmp(response, "HTTP/1.1 200", 12) == 0)
      return 1;
    else
    {
      #ifdef DEBUG
      Serial.println(response);
      #endif
      return 0; // Non-200 response
    }
  }
  else // Otherwise timeout, no response from server
    return -1;
}

// readSensors() will simply update a handful of global variables
// It updates tempVal, lightVal, coVal, and methaneVal
void readSensors()
{
  tempVal = ((analogRead(tempPin)*opVoltage/1024.0)-0.5)*100;
  tempVal = (tempVal * 9.0/5.0) + 32.0; // Convert to farenheit
  lightVal = analogRead(lightPin);
  methaneVal = analogRead(methanePin);
  coVal = analogRead(coPin);  
}

void checkOK() {
  #ifdef DEBUG
  
    if(xB.available() >= 2) {
      char c = xB.read();
      if(c == 'O') {
        Serial.print(c);
        c = xB.read();
        Serial.println(c);       
      } else {
        Serial.println("NOT OK");
      }
    }
    
  #endif
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
  commandOK("ATIP1"); // RESP: OK
  // Set DL (destination IP address)
  commandOK("ATDL" + address); // RESP: OK
  // Set DE (0x50 - port 80)
//  commandOK("ATDE50"); // RESP: OK
  commandOK("ATDE" + destPort);
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
  xB.flush();
  // Send the ATMY command. Should at least respond with
  // "0.0.0.0\r" (7 characters):
  command("ATMY", 7);
  // While there are characters to be read, read them and throw
  // them out to the serial monitor.
  while (xB.available() > 0)
  {
    Serial.write(xB.read());
  }
  Serial.println();
  // Exit command mode:
  commandMode(0);
}

//////////////////////////////
// connectWiFi(id, ee, psk) //
//////////////////////////////
// For all of your connecting-to-WiFi-networks needs, we present
// the connectWiFi() function. Supply it an SSID, encryption
// setting, and passphrase, and it'll try its darndest to connect
// to your network.
int connectWiFi(String id, byte auth, String psk)
{
  const String CMD_SSID = "ATID";
  const String CMD_ENC = "ATEE";
  const String CMD_PSK = "ATPK";
  // Check if we're connected. If so, sweet! We're done.
  // Otherwise, time to configure some settings, and print
  // some status messages:
  int status;
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
    #ifdef DEBUG
    Serial.print("Waiting to connect: ");
    Serial.println(status, HEX);
    #endif

    commandMode(1); // Enter command mode

    // Write AH (2 - Infrastructure) -- Locked in
    commandOK("ATAH2");
    // Write CE (2 - STA) -- Locked in
    commandOK("ATCE2");  
    // Write ID (SparkFun) -- Defined as parameter
    commandOK(CMD_SSID + id);
    // Write EE (Encryption Enable) -- Defined as parameter

    commandOK(CMD_ENC + auth);
    // Write PK ("sparkfun6175") -- Defined as parameter
    commandOK(CMD_PSK + psk);
    // Write MA (0 - DHCP) -- Locked in
    commandOK("ATMA0");
    // Write IP (1 - TCP) -- Loced in
    commandOK("ATIP1");

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
  char temp[2];
  commandMode(0);
  while (!commandMode(1))
    ;
  xB.flush();
  command("ATAI", 2);
  char c0 = xB.read();
  char c1 = xB.read();

  Serial.print("AI returned: ");
  Serial.print(c0);
  Serial.println(c1);

  temp[0] = hexToInt(c0);
  temp[1] = hexToInt(c1);

  xB.flush();

  if (temp[0] == 0)
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
      delay(1); // Was getting return status of 0xFE because we were exiting this loop too soon.
                // adding this delay seems to allow the data to transfer fast enough
    }
    if (atid == id)
      return 0;
    else {
      #ifdef DEBUG
      Serial.print(atid);
      Serial.println(" returned ID");
      Serial.print(id);
      Serial.println(" expected ID");
      #endif
      return 0xFE;
    }
  }
  else
  {
    if (temp[1] == 0x13)
      return temp[0];
    else
      return (temp[0]<<4) | temp[1];
  }
}

/*
void commandInt(String atcmd, int parm, int rsplen) {
  xB.flush();
  xB.print(atcmd);
  xB.write(parm);
}*/

/////////////////////////////////////
// Low-level, ugly, XBee Functions //
/////////////////////////////////////
void command(String atcmd, int rsplen)
{
  xB.flush();
  xB.print(atcmd);
  xB.print("\r");
  waitForAvailable(rsplen);
}

void commandOK(String atcmd) {
  xB.flush();
  xB.print(atcmd);
  xB.print("\r");
  Serial.print(atcmd);
  Serial.print(" - ");
  waitForAvailable(2);
  checkOK();
}

int commandMode(boolean enter)
{
  xB.flush();

  if (enter)
  {
    char c;
    delay(1000);
    xB.print("+++");   // Send CMD mode string
    delay(1000);
    
    Serial.println("+++");
    waitForAvailable(3);
    Serial.print(xB.available());
    Serial.println(" available characters after +++");
    if (xB.available() > 0)
    {
      c = xB.read();
      if (c == 'O') {// That's the letter 'O', assume 'K' is next
        Serial.println("OK");
        return 1; // IF we see "OK" return success
      }
    }
    Serial.println("Failed to enter command mode");
    return 0; // If no (or incorrect) receive, return fail
  }
  else
  {
    commandOK("ATCN");

    return 1;
  }
}

void printAvailable() {
    Serial.print(xB.available());
    Serial.println(" available characters");
    for(int i = 0; i < xB.available(); i++) {
      char c = xB.read();
      Serial.print(c);
    }
    Serial.println();
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

