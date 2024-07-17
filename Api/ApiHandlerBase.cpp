// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "ApiHandlerBase.h"

Json ApiHandlerBase :: GetOkReply()
{
    Json reply;

    reply.SetInt("status", 0);
    return reply;
}

Json ApiHandlerBase :: GetErrorReply(int code, string errorMessage)
{
    Json reply;

    reply.SetInt("status", code);
    reply.SetString("errorMessage", errorMessage);

    return reply;
}

Json ApiHandlerBase :: GetErrorReply(int code)
{
    Json reply;

    reply.SetInt("status", code);
    return reply;
}
