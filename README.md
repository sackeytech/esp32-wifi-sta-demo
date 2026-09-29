# ESP32 Wi-Fi Station (`wifi_sta`) Component & Demo — ESP-IDF

A modular Wi-Fi Station implementation for the ESP32 built with the **ESP-IDF framework** inside a Dockerized **VS Code DevContainer**. This project separates low-level Wi-Fi driver and network interface initialization into a reusable `wifi_sta` component and synchronizes network states with the main application using **FreeRTOS Event Groups**.

## Project Structure

```text
├── .devcontainer/
│   └── devcontainer.json          # Docker DevContainer with USB passthrough (/dev:/dev)
├── Dockerfile.esp-idf             # ESP-IDF toolchain container image definition
└── workspace/
    ├── app/
    │   └── wifi_demo/             # Main application consuming the wifi_sta component
    │       ├── CMakeLists.txt
    │       └── main/
    │           ├── CMakeLists.txt
    │           └── main.c
    └── components/
        └── wifi_sta/              # Reusable Wi-Fi Station component
            ├── CMakeLists.txt
            ├── Kconfig            # Custom menuconfig options (SSID, Password, Auth, IP)
            ├── wifi_sta.c
            └── include/
                └── wifi_sta.h
```

## How the ESP-IDF Wi-Fi Architecture Works

1. **NVS Flash (`nvs_flash_init`)**: Initializes Non-Volatile Storage required by the ESP32 RF calibration and Wi-Fi driver.
2. **TCP/IP Stack (`esp_netif_init`)**: Initializes the LwIP TCP/IP network stack.
3. **System Event Loop (`esp_event_loop_create_default`)**: Starts the background FreeRTOS task that dispatches `WIFI_EVENT` and `IP_EVENT` callbacks.
4. **Netif & Driver Binding (`wifi_sta_init`)**: Binds the Wi-Fi station interface, loads `WIFI_INIT_CONFIG_DEFAULT()`, registers event handlers, and starts the driver via `esp_wifi_start()`.
5. **FreeRTOS `EventGroup` Synchronization**:
   * `WIFI_EVENT_STA_START` -> Calls `esp_wifi_connect()`.
   * `WIFI_EVENT_STA_CONNECTED` -> Sets `WIFI_STA_CONNECTED_BIT`.
   * `IP_EVENT_STA_GOT_IP` -> Sets `WIFI_STA_IPV4_OBTAINED_BIT`.
   * `WIFI_EVENT_STA_DISCONNECTED` -> Clears both bits and triggers `esp_wifi_connect()` for automatic recovery.

## Build, Flash, and Monitor

```bash
cd /workspace/workspace/app/wifi_demo
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 -b 115200 flash monitor
```
