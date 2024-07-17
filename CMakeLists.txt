idf_component_register(SRCS

"Eelybase.cpp"

"Api/ApiHandlerBase.cpp" "Api/ApiManager.cpp" "Api/DiagnosticGetHandler.cpp"
"Api/WifiSettingsSetHandler.cpp" "Api/OtaUploadHandler.cpp" "Api/RestartHandler.cpp"
"Api/DeviceInfoGetHandler.cpp" "Api/ConfigSetHandler.cpp" "Api/ConfigGetHandler.cpp"

"Helper/DiagnosticBuffer.cpp" "Helper/Json.cpp" "Helper/JsonArray.cpp"  "Helper/base64.c"

"Interfaces/I2c.cpp" "Interfaces/ModbusRTUSimple.cpp"

"Peripherals/Hmc5883l.cpp"

"System/wifiAp.cpp" "System/wifiName.cpp" "System/wifiSta.cpp" "System/wifiStart.cpp" 

"System/ConfigStore.cpp"  "System/spiffs.c" "System/factory_reset.c" "System/SystemTime.cpp" "System/SystemMessage.cpp"

"Tasks/TaskBase.cpp"

"WebServer/WebServer.cpp"

"DnsServer/DnsServer.cpp"

"HttpClient/default_http_client_event_handler.c" "HttpClient/HttpClient.cpp"

REQUIRES esp_wifi driver efuse esp_netif esp_timer esp_http_server json nvs_flash spiffs app_update esp_http_client esp-tls 
INCLUDE_DIRS "." "Api" "Helper" "Interfaces" "Peripherals" "System" "Tasks" "WebServer" "HttpClient")
