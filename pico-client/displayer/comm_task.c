#include <stdio.h>
#include <string.h>
#include "proto.h"
#include "sched.h"
#include "hal_server.h"
#include "hal_network.h"

#define WAIT_TIME 3000

volatile bool new_data;
uint8_t battery_level;
uint8_t water_level;
uint16_t temp_level;
uint16_t moist_level;

extern packet_t *rx_packet;
extern volatile bool raw_packet_ready;

static uint16_t msg_id_counter = 0;

int
comm_task(void)
{
  if (raw_packet_ready) {
    printf("New data arrived\n");
    raw_packet_ready = false;

    if (rx_packet != NULL && rx_packet->header.cmd_ack == GET_WATERING_CTX_CMD) {
      get_watering_ctx_t ctx;
      hal_network_lock();
      memcpy(&ctx, &rx_packet->data.get_ctx, sizeof(get_watering_ctx_t));
      hal_network_unlock();

      battery_level = ctx.battery_lvl;
      water_level = ctx.water_lvl;
      temp_level = ctx.temp_lvl;
      moist_level = ctx.moisture_lvl;
      new_data = true;
    }
    return WAIT_TIME;
  }

  packet_t tx_packet = {0};
  tx_packet.header.cmd_ack = GET_WATERING_CTX_CMD;
  tx_packet.header.msg_id = ++msg_id_counter;
  tx_packet.header.length = 0;
  
  hal_network_lock();
  if (display_conn != NULL) {
    hal_network_send(display_conn, &tx_packet, HEADER_LEN);
    hal_network_flush(display_conn);
  }
  hal_network_unlock();

  return 500;
}

REGISTER_TASK("Comm task", 500, comm_task, NULL, true);