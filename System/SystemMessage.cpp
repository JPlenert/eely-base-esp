// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "SystemMessage.h"
#include "Eelybase.h"
#include "ApiHandlerBase.h"

int SystemMessage :: AddMessage(string message)
{
    _systemMessages.insert(pair<int, string>(++_curMsgId, message));
    return _curMsgId;
}

void SystemMessage :: RemoveMessage(int msgId)
{
    _systemMessages.erase(msgId);
}

Json SystemMessage :: GetMessages()
{
    Json reply = ApiHandlerBase::GetOkReply();
    JsonArray array = reply.AddArray("messages");

    for (auto &item : _systemMessages)
    {
        array.AddString(item.second);      
    }

    return reply;
}

SystemMessage* SystemMessage :: GetInstance()
{
    return (SystemMessage*)g_eelybase->GetFeature(g_eelybase->FEATKEY_SYSTEMMESSAGE);
}
