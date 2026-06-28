#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include <hiredis/hiredis.h>
#include <hiredis/hiredis_ssl.h>
#include <hiredis/async.h>
#include <hiredis/adapters/poll.h>

#include "redis.c"
#include "cloud.h"
#include "logger.h"

#ifndef API_KEY
  #error "API_KEY not defined"
#endif

CURL *cloud_point;
redisAsyncContext *redis_ctx;
static redisSSLContext *ssl_context;

struct resp_data {
    char *memory;
    size_t size;
};

struct resp_data data_resp;

static size_t get_resp_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct resp_data *mem = (struct resp_data *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    // Do NOT reset size here – accumulate the whole response
    // Optional: log each chunk (be careful with binary/UTF-8 cut)
    // log_info("Partial response: %.*s\n", (int)realsize, (char*)contents);

    return realsize;
}

static int
init_curl_conn(void)
{
    log_info("Initialising connection with %s endpoint\n", ENDPOINT_HTTPS);
    CURLcode result;

    // Free any previous buffer and reset for a clean request
    free(data_resp.memory);
    data_resp.memory = NULL;
    data_resp.size = 0;

    result = curl_global_init(CURL_GLOBAL_ALL);
    if (result != CURLE_OK) {
        log_err("curl_global_init returned %d\n", result);
        return (int)result;
    }

    cloud_point = curl_easy_init();
    if (!cloud_point) {
        log_err("curl_easy_init failed\n");
        return -1;
    }

    curl_easy_setopt(cloud_point, CURLOPT_URL, HEALTH_ENDPOINT);
    curl_easy_setopt(cloud_point, CURLOPT_HTTPGET, 1L);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(cloud_point, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(cloud_point, CURLOPT_WRITEFUNCTION, get_resp_cb);
    curl_easy_setopt(cloud_point, CURLOPT_WRITEDATA, (void *)&data_resp);

    result = curl_easy_perform(cloud_point);

    // After the request, log the full accumulated response (if any)
    if (result == CURLE_OK && data_resp.memory && data_resp.size > 0) {
        log_info("Complete response:\n%s\n", data_resp.memory);
    } else if (result != CURLE_OK) {
        log_err("curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
    }

    // Clean up headers list
    if (headers)
        curl_slist_free_all(headers);

    // Free the response data now that we have logged it
    free(data_resp.memory);
    data_resp.memory = NULL;
    data_resp.size = 0;

    return (int)result;
}

static int
init_redis_conn(void)
{
    signal(SIGPIPE, SIG_IGN);

    redisSSLContextError ssl_error = REDIS_SSL_CTX_NONE;

    redisInitOpenSSL();

    log_info("CA Cert path: " WATERING_SSL_CERT_PATH "\n");
    ssl_context = redisCreateSSLContext(
        WATERING_SSL_CERT_PATH,
        NULL,
        NULL,
        NULL,
        ENDPOINT,
        &ssl_error);

    if(ssl_context == NULL || ssl_error != REDIS_SSL_CTX_NONE)
    {
        log_err("SSL context error: %s\n",
            (ssl_error != REDIS_SSL_CTX_NONE) ?
                redisSSLContextGetError(ssl_error) : "Unknown error");
        return -1;
    }

    log_info("Connecting to %s via %u port\n", ENDPOINT, REDIS_PORT);
    redis_ctx = redisAsyncConnect(ENDPOINT, REDIS_PORT);

    if (redis_ctx == NULL || redis_ctx->err)
    {
        if (redis_ctx)
        {
            log_err("Connection error: %s\n", redis_ctx->errstr);
            redisAsyncFree(redis_ctx);
        } else {
            log_err("Connection error: can't allocate redis context\n");
        }
        return -1;
    }

    if (redisInitiateSSLWithContext(&redis_ctx->c, ssl_context) != REDIS_OK)
    {
        log_err("InitiateSSL error: %s\n", redis_ctx->errstr);
        redisAsyncFree(redis_ctx);
        return -1;
    }

    redisPollAttach(redis_ctx);
    redisAsyncSetConnectCallback(redis_ctx, con_callback);
    redisAsyncSetDisconnectCallback(redis_ctx, dcon_callback);

    int status = redisAsyncCommand(redis_ctx, auth_callback, NULL, "AUTH %s", REDIS_PASS);
    if (status != REDIS_OK) {
        log_err("Could not send AUTH command: %s\n", redis_ctx->errstr);
        return -1;
    }

    return 0;
}

int
init_cloud_conn(void)
{
    if (init_curl_conn())
    {
        log_err("Curl init failed\n");
        return -1;
    }

    if (init_redis_conn())
    {
        log_err("Redis init failed\n");
        return -1;
    }

    log_info("Cloud connection established\n");
    return 0;
}

static void
deinit_curl_conn(void)
{
    curl_easy_cleanup(cloud_point);
    curl_global_cleanup();
    // Free any remaining response buffer
    free(data_resp.memory);
    data_resp.memory = NULL;
    data_resp.size = 0;
}

static void
deinit_redis_conn(void)
{
    if (redis_ctx) {
        redisAsyncFree(redis_ctx);
        redis_ctx = NULL;
    }
    if (ssl_context) {
        redisFreeSSLContext(ssl_context);
        ssl_context = NULL;
    }
}

void
deinit_cloud_conn(void)
{
    deinit_curl_conn();
    deinit_redis_conn();
}

// TODO: make more generic function, based probably on some defines, for now it's fine
int
cloud_post_telemetry(const uint64_t pico_id, uint16_t soil_moisture, uint8_t water_lvl,
                     uint8_t battery_lvl, uint32_t uptime, uint16_t temperature)
{
    CURLcode result = CURLE_OK;

    char json_data[256];
    snprintf(json_data, sizeof(json_data),
        "{"
        "\"device_id\": \"%08X%08X\","
        "\"moisture_lvl\": %u,"
        "\"battery_lvl\": %u,"
        "\"water_lvl\": %u,"
        "\"uptime\": %u"
        "}", pico_id >> 32, pico_id, soil_moisture, battery_lvl, water_lvl, uptime);

    log_info("Posting: %s\n", json_data);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    char api_header[256];
    snprintf(api_header, sizeof(api_header), "X-Api-Key: %s", API_KEY);
    headers = curl_slist_append(headers, api_header);

    curl_easy_setopt(cloud_point, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(cloud_point, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(cloud_point, CURLOPT_URL, TELEMETRY_ENDPOINT);
    curl_easy_setopt(cloud_point, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(cloud_point, CURLOPT_POSTFIELDS, json_data);

    result = curl_easy_perform(cloud_point);
    if (result != CURLE_OK)
        log_err("Telemetry POST failed: %s\n", curl_easy_strerror(result));

    curl_slist_free_all(headers);
    return result;
}

void *
cloud_main(void *ptr)
{
    // Loop until connection initialised successfully
    while (1)
    {
        if (init_cloud_conn() == 0)
            break;
        usleep(10000);
    }

    // Main event loop (poll Redis)
    while (1)
    {
        redisPollTick(redis_ctx, 0.5);
    }

    deinit_cloud_conn();
    return NULL;
}
