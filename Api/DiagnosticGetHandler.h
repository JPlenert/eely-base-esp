// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef DiagnosticGetHandler_h
#define DiagnosticGetHandler_h

#include "ApiHandlerBase.h"
#include "cJSON.h"

class DiagnosticGetHandler : public ApiHandlerBase
{
    protected:
        int m_lastLineNo;

    public:
        DiagnosticGetHandler() : m_lastLineNo(0) {}

        Json HandleRequest(Json& request);
        string GetApiId();
};

#endif





