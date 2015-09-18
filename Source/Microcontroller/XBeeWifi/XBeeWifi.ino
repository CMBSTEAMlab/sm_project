#include <Phant.h>
#include <Narcoleptic.h>
#include <CapacitiveSensor.h>

//#define DEBUG // uncomment this to print debugging information to Serial monitor, 
                // comment out when not connected via USB or program will not progress past setup
#define COMMAND_TIMEOUT 15000
#define XB_SERIAL Serial1

const String WIFI_SSID = "CMB STEAMlab";
const String WIFI_PSK = "CMBGuest0987";
const String stationID = "CMB_MONTERRA1"; // ID of the monitoring station, this value will be sent to phant

enum encryption{NO_SECURITY, WPA_TKIP, WPA2_AES, WEP};
const encryption WIFI_EE = WPA2_AES;


//const String destIP = "192.168.1.122";
//const String destPort = "1F90"; // hex for port 8080
//const String destIP = "54.86.132.254"; // data.sparkfun.com's IP address
//const String destPort = "50"; // hex for port 80

const String destIP = "69.145.204.62"; // CMB IP address
const String destPort = "1F90"; // hex for port 8080

//Phant phant("data.sparkfun.com", "8dLOAE2157Tp9xqmWNz3", "pzJvWEV6oYTXdPxE8Kpm");
//Phant phant("192.168.1.122", "Z0wjAMb8YDSlNxKV6ZexuMgaMAz", "x7OjVwl9QgIvEyjMgqmyFgKegVO");
Phant phant("69.145.204.62", "mqBpa2Gdg6urqo4XzA6rCGvMeMEJ", "jLNzWvK07VhDlaeb9x5DCD0jQjmL");
//Phant phant("69.145.204.62", "vWoxJEqQvASpYL9vlB2JiyPb6yd", "r3ZqdGpP6WFwqjg8m37ES04qQ0d");

const String moistureField = "moisture";
const String stationIDField = "stationid";
const String attemptField = "attempt";
const String sequenceField = "sequence";

const byte XB_SLEEP_PIN = 6; // XBee's DTR pin
const byte XB_ON_PIN = 13; // XBee's ON pin
const int XBEE_BAUD = 9600; // Your XBee's baud (9600 is default)

CapacitiveSensor cs = CapacitiveSensor(9,10); 
long moistureReading;
unsigned long seq = 0;

const unsigned long DEBUG_UPDATE_DELAY = 20000;
const unsigned long UPDATE_DELAY = 300000;
//const unsigned long UPDATE_DELAY = 20000;

void setup() {
  cs.set_CS_AutocaL_Millis(0xFFFFFFFF);
  
  pinMode(XB_SLEEP_PIN, OUTPUT);
  digitalWrite(XB_SLEEP_PIN, LOW);
  
  pinMode(XB_ON_PIN, INPUT);

#ifdef DEBUG
  Serial.begin(9600);
  while(!Serial) {
  }
  Serial.println("In setup()");
#endif
  
  XB_SERIAL.begin(XBEE_BAUD);

#ifdef DEBUG
  Serial.println("Resetting XBee to factory defaults");
#endif
  enterCommandMode();
  command("ATRE"); // reset XBee to factory defaults
  while(!waitForOK()) {
    command("ATRE");
  }
  delay(3000);
}

void setupSleepMode() {
  command("ATD50");
  if(!waitForOK()) {
  }

  command("ATD70");
  if(!waitForOK()) {
  }

  command("ATD81");
  if(!waitForOK()) {
  }

  command("ATSM1");
  if(!waitForOK()) {
  }
}

void readData() {
  moistureReading = cs.capacitiveSensorRaw(30);
}

