#pragma once

#include <stdint.h>

#include <curl/curl.h>

#ifndef ENDPOINT
  #define ENDPOINT "localhost"
#endif

#define ENDPOINT_HTTPS "https://"ENDPOINT
#define HEALTH_ENDPOINT ENDPOINT_HTTPS"/health"
#define TELEMETRY_ENDPOINT ENDPOINT_HTTPS"/api/v1/telemetry"
#ifndef REDIS_PORT
  #define REDIS_PORT 31337
#endif

extern CURL *cloud_point;

int init_cloud_conn(void);
void deinit_cloud_conn(void);
int cloud_post_telemetry(const uint64_t pico_id, uint16_t soil_moisture, uint8_t water_lvl,
                     uint8_t battery_lvl, uint32_t uptime, uint16_t temp_lvl);
void *cloud_main(void *ptr);
