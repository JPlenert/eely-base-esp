// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#ifndef OtaUploadHandler_h
#define OtaUploadHandler_h

#include <cJSON.h>
#include <string>
#include "ApiHandlerBase.h"
#include "esp_ota_ops.h"


class OtaUploadHandler : public ApiHandlerBase
{
    protected: 
        int _curWritePos;
        int _lastChunkNo;
        int _uploadLen;
        esp_ota_handle_t _updateHandle;
        const esp_partition_t* _updatePartition;

    public:
        Json HandleRequest(Json& request);
        string GetApiId();

    protected:
        Json ProcessChunkZero(unsigned char *chunkData, int chunkDataLen);
        Json WriteChunk(int chunkNo, unsigned char *chunkData, int chunkDataLen);
        Json FinalizeUpdate();
};

#endif
