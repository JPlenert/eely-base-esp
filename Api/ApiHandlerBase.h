// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef ApiHandlerBase_h
#define ApiHandlerBase_h

#include <string>
#include "JSON.h"

using namespace std;

class ApiHandlerBase
{
    public:
        virtual Json HandleRequest(Json& request) = 0;
        virtual std::string GetApiId() = 0;
    
        static Json GetOkReply();
        
    protected:
        Json GetErrorReply(int code);
        Json GetErrorReply(int code, string errorMessage);
};

#endif