// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "ModbusRTUSimple.h"
#include "esp_check.h"
#include <string.h>
#include <stdio.h>

#define UART_BUFFER_SIZE 1024
static const char *TAG = "ModbusRTUSimple";

#define FUNC_WRITE_MULTIPLE 0x10
#define FUNC_WRITE_HOLDING_REGISTER 0x06
#define FUNC_READ_INPUT_REGISTERS 0x04
#define FUNC_READ_HOLDING_REGISTERS 0x03

esp_err_t ModbusRTUSimple :: Init()
{
    esp_err_t err;

    err = uart_driver_install(_port, UART_BUFFER_SIZE * 2, 0, 0, NULL, 0);
    ESP_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG, "uart_driver_install failed");

    err = uart_set_baudrate(_port, _baud);
    ESP_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG, "uart_set_baudrate failed");

    err = uart_set_word_length(_port, UART_DATA_8_BITS);
    ESP_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG, "uart_set_word_length failed");

    err = uart_set_parity(_port, UART_PARITY_EVEN);
    ESP_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG, "uart_set_parity failed");

    err = uart_set_stop_bits(_port, UART_STOP_BITS_1);
    ESP_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG, "uart_set_stop_bits failed");

    err = uart_set_hw_flow_ctrl(_port, UART_HW_FLOWCTRL_DISABLE, 122);
    ESP_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG, "uart_set_hw_flow_ctrl failed");

    err = uart_set_pin(_port, _tx_io_num, _rx_io_num, _rts_io_num, UART_PIN_NO_CHANGE);
    ESP_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG, "uart_set_pin failed");

    err = uart_set_mode(_port, UART_MODE_RS485_HALF_DUPLEX);
    ESP_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG, "uart_set_mode failed");

    return err;
}

