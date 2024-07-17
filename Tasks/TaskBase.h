// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef TaskBase_h
#define TaskBase_h

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TASKBASE_DEFAULT_STACKSIZE 3*1024

class TaskBase
{   
    protected:
        int _taskDelay;
        TaskHandle_t _taskHandle;
        bool _doStop;
        int _stackSize;
        const char* _taskName;

    public:
        TaskBase(const char* taskname = NULL, int stackSize = 0);
        void Start();
        void Stop();
        void TaskCode();

    protected:
        virtual void PreWork() {}
        virtual void Work() = 0;
        virtual void PostWork() {}
};

#endif