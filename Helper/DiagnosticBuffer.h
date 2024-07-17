// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef DiagnosticBuffer_h
#define DiagnosticBuffer_h

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string>
#include <list>

using namespace std;

class DiagnosticBuffer
{
    protected:
        list<string> _buffer;
        SemaphoreHandle_t _semaphore;
        int _maxLines;
        int _maxMem;
        int _curMem;

        // Line number of the first line in the buffer
        unsigned int _lineNoOffset;

    public:
        DiagnosticBuffer();
        void AddLine(string line);
        list<string> GetLines(int requestStartLineNo, int& startLineNo);

    protected:
        void RemoveFront();
};

extern DiagnosticBuffer g_DiagnosticBuffer;

#endif