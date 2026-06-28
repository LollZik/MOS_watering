#include <string.h>

#include <hiredis/hiredis.h>
#include <cjson/cJSON.h>

#include "creators.h"
#include "serv.h"
#include "cloud.h"
#include "logger.h"

typedef struct redis_cmd {
  const char *cmd_string;
  int (*cmd)(cJSON *json, pico_ctx_t *pico_ctx);
} redis_cmd_t;

  int
do_water(cJSON *json, pico_ctx_t *pico_ctx)
{
  cJSON *time_sec = cJSON_GetObjectItem(json, "time_ms");

  log_info("Watering for: %u\n", time_sec->valueint);
  uint32_t time_ms = time_sec->valueint;

  create_water_trigger_packet(0xEF, pico_ctx, time_ms);
  send_packet(pico_ctx);
}

  int
do_reboot(cJSON *json, pico_ctx_t *pico_ctx)
{
  log_info("Rebooting %s\n", pico_ctx->pico_name); 

  create_reset_packet(0xFE, pico_ctx);
  send_packet(pico_ctx);
}

  int
update_watering_time(cJSON *json, pico_ctx_t *pico_ctx)
{
  cJSON *time_sec = cJSON_GetObjectItem(json, "time_ms");

  log_info("Updating watering time: %u\n", time_sec->valueint);
  uint32_t time_ms = time_sec->valueint;

  create_watering_time_packet(0xEF, pico_ctx, time_ms);
  send_packet(pico_ctx);
}

  int
update_watering_threshold(cJSON *json, pico_ctx_t *pico_ctx)
{
  cJSON *threshold = cJSON_GetObjectItem(json, "threshold");

  log_info("Setting threshold at: %u\n", threshold->valueint);
  uint16_t threshold_val = threshold->valueint;

  create_water_threshold_packet(0xEF, pico_ctx, threshold_val);
  send_packet(pico_ctx);
}

redis_cmd_t redis_cmd[] = {
  {"WATER", &do_water},
  {"REBOOT", &do_reboot},
  {"WATERING_TIME", &update_watering_time},
  {"WATERING_THRESHOLD", &update_watering_threshold},
};

  int
parse_cmd(pico_ctx_t *pico_ctx, cJSON *cmd, cJSON *json)
{
  for (int i=0; i<sizeof(redis_cmd)/sizeof(redis_cmd_t); i++)
  {
    if (strcmp(redis_cmd[i].cmd_string, cmd->valuestring) == 0)
    {
      log_info("Got CMD: %s\n", cmd->valuestring);
      redis_cmd[i].cmd(json, pico_ctx);
    }
  }
}

extern redisAsyncContext *redis_ctx;

  void
get_callback(redisAsyncContext *c, void *r, void *privdata)
{
  redisReply *rep = r;
  if (!rep)
  {
    return;
  }

  log_info("argv[%s]: %s\n", (char *)privdata, rep->str);
}

  void
con_callback(const redisAsyncContext *c, int status)
{
  if (status != REDIS_OK)
  {
    log_err("Error connecting to redis: %s\n", c->errstr);
    return;
  }

  log_info("Connected to redis server\n");
}

  void
dcon_callback(const redisAsyncContext *c, int status)
{
  if (status != REDIS_OK)
  {
    log_err("Disconnected from redis error: %s\n", c->errstr);
    return;
  }

  log_info("Disconnected to redis server\n");
}

  void
parse_json(const char *json)
{
  cJSON *device_id;
  cJSON *cmd;
  cJSON *args;

  int status = 0;
  cJSON *data = cJSON_Parse(json);

  if (data == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    status = 0;
    return;
  }

  device_id = cJSON_GetObjectItemCaseSensitive(data, "device_id");
  cmd = cJSON_GetObjectItem(data, "cmd");
  if (cJSON_IsString(device_id) && (device_id->valuestring != NULL))
  {
    log_info("Got device_id \"%s\"\n", device_id->valuestring);
    uint64_t did = strtoull(device_id->valuestring, NULL, 16);

    for (int i=0; i<pico_count; i++)
    {
      if (pico_ctxs[i].pico_id == did)
      {
        parse_cmd(&pico_ctxs[0], cmd, data);
      }
    }
  }
}

void 
subscribe_callback(redisAsyncContext *ac, void *reply, void *privdata) {
  redisReply *r = reply;
  if (r == NULL) {
    log_err("SUBSCRIBE error: %s\n", ac->errstr);
    return;
  }
  if (r->type == REDIS_REPLY_ERROR) {
    log_err("SUBSCRIBE failed: %s\n", r->str);
    return;
  }
  if (r->type == REDIS_REPLY_ARRAY && r->elements >= 3) {
    const char *type = r->element[0]->str;
    if (strcmp(type, "subscribe") == 0) {
      log_info("Subscribed to %s (count %lld)\n", r->element[1]->str, r->element[2]->integer);
    } else if (strcmp(type, "message") == 0) {
      log_info("Message on %s: %s\n", r->element[1]->str, r->element[2]->str);

      parse_json(r->element[2]->str);
    }
  }
}

void auth_callback(redisAsyncContext *ac, void *reply, void *privdata) {
  redisReply *r = reply;
  if (r == NULL) {
    log_err("AUTH error: %s\n", ac->errstr);
    return;
  }
  if (r->type == REDIS_REPLY_ERROR) {
    log_err("AUTH failed: %s\n", r->str);
    return;
  }
  log_info("AUTH successful: %s\n", r->str);

  int status = redisAsyncCommand(redis_ctx, subscribe_callback, NULL, "SUBSCRIBE commands_channel");
  if (status != REDIS_OK) {
    log_err("Could not send AUTH command: %s\n", redis_ctx->errstr);
  }
}
