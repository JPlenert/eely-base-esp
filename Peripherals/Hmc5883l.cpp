// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include <stdio.h>
#include "Hmc5883l.h"
#include "esp_log.h"

#define HMC_ADDR_CONFIG_A 0
#define HMC_ADDR_CONFIG_B 1
#define HMC_ADDR_MODE 2
#define HMC_ADDR_X 3

#define QMC_ADDR_X 0
#define QMC_ADDR_CONROL 9

static const char *TAG = "Hmc5883l";

Hmc5883l::Hmc5883l(I2c &i2c) : _i2c(i2c) { }

void Hmc5883l::Init()
{
    for (int i=0; i<5; i++)
    {
        ESP_LOGI(TAG, "Checking for Hmc5883l @ 0x1e");    
        if (_i2c.Check(0x1e) == ESP_OK)
        {        
            ESP_LOGI(TAG, "Found Hmc5883l @ 0x1e");    
            _isQmc = false;
            _chipAddr = 0x1e;
            break;
        }
        else 
        {
            ESP_LOGI(TAG, "Checking for Qmc5883l @ 0x0d");
            if (_i2c.Check(0x0d) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found Qmc5883l @ 0x0d");    
                _isQmc = true;
                _chipAddr = 0x0d;
                break;
            }
            else
                ESP_LOGE(TAG, "Unable to find Hmc chip");
        }

         vTaskDelay(500);
    }

    if (_isQmc)
        _i2c.Write(_chipAddr, QMC_ADDR_CONROL, 0b10000001); // => 512 Oversampling, 2G, 10Hz, Continous
    else
        _i2c.Write(_chipAddr, HMC_ADDR_MODE, 0); // => Continous mode

    SetGain(0);
}

void Hmc5883l::SetGain(int gain)
{  
    if (_isQmc)
    {
        if (gain == 0)
        {
            _i2c.Write(_chipAddr, QMC_ADDR_CONROL, 0b10010001); // => 512 Oversampling, 8G, 10Hz, Continous
            ESP_LOGI(TAG, "setGain to 8Ga");
        }
        else
        {
            _i2c.Write(_chipAddr, QMC_ADDR_CONROL, 0b10000001); // => 512 Oversampling, 2G, 10Hz, Continous
            ESP_LOGI(TAG, "setGain to 2Ga");
        }
    }
    else
    {
        int gainRegister;

        if (gain == 1)
        {
            gainRegister = 0b011 << 5;  // 2.5Ga
            ESP_LOGI(TAG, "setGain to 2.5Ga");
        }
        else if (gain == 2)
        {
            gainRegister = 0b001 << 5;  // 1.3Ga
            ESP_LOGI(TAG, "setGain to 1.3Ga");
        }
        else
        {
            gainRegister = 0b111 << 5;  // 8Ga
            ESP_LOGI(TAG, "setGain to 8Ga");
        }

        _i2c.Write(_chipAddr, HMC_ADDR_CONFIG_B, gainRegister);
    }
}

void Hmc5883l::Read()
{
    unsigned char data[6];
    esp_err_t err;

    if (_isQmc)
    {
        if ((err = _i2c.Read(_chipAddr, QMC_ADDR_X, 6, data)) == ESP_OK)
        {    
            x = (int)((int16_t)(data[1] << 8 | data[0]));
            y = (int)((int16_t)(data[3] << 8 | data[2]));
            z = (int)((int16_t)(data[5] << 8 | data[4]));
        }
    }
    else
    {
        if ((err = _i2c.Read(_chipAddr, HMC_ADDR_X, 6, data)) == ESP_OK)
        {    
            x = (int)((int16_t)(data[0] << 8 | data[1]));
            z = (int)((int16_t)(data[2] << 8 | data[3]));
            y = (int)((int16_t)(data[4] << 8 | data[5]));
        }
    }

    if (err != ESP_OK)
    {
        x = 0x80004001;
        y = 0x80004001;
        z = 0x80004001;
    }
}