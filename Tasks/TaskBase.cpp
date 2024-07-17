// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "TaskBase.h"

void TaskBase_vTaskCode(void* pvParameters)
{
    ((TaskBase*)pvParameters)->TaskCode();
}

TaskBase :: TaskBase(const char* taskName, int stacksize)
{
    _taskHandle = NULL;
    _taskDelay = 100;
    _stackSize = stacksize == 0 ? TASKBASE_DEFAULT_STACKSIZE : stacksize;
    _doStop = false;
    _taskName = taskName == NULL ? "NONAME" : taskName;
}

void TaskBase :: Start()
{
    xTaskCreate(TaskBase_vTaskCode, _taskName, _stackSize, this, tskIDLE_PRIORITY, &_taskHandle);
}

void TaskBase :: Stop()
{
    _doStop = true;
}

void TaskBase :: TaskCode()
{
    PreWork();
    while (!_doStop)
    {
        Work();
        vTaskDelay(_taskDelay);
    }
    PostWork();
    vTaskDelete(_taskHandle);
}