void loop() {
  seq++;
  
  wakeXBee();

  readData();

  int attempts = 0;
  while(!sendData(attempts+1) && attempts < 10) {
     attempts++;
#ifdef DEBUG
     Serial.print("Failed attempt ");
     Serial.println(attempts);
#endif
  }

  sleepXBee();

#ifdef DEBUG
  Serial.flush();
#endif

  delay(1000);

#ifdef DEBUG
  delay(DEBUG_UPDATE_DELAY); // use this when connected to USB to debug with the Serial monitor
#else
  Narcoleptic.delay(UPDATE_DELAY); // don't use when connected to USB
#endif
}

void wakeXBee() {
#ifdef DEBUG
  Serial.println("Waking up");
#endif

  digitalWrite(XB_SLEEP_PIN, LOW);
  delay(1000);
  
  enterCommandMode();

#ifdef DEBUG
  Serial.println("Resetting network settings");
#endif

  command("ATNR");
  if(!waitForOK()) {
  }
  delay(2000);

  enterCommandMode();
  
  connectToWiFi();
  setupHTTP();
  setupSleepMode();
  
  exitCommandMode();
}

void sleepXBee() {
#ifdef DEBUG
  Serial.println("Sleeping");
#endif

  emptySerialBuffer();
  digitalWrite(XB_SLEEP_PIN, HIGH);
}

bool sendData(int attempt) {
  bool success = false;
  XB_SERIAL.flush();

  phant.add(moistureField, moistureReading);
  phant.add(stationIDField, stationID);
  phant.add(attemptField, attempt);
  phant.add(sequenceField, seq);
  String request = phant.post();

  int status;
  while((status = checkWiFiConnection()) != 0) {
#ifdef DEBUG
    Serial.print("Waiting to connect to wifi sendData: ");
    Serial.print(status, HEX);
    Serial.println();
#endif
  }

  command("ATCN");
  XB_SERIAL.flush();
  delay(2000);
  emptySerialBuffer();

#ifdef DEBUG
  Serial.println("Attempting to send request:");
  Serial.println(request);
#endif

  delay(2000);
  writeToXB(request);
  delay(2000);
  char response[12];
  waitForAvailable(12);
  if(XB_SERIAL.available() >= 12) {
    for(int i = 0; i < 12; i++) {
      response[i] = XB_SERIAL.read();
    }
    if(memcmp(response, "HTTP/1.1 200", 12) == 0) {
#ifdef DEBUG
      Serial.println("SUCCESS!");
#endif
      success = true;
    } else {
#ifdef DEBUG
      Serial.println("FAILURE!");
      Serial.println(response);
#endif
    }
  } else {
#ifdef DEBUG
    Serial.println("Request timed out");
#endif
  }

  emptySerialBuffer();
  return success;
}

void setupHTTP() {
  command("ATIP1"); // set protocol to TCP
  if(!waitForOK()) {
  }

  const String CMD_ATDL = "ATDL";
  command(CMD_ATDL + destIP); // set ip address for our requests
  if(!waitForOK()) {
  }

  const String CMD_ATDE = "ATDE";
  command(CMD_ATDE + destPort); // set port for our requests
  if(!waitForOK()) {
  }
}

byte checkWiFiConnection() {
  enterCommandMode();
/*
Read information regarding last node join request:
0x00 - Successfully joined an access point, established IP addresses and IP listening
sockets.
0x01 - WiFi transceiver initialization in progress.
0x02 - WiFi transceiver initialized, but not yet scanning for access point.
0x13 - Disconnecting from access point.
0x22 – Configured SSID not found
0x23 – SSID not configured.
0x27 – SSID was found, but join failed.
0x41 – Module joined a network and is waiting for IP configuration to complete,
which usually means it is waiting for a DHCP provided address.
0x42 – Module is joined, IP is configured, and listening sockets are being set up.
0xFF– Module is currently scanning for the configured SSID.

0xDD - Added failure executing ATAI command
*/

  byte status;
  
  command("ATAI");
  waitForAvailable(2);
  if(XB_SERIAL.available() >= 2) {
    char c0 = XB_SERIAL.read();
    char c1 = XB_SERIAL.read();

#ifdef DEBUG
    Serial.print("ATAI returned ");
    Serial.print(c0, HEX);
    Serial.print(c1, HEX);
    Serial.println();
#endif

    if(c1 == 0xD) {
      status = hexToInt(c0);
    } else {
      status = hexToInt(c0) << 4 | hexToInt(c1);
    }
  } else {
    status = 0xDD;
  }

  exitCommandMode();
  
  return status;
}

