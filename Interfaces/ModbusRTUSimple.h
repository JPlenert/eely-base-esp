// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef ModbusRTUSimple_h
#define ModbusRTUSimple_h

#include "esp_log.h"
#include "driver/uart.h"
#include <list>
#include <vector>
#include <optional>

using namespace std;

class ModbusRTUResponse {
    public:
        int deviceAddress;
        int functionCode;
        int dataLen;
        unsigned char *data;

    public:
        ModbusRTUResponse(int addr, int func) : deviceAddress(addr), functionCode(func), dataLen(0), data(nullptr) {}
        ~ModbusRTUResponse();
        void AssignData(unsigned char* frame, int start, int len);

        int16_t GetInt16FromData(int idx);
        uint16_t GetUInt16FromData(int idx);

};

class ModbusRTUSimple {
    private:
        uart_port_t _port;
        int _tx_io_num;
        int _rx_io_num;
        int _rts_io_num;
        int _baud;
        uart_parity_t _parity;

    public:
        ModbusRTUSimple(int baud, uart_parity_t parity) : ModbusRTUSimple(UART_NUM_2, 17, 16, 18, baud, parity) {}
        ModbusRTUSimple(uart_port_t port, int tx_io_num, int rx_io_num, int rts_io_num, int baud, uart_parity_t parity) : _port(port), _tx_io_num(tx_io_num), _rx_io_num(rx_io_num), _rts_io_num(rts_io_num), _baud(baud), _parity(parity) {}

    private:
        uint16_t CalcCRC(unsigned char* frame, int len);
        bool CheckCRC(unsigned char* frame, int len);
        void SetUInt16(unsigned char* frame, int index, uint16_t value);
        ModbusRTUResponse* CalcFrameSendAndReceive(unsigned char* frame, int frameLen);
        ModbusRTUResponse* ReadResponse();

    public:
        esp_err_t Init();
        void Test();
        bool WriteRegister(int reg, int value);
        bool WriteRegisters(int startRegister, int* values, int valueCount);

        ModbusRTUResponse* ReadInputRegisters(int startRegister, int registerCount);
        ModbusRTUResponse* ReadHoldingRegisters(int startRegister, int registerCount);
};

#endif