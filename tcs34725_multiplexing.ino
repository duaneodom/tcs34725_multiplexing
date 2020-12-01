#include <Wire.h>
#include "tcs34725.h"


#define SENSOR1     26
#define SENSOR2     27


tcs34725_color s1_rgb_value;
tcs34725_color s2_rgb_value;
int blueness = 0;


void setup()
{
    Serial.begin(9600);

    pinMode(SENSOR1, OUTPUT);
    digitalWrite(SENSOR1, HIGH);
    pinMode(SENSOR2, OUTPUT);
    digitalWrite(SENSOR2, LOW);

    Wire.begin();
}


tcs34725_color read_sensor(uint8_t sensor_pin)
{
    tcs34725_color rv;

    digitalWrite(sensor_pin, HIGH);
    delay(5);
    tcs34725_init();
    tcs34725_set_integration_time(TCS34725_MASK_INT_TIME_24ms);
    tcs34725_set_gain(TCS34725_MASK_GAIN_1X);

    while (!tcs34725_available())
    {
    }

    rv = tcs34725_get_mapped_color();
    digitalWrite(sensor_pin, LOW);
    return rv;
}


void loop()
{
//    Serial.println("reading sensor 1..");
    s1_rgb_value = read_sensor(SENSOR1);
    blueness = tcs34725_blueness(s1_rgb_value);
    Serial.print("s1 blueness:");
    Serial.println(blueness);
//    Serial.print("S1:");
//    Serial.print(s1_rgb_value.r);
//    Serial.print(",");
//    Serial.print(s1_rgb_value.g);
//    Serial.print(",");
//    Serial.println(s1_rgb_value.b);

//    Serial.println("reading sensor 2..");
    s2_rgb_value = read_sensor(SENSOR2);
    blueness = tcs34725_blueness(s2_rgb_value);
    Serial.print("s2 blueness:");
    Serial.println(blueness);

//    delay(1000);
}
