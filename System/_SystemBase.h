// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef SystemBase_h
#define SystemBase_h

#include "ConfigStore.h"
#include <functional>

extern "C"
{
    // factory_reset.h
    bool factory_reset_check();
    // spiffs.c
    void spiffs_init(bool doFormat);
}

#include "wifi.h"

#endif