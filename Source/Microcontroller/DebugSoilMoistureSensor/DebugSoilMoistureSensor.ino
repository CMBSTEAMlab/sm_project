#include "CapacitiveSensor.h"

CapacitiveSensor   cs = CapacitiveSensor(9,10);        

void setup()                    
{
   cs.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
   Serial.begin(9600);
   while(!Serial) {}
}

void loop()                    
{
    long start = millis();
    long moistureReading =  cs.capacitiveSensorRaw(30);

    Serial.println(moistureReading); // print sensor output

    delay(1000); // 1 second delay between readings
}
