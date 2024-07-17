// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "DiagnosticBuffer.h"
#include "esp_log.h"
#include "sys/time.h"
#include "string_format.h"

DiagnosticBuffer g_DiagnosticBuffer;

DiagnosticBuffer :: DiagnosticBuffer()
{
    _semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(_semaphore);
    _maxLines = 500;
    _maxMem = 10*1024;
    _curMem = 0;

    _lineNoOffset = 0;
}

void DiagnosticBuffer :: AddLine(string line)
{
    time_t current_time;
    struct tm timeinfo;

    time(&current_time);
    gmtime_r(&current_time, &timeinfo);

    line = string_format("%04d-%02d-%02d %02d:%02d:%02d;", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday, 
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec) + line;

    if (xSemaphoreTake( _semaphore, ( TickType_t ) 10 ) != pdTRUE)
    {
        ESP_LOGI("DiagnosticBuffer", "Unable to lock");
        return;
    }

    // Check if cleanup of buffer is needed
    while (_buffer.size() + 1 > _maxLines || _curMem + line.size() > _maxMem)
        RemoveFront();

    _buffer.push_back(line);
    _curMem += line.length();

    xSemaphoreGive( _semaphore );
}

void DiagnosticBuffer :: RemoveFront()
{
    _lineNoOffset++;
    _curMem -= _buffer.begin()->size();
    _buffer.pop_front();
}

list<string> DiagnosticBuffer :: GetLines(int requestStartLineNo, int& startLineNo)
{
    if (xSemaphoreTake( _semaphore, ( TickType_t ) 10 ) != pdTRUE)
    {
        ESP_LOGI("DiagnosticBuffer", "Unable to lock");
        return {};
    }

    // Remove not needed lines
    while (requestStartLineNo > _lineNoOffset)
        RemoveFront();

    startLineNo = _lineNoOffset;
    list<string> newList(_buffer);

    xSemaphoreGive( _semaphore );

    return newList;
}