void connectToWiFi() {
  command("ATAH2"); // set network type to Infrastructure
  if(!waitForOK()) {
  }

  command("ATCE2");  // ???
  if(!waitForOK()) {
  }

  const String CMD_ATID = "ATID";
  command(CMD_ATID + WIFI_SSID); // set network SSID
  if(!waitForOK()) {
  }

  byte ee = WIFI_EE;
  command("ATEE2"); // set network encryption
  if(!waitForOK()) {
  }

  const String CMD_ATPK = "ATPK";
  command(CMD_ATPK + WIFI_PSK); // set network password, no commas allowed in password as it's a limitation of the XBee command mode (commas can separate multiple commands)
  if(!waitForOK()) {
  }

  command("ATMA0"); // set addressing mode to DHCP
  if(!waitForOK()) {
  }

  command("ATIP1"); // set protocol to TCP
  if(!waitForOK()) {
  }
}

void emptySerialBuffer() {
  XB_SERIAL.flush();
  while(XB_SERIAL.available() > 0) {
    XB_SERIAL.read();
  }
}

void enterCommandMode() {
#ifdef DEBUG
  Serial.println("+++ Entering command mode");
#endif

  emptySerialBuffer();

  delay(2000);
  writeToXB("+++"); 
  delay(1000);
  while(!waitForOK()) {
#ifdef DEBUG
      Serial.println("Failed to enter command mode, retrying +++");
#endif

      delay(5000);
      writeToXB("+++"); 
      delay(1000);
  }
}

void exitCommandMode() {
#ifdef DEBUG
  Serial.println("Exiting command mode");
#endif
  command("ATCN");
  if(!waitForOK()) {
#ifdef DEBUG
    Serial.println("Failed to exit command mode");
#endif
  }
}

// Waits for an OK or times out after COMMAND_TIMEOUT milliseconds
// Returns true if OK was returned or false if it timed out or something other than OK was returned
bool waitForOK() {
    waitForAvailable(3);
    if(XB_SERIAL.available() >= 3) {
      char c = XB_SERIAL.read();
      if(c == 'O') {
          c = XB_SERIAL.read();
          if(c == 'K') {
            c = XB_SERIAL.read();
            if(c == '\r') {
#ifdef DEBUG
              Serial.println("OK");
              Serial.print(XB_SERIAL.available());
              Serial.println();
#endif
              return true;
            }
          }
      }
    }

#ifdef DEBUG
    Serial.println("NOT OK");
    Serial.print(XB_SERIAL.available());
    Serial.println();
    while(XB_SERIAL.available()) {
      Serial.write(XB_SERIAL.read());
    }
    Serial.println();
#endif
    
    return false;
}

int waitForAvailable(int numBytes) {
    unsigned int timeout = COMMAND_TIMEOUT;
    while((--timeout > 0) && XB_SERIAL.available() < numBytes) {
        delay(1);
    }

    return timeout;
}

void command(String atcmd) {
  emptySerialBuffer();
  writeToXB(atcmd);
  writeToXB("\r");
  delay(10);
#ifdef DEBUG
  Serial.println(atcmd);
#endif
}

void writeToXB(String atcmd) {
  /*
    while(!digitalRead(XB_ON_PIN)) {
#ifdef DEBUG
        Serial.println("XBee ON pin OFF, not sending serial data...");
        delay(100);
#endif
    }
    */
    XB_SERIAL.print(atcmd);
    delay(10);
}

byte hexToInt(char c)
{
  if (c >= 0x41) // If it's A-F
    return c - 0x37;
  else
    return c - 0x30; // otherwise its 0-9
}