uint16_t ModbusRTUSimple :: CalcCRC(unsigned char* frame, int len)
{
    uint16_t crc = 0xFFFF;
    int pos = 0;

    for (; pos < len; pos++)
    {
        crc ^= (uint16_t)frame[pos];          // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--)
        {    // Loop over each bit
            if ((crc & 0x0001) != 0)
            {      // If the LSB is set
                crc >>= 1;                    // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else                            // Else LSB is not set
                crc >>= 1;                    // Just shift right
        }
    }

    return crc;
}

void ModbusRTUSimple :: SetUInt16(unsigned char* frame, int index, uint16_t value)
{
    frame[index] = (unsigned char)((value & 0xFF00) >> 8);
    frame[index + 1] = (unsigned char)(value & 0xFF);
}

bool ModbusRTUSimple :: WriteRegister(int reg, int value)
{
    return WriteRegisters(reg, &value, 1);
}

bool ModbusRTUSimple :: WriteRegisters(int startRegister, int* values, int valueCount)
{
    unsigned char* frame;
    int frameLen = 1 + 1 + 2 + 2 + 1 + valueCount * 2 + 2;

    frame = (unsigned char*)malloc(frameLen);
    frame[0] = 0x01;    // Device-Address
    frame[1] = FUNC_WRITE_MULTIPLE;
    SetUInt16(frame, 2, startRegister); // Starting Address
    SetUInt16(frame, 4, (uint16_t)valueCount); // Number of Registers
    frame[6] = (unsigned char)(valueCount * 2); // Number of Bytes
    for (int i = 0; i < valueCount; i++)
        SetUInt16(frame, 7 + i * 2, values[i]);

    ModbusRTUResponse* response = CalcFrameSendAndReceive(frame, frameLen);
    free(frame);

    if (response != nullptr)
    {
        delete response;
        return true;
    }
    return false;
}

ModbusRTUResponse* ModbusRTUSimple :: ReadInputRegisters(int startRegister, int registerCount)
{
    unsigned char frame[1 + 1 + 2 + 2 + 2];
    frame[0] = 0x01;    // Device-Address
    frame[1] = FUNC_READ_INPUT_REGISTERS;
    SetUInt16(frame, 2, startRegister); // Starting Address
    SetUInt16(frame, 4, (uint16_t)registerCount); // Number of Registers

    return CalcFrameSendAndReceive(frame, 1 + 1 + 2 + 2 + 2);
}

ModbusRTUResponse* ModbusRTUSimple :: ReadHoldingRegisters(int startRegister, int registerCount)
{
    unsigned char frame[1 + 1 + 2 + 2 + 2];
    frame[0] = 0x01;    // Device-Address
    frame[1] = FUNC_READ_HOLDING_REGISTERS;
    SetUInt16(frame, 2, startRegister); // Starting Address
    SetUInt16(frame, 4, (uint16_t)registerCount); // Number of Registers

    return CalcFrameSendAndReceive(frame, 1 + 1 + 2 + 2 + 2);
}

ModbusRTUResponse* ModbusRTUSimple :: CalcFrameSendAndReceive(unsigned char* frame, int frameLen)
{
    // Calc & set crc
    uint16_t crc = CalcCRC(frame, frameLen - 2);
    memcpy(&frame[frameLen - 2], &crc, 2);

    // Display output frame
    // for (int i=0; i<frameLen; i++)
    //     ESP_LOGI(TAG, "Byte %d - %x", i, frame[i]);

    // Sending frame
    int byteCou = uart_write_bytes(_port, frame, frameLen);
    // Remove all "garbage" from input buffer
    // uart_flush_input(_port);
    ESP_LOGI(TAG, "Sent %d bytes frame", byteCou);

    return ReadResponse();
}

#define TIMEOUT 50

ModbusRTUResponse* ModbusRTUSimple :: ReadResponse()
{
    int inputBufferLen = 30;
    unsigned char* inputBuffer = (unsigned char*)malloc(inputBufferLen);

    ModbusRTUResponse* response = nullptr;
    int addr;
    int func;   
    int curLen;

    // get first bytes - must be at lease 4
    curLen = uart_read_bytes(_port, inputBuffer, 4, TIMEOUT);
    ESP_LOGI(TAG, "Got %d bytes", curLen);
    if (curLen != 4)  
        goto end;  

    addr = inputBuffer[0];
    func = inputBuffer[1];

    if (func == FUNC_WRITE_MULTIPLE)
    {
        // read 4 more bytes
        curLen = uart_read_bytes(_port, &inputBuffer[4], 4, TIMEOUT);
        ESP_LOGI(TAG, "Got %d bytes on FUNC_WRITE_MULTIPLE", curLen);
        if (curLen != 4)
            goto end;  
        
        if (!CheckCRC(inputBuffer, 8))
        {            
            ESP_LOGI(TAG, "Received invalid CRC on FUNC_WRITE_MULTIPLE");
            goto end;  
        }

        response = new ModbusRTUResponse(addr, func);
        response->AssignData(inputBuffer, 2, 4);
    }
    else if (func == FUNC_READ_INPUT_REGISTERS || func == FUNC_READ_HOLDING_REGISTERS)
    {
        int dataLen = inputBuffer[2];

        if (dataLen+5 > inputBufferLen)
            inputBuffer = (unsigned char*)realloc(inputBuffer, dataLen+5 + 10);

        // read rest
        curLen = uart_read_bytes(_port, &inputBuffer[4], dataLen+5-4, TIMEOUT);
        ESP_LOGI(TAG, "Got %d bytes on FUNC_READ_INPUT_REGISTERS", curLen);
        if (curLen != dataLen+5-4)
        {            
            ESP_LOGI(TAG, "Received invalid data length on FUNC_READ_INPUT_REGISTERS");
            goto end;  
        }

        if (!CheckCRC(inputBuffer, dataLen+5))
        {            
            ESP_LOGI(TAG, "Received invalid CRC on FUNC_READ_INPUT_REGISTERS");
            goto end;  
        }

        response = new ModbusRTUResponse(addr, func);
        response->AssignData(inputBuffer, 3, dataLen);
    }

end:
    free(inputBuffer);
    return response;
}

bool ModbusRTUSimple :: CheckCRC(unsigned char* frame, int len)
{
    return CalcCRC(frame, len - 2) == *((uint16_t*)&frame[len-2]);
}

void ModbusRTUSimple :: Test()
{
    while (true){
        ESP_LOGI(TAG, "Starting WriteRegister");
        if (WriteRegister(258, 4))
        {
            ESP_LOGI(TAG, "Finishes WriteRegister successfully");
        }
        else
        {
            ESP_LOGI(TAG, "Finishes WriteRegister with failure");
        }

        vTaskDelay(500);
    }
    
    ESP_LOGI(TAG, "Init data");
    unsigned char dataOut[] = {0x01, 0x10, 0x01, 0x02, 0x00, 0x01, 0x02, 0x00, 0x04, 0xb6, 0xb1};
    unsigned char dataIn[20];

    while (true){
        ESP_LOGI(TAG, "Sending");
        int err = uart_write_bytes(_port, dataOut, 11);
        ESP_LOGI(TAG, "Sent %d bytes", err);

        int len = uart_read_bytes(_port, dataIn, 10, 100);
        ESP_LOGI(TAG, "Got %d bytes", len);
        for (int i=0; i<len; i++)
            ESP_LOGI(TAG, "Byte %d - %x", i, dataIn[i]);
    }
}

void ModbusRTUResponse :: AssignData(unsigned char* frame, int start, int len)
{
    dataLen = len;
    data = (unsigned char*)malloc(len);
    memcpy(data, &frame[start], len);
}

ModbusRTUResponse :: ~ModbusRTUResponse()
{
    if (data != nullptr){
        free(data);
        data = nullptr;
    }
}

int16_t ModbusRTUResponse :: GetInt16FromData(int idx)
{
    return (int16_t)((((int)data[idx*2])<<8) + (int)data[idx*2+1]);
}

uint16_t ModbusRTUResponse :: GetUInt16FromData(int idx)
{
    return (uint16_t)((((int)data[idx*2])<<8) + (int)data[idx*2+1]);
}
