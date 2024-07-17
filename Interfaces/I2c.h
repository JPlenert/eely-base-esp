// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef I2c_h
#define I2c_h

#include "driver/i2c.h"

class I2c
{
    private:
        int _gpio_sda;
        int _gpio_scl;
        int _freq;
        i2c_port_t _i2c_port;

    public: 
        I2c(int gpio_sda, int gpio_scl, int freq);
        esp_err_t Init();
        esp_err_t Write(int chip_addr, int data_addr, unsigned char data);
        esp_err_t Write(int chip_addr, int data_addr, int datalen, unsigned char* data);
        esp_err_t Read(int chip_addr, int data_addr, int datalen, unsigned char* data);
        // Checks if the chip Acks on addr request 
        esp_err_t Check(int chip_addr);
};

#endif