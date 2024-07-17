// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef SystemMessage_h
#define SystemMessage_h

#include <string>
#include <map>
#include <functional>
#include "Json.h"

using namespace std;

class SystemMessage
{
    protected:
        std::map<int, string> _systemMessages;
        int _curMsgId;

    public:
        int AddMessage(string message);
        void RemoveMessage(int msgId);

        Json GetMessages();

        static SystemMessage* GetInstance();
};

#endif