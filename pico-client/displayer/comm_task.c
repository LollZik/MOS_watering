#include "proto.h"
#include "tcp_server.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"

#include "sched.h"

#define WAIT_TIME 3000

volatile bool new_data;
uint8_t battery_level;
uint8_t water_level;
uint16_t temp_level;
uint16_t moist_level;

extern struct tcp_pcb *display_tcp;

extern packet_t rx_packet;
extern volatile bool raw_packet_ready;

static uint16_t msg_id_counter = 0;

int comm_task(void) {
    if (raw_packet_ready) {
        printf("New data arrived\n");
        raw_packet_ready = false;

        if (rx_packet.header.cmd_ack == GET_WATERING_CTX_CMD) {
            
            get_watering_ctx_t ctx;
            cyw43_arch_lwip_begin();
            memcpy(&ctx, &rx_packet.data.get_ctx, sizeof(get_watering_ctx_t));
            cyw43_arch_lwip_end();
    

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

    cyw43_arch_lwip_begin();

    if (display_tcp != NULL) {
        tcp_write(display_tcp, &tx_packet, HEADER_LEN, TCP_WRITE_FLAG_COPY);
        tcp_output(display_tcp);
    }
    
    cyw43_arch_lwip_end();

    return 500;
}

REGISTER_TASK("Comm task", 500, comm_task, NULL, true);