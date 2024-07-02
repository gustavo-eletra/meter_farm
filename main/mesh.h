#pragma once
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_event.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_mesh.h"
#include "esp_mesh_internal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/sys.h"

#define RX_WIFI_SIZE 526
#define TX_WIFI_SIZE 526

static const char *MESH_TAG = "MESH";
static const char *WIFI_TAG = "WIFI";

static bool is_mesh_connected = false;
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;
static esp_netif_t *netif_sta = NULL;