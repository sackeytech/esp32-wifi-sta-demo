set(srcs)
set(include_dirs "include")

if(CONFIG_WIFI_STA_CONNECT)
    list(APPEND srcs
         "wifi_sta.c")
endif()

idf_component_register(
    SRCS "wifi_sta.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_wifi esp_netif esp_event nvs_flash
)
