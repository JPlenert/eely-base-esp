// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include <stdio.h>
#include "I2c.h"

I2c::I2c(int gpio_sda, int gpio_scl, int freq)
{
    _i2c_port = I2C_NUM_0;
    _gpio_sda = gpio_sda;
    _gpio_scl = gpio_scl;
    _freq = freq;
}

esp_err_t I2c::Init()
{
    esp_err_t err = ESP_OK;

    err = i2c_driver_install(_i2c_port, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK)
        return err;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = _gpio_sda,
        .scl_io_num = _gpio_scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE
    };
    conf.master.clk_speed = _freq;

    return i2c_param_config(_i2c_port, &conf);
}

esp_err_t I2c::Write(int chip_addr, int data_addr, unsigned char data)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    err = i2c_master_start(cmd);
    
    if (err == ESP_OK)
        err = i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, true);

    if (err == ESP_OK)
        err = i2c_master_write_byte(cmd, data_addr | I2C_MASTER_WRITE, true);

    if (err == ESP_OK)
        err = i2c_master_write_byte(cmd, data, true);

    if (err == ESP_OK)
        err = i2c_master_stop(cmd);

    if (err == ESP_OK)
        err = i2c_master_cmd_begin(_i2c_port, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);    

    return err;
}


esp_err_t I2c::Write(int chip_addr, int data_addr, int datalen, unsigned char* data)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    err = i2c_master_start(cmd);
    
    if (err == ESP_OK)
        err = i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, true);

    if (err == ESP_OK)
        err = i2c_master_write_byte(cmd, data_addr | I2C_MASTER_WRITE, true);

    if (err == ESP_OK)
        err = i2c_master_write(cmd, data, datalen, true);

    if (err == ESP_OK)
        err = i2c_master_stop(cmd);

    if (err == ESP_OK)
        err = i2c_master_cmd_begin(_i2c_port, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);    

    return err;
}

esp_err_t I2c::Read(int chip_addr, int data_addr, int datalen, unsigned char* data)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    err = i2c_master_start(cmd);
    
    if (err == ESP_OK)
        err = i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, true);

    if (err == ESP_OK)
        err = i2c_master_write_byte(cmd, data_addr, true);

    if (err == ESP_OK)
        err = i2c_master_start(cmd);

    if (err == ESP_OK)
        err = i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_READ, true);
    
    if (err == ESP_OK && datalen > 1)
        err = i2c_master_read(cmd, data, datalen - 1, I2C_MASTER_ACK);

    if (err == ESP_OK)
        err = i2c_master_read_byte(cmd, data + datalen - 1, I2C_MASTER_LAST_NACK);

    if (err == ESP_OK)
        err = i2c_master_stop(cmd);

    if (err == ESP_OK)
        err = i2c_master_cmd_begin(_i2c_port, cmd, 50 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);

    return err;
}

esp_err_t I2c::Check(int chip_addr)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    err = i2c_master_cmd_begin(_i2c_port, cmd, 50 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);

    return err;
}
