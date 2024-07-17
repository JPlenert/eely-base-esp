// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
#include "OtaUploadHandler.h"
#include "esp_log.h"
#include "esp_app_format.h"

static const char *TAG = "OtaUploadHandler";

Json OtaUploadHandler :: HandleRequest(Json& request)
{
    int chunkNo = request.GetInt("chunkNo");
    ESP_LOGI(TAG, "Got chunk %u", chunkNo);

    size_t chunkDataLen;
    unsigned char *chunkData = request.GetBinary("bin", &chunkDataLen);    
    if (chunkNo == 0)
    {
        // If previous update was not finished
        if (_updateHandle != 0)
        {
             esp_ota_abort(_updateHandle);
             _updateHandle = 0;
        }

        _lastChunkNo = -1;
        _curWritePos = 0;
        _uploadLen = request.GetInt("uploadLen");

        Json jsonReply = ProcessChunkZero(chunkData, chunkDataLen);
        if (chunkData != NULL)
            free(chunkData);

        return jsonReply;
    }
    else
    {
        if (_curWritePos + chunkDataLen > _uploadLen)
        {
            free(chunkData);
            return GetErrorReply(-1, "Invlid upload len");
        }
        else
        {
            Json jsonReply = WriteChunk(chunkNo, chunkData, chunkDataLen);
            if (chunkData != NULL)
                free(chunkData);

            if (_curWritePos == _uploadLen)
                return FinalizeUpdate();
            {

            }
        }
    }

    return GetOkReply();
}

string OtaUploadHandler :: GetApiId() { return "OtaUpload"; }

Json OtaUploadHandler :: ProcessChunkZero(unsigned char *chunkData, int chunkDataLen)
{
    esp_err_t err;

    if (chunkDataLen < sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))
    {
        return GetErrorReply(-1, "Invalid bin size at first packet");
    }

    // Get and check headers
    esp_image_header_t* header = (esp_image_header_t*)chunkData;
    //esp_image_segment_header_t* segment = (esp_image_segment_header_t*)&chunkData[sizeof(esp_image_header_t)];
    esp_app_desc_t* appdesc = (esp_app_desc_t*)&chunkData[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)];

    if (header->magic != ESP_IMAGE_HEADER_MAGIC || appdesc->magic_word != ESP_APP_DESC_MAGIC_WORD)
    {
        return GetErrorReply(-1, "Invalid header magic bytes");
    }

    ESP_LOGI(TAG, "New firmware version: %s", appdesc->version);

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

    // Prepare update
    _updatePartition = esp_ota_get_next_update_partition(NULL);
    if (_updatePartition == NULL)
    {
        return GetErrorReply(-1, "No update partition found");
    }
    else
        ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%lx", _updatePartition->subtype, _updatePartition->address);

    err = esp_ota_begin(_updatePartition, OTA_WITH_SEQUENTIAL_WRITES, &_updateHandle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        return GetErrorReply(-1, "Unable to begin OTA update");
    }

    return WriteChunk(0, chunkData, chunkDataLen);
}

Json OtaUploadHandler :: WriteChunk(int chunkNo, unsigned char *chunkData, int chunkDataLen)
{
    esp_err_t err;

    if (chunkNo != _lastChunkNo + 1)
    {
        ESP_LOGE(TAG, "Received unexpected chunkNo %d (expected %d)", chunkNo, _lastChunkNo + 1);
        return GetErrorReply(-1, "Received unexpected chunkNo");
    }

    ESP_LOGI(TAG, "Writing chunk %d with %d bytes to %d", chunkNo, chunkDataLen, _curWritePos);
    err = esp_ota_write(_updateHandle, (const void *)chunkData, chunkDataLen);
    if (err != ESP_OK) 
    {
        esp_ota_abort(_updateHandle);
        _updateHandle = 0;
        _lastChunkNo = -1;
        return GetErrorReply(-1, "Error writing update chunk");
    }
    _lastChunkNo++;
    _curWritePos += chunkDataLen;

    return GetOkReply();
}

Json OtaUploadHandler :: FinalizeUpdate()
{
    esp_err_t err;

    err = esp_ota_end(_updateHandle);
    if (err != ESP_OK) 
    {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) 
        {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            return GetErrorReply(-1, "Image is corrupted");
        } 
        else 
        {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
            return GetErrorReply(-1, "Image write failed");
        }
    }
    else
        ESP_LOGI(TAG, "esp_ota_end finished");

    err = esp_ota_set_boot_partition(_updatePartition);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return GetErrorReply(-1, "Image activate failed");
    }

    return GetOkReply();
}