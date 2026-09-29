#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "wifi_sta.h"

// Settings
static const uint64_t connection_timeout_ms = 10000;
static const uint32_t sleep_time_ms = 1000;

// TAG for debug
static const char *TAG = "wifi_demo";

// App entrypoint
void app_main(void)
{
    esp_err_t esp_ret;
    EventGroupHandle_t network_event_group;
    EventBits_t network_event_bits;

    network_event_group = xEventGroupCreate();
    if (network_event_group == NULL) {
        ESP_LOGE(TAG, "Error: Could not initialize event group");
        abort();
    }

    // Initialize NVS: ESP32 WiFi driver uses NVS to store Wifi settings
    esp_ret = nvs_flash_init();
    if ((esp_ret == ESP_ERR_NVS_NO_FREE_PAGES) ||
        (esp_ret == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        esp_ret = nvs_flash_init();
    }
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%d): Failed to initialize NVS", esp_ret);
        abort();
    }

    // Initialize TCP/IP network interface (only call once in application)
    esp_ret = esp_netif_init();
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%d): Failed to initialize network interface", esp_ret);
        abort();
    }

    // Create default event loop that runs in the background
    esp_ret = esp_event_loop_create_default();
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%d): Failed to create default event loop", esp_ret);
        abort();
    }

    // Initialize network connection
    esp_ret = wifi_sta_init(network_event_group);
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Error (%d): Failed to initialize WiFi", esp_ret);
        abort();
    }

    // Wait for network to connect
    ESP_LOGI(TAG, "Waiting for network to connect...");
    network_event_bits = xEventGroupWaitBits(network_event_group,
                                             WIFI_STA_CONNECTED_BIT,
                                             pdFALSE,
                                             pdTRUE,
                                             pdMS_TO_TICKS(connection_timeout_ms));
    if (network_event_bits & WIFI_STA_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi network");
    } else {
        ESP_LOGE(TAG, "Failed to connect to network");
        abort();
    }

    // Wait for IP address
    ESP_LOGI(TAG, "Waiting for IP address...");
    network_event_bits = xEventGroupWaitBits(network_event_group,
                                             WIFI_STA_IPV4_OBTAINED_BIT |
                                             WIFI_STA_IPV6_OBTAINED_BIT,
                                             pdFALSE,
                                             pdFALSE,
                                             pdMS_TO_TICKS(connection_timeout_ms));
    if (network_event_bits & WIFI_STA_IPV4_OBTAINED_BIT) {
        ESP_LOGI(TAG, "Obtained IPv4 address");
    } else if (network_event_bits & WIFI_STA_IPV6_OBTAINED_BIT) {
        ESP_LOGI(TAG, "Obtained IPv6 address");
    } else {
        ESP_LOGE(TAG, "Failed to obtain IP address");
    }

    // Superloop
    while (1) {
        // Read live event group bits on each iteration
        network_event_bits = xEventGroupGetBits(network_event_group);

        // Make sure we are still connected
        if (network_event_bits & WIFI_STA_CONNECTED_BIT) {
            ESP_LOGI(TAG, "Still connected to WiFi network");
        } else {
            ESP_LOGE(TAG, "Lost connection to the network");
            abort();
        }

        // Delay
        vTaskDelay(pdMS_TO_TICKS(sleep_time_ms));
    }
}