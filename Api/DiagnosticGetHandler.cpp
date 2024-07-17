// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "DiagnosticGetHandler.h"
#include "DiagnosticBuffer.h"
#include "esp_log.h"
#include <list>
#include <string>

using namespace std;

Json DiagnosticGetHandler :: HandleRequest(Json& request)
{
    if (request.HasNode("startLineNo"))
        m_lastLineNo = request.GetInt("startLineNo");;

    int effStartLineNo;
    list<string> lines = g_DiagnosticBuffer.GetLines(m_lastLineNo, effStartLineNo);
    m_lastLineNo = effStartLineNo + lines.size();

    Json reply = GetOkReply();
    JsonArray lineArray = reply.AddArray("lines");

    reply.SetInt("startLineNo", effStartLineNo);
    for(auto it = lines.begin(); it != lines.end(); it++)
        lineArray.AddString(it->c_str());

    return reply;
}

string DiagnosticGetHandler :: GetApiId() { return "DiagnosticGet"; }
