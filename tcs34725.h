#pragma once
#include <Arduino.h>
#include <Wire.h>




static const uint8_t TCS34725_I2C_ADDR                  = 0x29;
static const uint8_t TCS34725_PART_NUM                  = 0x44;

static const uint8_t TCS34725_CMD_BIT                   = 0x80;
static const uint8_t TCS34725_CMD_SPECIAL_CLEAR_INT     = 0x66;

static const uint8_t TCS34725_ADDR_ENABLE               = 0x00;
static const uint8_t TCS34725_ADDR_ID                   = 0x12;
static const uint8_t TCS34725_ADDR_PERSISTENCE          = 0x0C;
static const uint8_t TCS34725_ADDR_RGBC_TIME            = 0x01;
static const uint8_t TCS34725_ADDR_CONTROL              = 0x0F;
static const uint8_t TCS34725_ADDR_STATUS               = 0x13;
static const uint8_t TCS34725_ADDR_RGBC_CDATA           = 0x14;
static const uint8_t TCS34725_ADDR_RGBC_RDATA           = 0x16;
static const uint8_t TCS34725_ADDR_RGBC_GDATA           = 0x18;
static const uint8_t TCS34725_ADDR_RGBC_BDATA           = 0x1A;

static const uint8_t TCS34725_MASK_ENABLE_AIEN          = 0x10;
static const uint8_t TCS34725_MASK_ENABLE_AEN           = 0x02;
static const uint8_t TCS34725_MASK_ENABLE_PON           = 0x01;
static const uint8_t TCS34725_MASK_STATUS_AINT          = 0x10;
static const uint8_t TCS34725_MASK_INT_TIME_2_4ms       = 0xFF;
static const uint8_t TCS34725_MASK_INT_TIME_24ms        = 0xF6;
static const uint8_t TCS34725_MASK_INT_TIME_50ms        = 0xEB;
static const uint8_t TCS34725_MASK_INT_TIME_101ms       = 0xD5;
static const uint8_t TCS34725_MASK_INT_TIME_154ms       = 0xC0;
static const uint8_t TCS34725_MASK_INT_TIME_700ms       = 0x00;
static const uint8_t TCS34725_MASK_GAIN_1X              = 0x00;
static const uint8_t TCS34725_MASK_GAIN_4X              = 0x01;
static const uint8_t TCS34725_MASK_GAIN_16X             = 0x02;
static const uint8_t TCS34725_MASK_GAIN_60X             = 0x03;



struct tcs34725_color { uint16_t r, g, b, c; };



tcs34725_color current_color;
tcs34725_color black = {.r=4, .g=2, .b=2, .c=0};
tcs34725_color white = {.r=31, .g=17, .b=13, .c=255};



bool tcs34725_init();
bool tcs34725_available();
tcs34725_color tcs34725_get_color();
void tcs34725_set_integration_time(uint8_t);
void tcs34725_set_gain(uint8_t);
void tcs34725_update();
void tcs34725_power(bool);
void tcs34725_set_interrupt();
void tcs34725_clear_interrupt();
void tcs34725_write8(uint8_t,uint8_t);
uint8_t tcs34725_read8(uint8_t);
uint16_t tcs34725_read16(uint8_t);
int tcs34725_redness(tcs34725_color);
int tcs34725_greenness(tcs34725_color);
int tcs34725_blueness(tcs34725_color);



bool tcs34725_init()
{
    bool response = false;

    uint8_t part_id = tcs34725_read8(TCS34725_ADDR_ID);

    if (part_id == TCS34725_PART_NUM)
    {
        tcs34725_power(true);                                   // turn power on
        tcs34725_set_interrupt();                               // interrupts for tcs34725_available checking
        tcs34725_write8(TCS34725_ADDR_PERSISTENCE, 0x00);       // persistence causes an interrupt on every rgb read
        response = true;
    }

    return response;
}



bool tcs34725_available()
{
    bool response = tcs34725_read8(TCS34725_ADDR_STATUS) & TCS34725_MASK_STATUS_AINT;

    if (response)
    {
        tcs34725_update();
        tcs34725_clear_interrupt();
    }

    return response;
}



tcs34725_color tcs34725_get_raw_color()
{
    return current_color;
}



tcs34725_color tcs34725_get_mapped_color()
{
    tcs34725_color mapped_color;

    mapped_color.r = constrain(map(current_color.r, black.r, white.r, 0, 255), 0, 255);
    mapped_color.g = constrain(map(current_color.g, black.g, white.g, 0, 255), 0, 255);
    mapped_color.b = constrain(map(current_color.b, black.b, white.b, 0, 255), 0, 255);
    mapped_color.c = constrain(map(current_color.c, black.c, white.c, 0, 255), 0, 255);

    return mapped_color;
}



void tcs34725_set_integration_time(uint8_t time)
{
    tcs34725_write8(TCS34725_ADDR_RGBC_TIME, time);
}



void tcs34725_set_gain(uint8_t gain)
{
    tcs34725_write8(TCS34725_ADDR_CONTROL, gain);
}



void tcs34725_update()
{
    current_color.r = tcs34725_read16(TCS34725_ADDR_RGBC_RDATA);
    current_color.g = tcs34725_read16(TCS34725_ADDR_RGBC_GDATA);
    current_color.b = tcs34725_read16(TCS34725_ADDR_RGBC_BDATA);
    current_color.c = tcs34725_read16(TCS34725_ADDR_RGBC_CDATA);
}



void tcs34725_power(bool on)
{
    if (on)
    {
        tcs34725_write8(TCS34725_ADDR_ENABLE, TCS34725_MASK_ENABLE_PON);
        delay(3); // wait 2.4 ms for RGBC to initialize
        tcs34725_write8(TCS34725_ADDR_ENABLE, TCS34725_MASK_ENABLE_PON | TCS34725_MASK_ENABLE_AEN);
    }
    else
    {
        uint8_t enable_reg = tcs34725_read8(TCS34725_ADDR_ENABLE);
        tcs34725_write8(TCS34725_ADDR_ENABLE, enable_reg & ~(TCS34725_MASK_ENABLE_PON | TCS34725_MASK_ENABLE_AEN));
    }
}



void tcs34725_set_interrupt()
{
    uint8_t enable_reg = tcs34725_read8(TCS34725_ADDR_ENABLE);
    enable_reg |= TCS34725_MASK_ENABLE_AIEN;
    tcs34725_write8(TCS34725_ADDR_ENABLE, enable_reg);
}



void tcs34725_clear_interrupt()
{
    Wire.beginTransmission(TCS34725_I2C_ADDR);
    Wire.write(TCS34725_CMD_BIT | TCS34725_CMD_SPECIAL_CLEAR_INT);
    Wire.endTransmission();
}



void tcs34725_write8(uint8_t addr, uint8_t val)
{
    Wire.beginTransmission(TCS34725_I2C_ADDR);
    Wire.write(TCS34725_CMD_BIT | addr);
    Wire.write(val);
    Wire.endTransmission();
}



uint8_t tcs34725_read8(uint8_t addr)
{
    Wire.beginTransmission(TCS34725_I2C_ADDR);
    Wire.write(TCS34725_CMD_BIT | addr);
    Wire.endTransmission();
    Wire.requestFrom(TCS34725_I2C_ADDR, (uint8_t)1);

    return Wire.read();
}



uint16_t tcs34725_read16(uint8_t addr)
{
    uint16_t low_byte;
    uint16_t high_byte;

    Wire.beginTransmission(TCS34725_I2C_ADDR);
    Wire.write(TCS34725_CMD_BIT | addr);
    Wire.endTransmission();

    Wire.requestFrom(TCS34725_I2C_ADDR, (uint8_t)2);
    low_byte = Wire.read();
    high_byte = Wire.read();
    high_byte <<= 8;
    high_byte |= low_byte;

    return high_byte;
}



int tcs34725_redness(tcs34725_color c)
{
    return c.r - (int)((c.g+c.b)/2.0);
}



int tcs34725_greenness(tcs34725_color c)
{
    return c.g - (int)((c.r+c.g)/2.0);
}



int tcs34725_blueness(tcs34725_color c)
{
    return c.b - (int)((c.r+c.g)/2.0);
}
