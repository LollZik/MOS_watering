#include "hal_network.h"
#include "hal_system.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "proto.h"
#include "dispatcher.h"

#define BUF_SIZE MAX_FLASH_DATA

#define POLL_TIME_S 10
#define TCP_PORT 8080

typedef struct TCP_CLIENT_T_ {
  struct tcp_pcb *tcp_pcb;
  ip_addr_t remote_addr;
  uint8_t buffer[BUF_LEN];
  int buffer_len;
  int sent_len;
  bool complete;
  int run_count;
  bool connected;
  struct tcp_pcb **out_tcp_ptr;
} TCP_CLIENT_T;

static err_t
tcp_client_close(void *arg)
{
  TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
  err_t err = ERR_OK;
  if (state->tcp_pcb != NULL) {
    tcp_arg(state->tcp_pcb, NULL);
    tcp_poll(state->tcp_pcb, NULL, 0);
    tcp_sent(state->tcp_pcb, NULL);
    tcp_recv(state->tcp_pcb, NULL);
    tcp_err(state->tcp_pcb, NULL);

    err = tcp_close(state->tcp_pcb);
    if (err != ERR_OK) {
      tcp_abort(state->tcp_pcb);
      err = ERR_ABRT;
    }
    state->tcp_pcb = NULL;
  }
  if (state != NULL) {
      free(state);
  }
  return err;
}

static err_t
tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
  state->sent_len += len;

  return ERR_OK;
}

static err_t
tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;

  if (err != ERR_OK) {
    return -1;
  }

  if (state->out_tcp_ptr != NULL) {
      *(state->out_tcp_ptr) = state->tcp_pcb; 
  }
  state->connected = true;
  return ERR_OK;
}

static err_t
tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
  return 0;
}

static void
tcp_client_err(void *arg, err_t err)
{
  TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
  if (state) {
    state->tcp_pcb = NULL; 
    state->connected = false;
  }
}

err_t
tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  TCP_CLIENT_T *state = (TCP_CLIENT_T*) arg;

  cyw43_arch_lwip_check();

  if (!p) {
    return err;
  }

  if (p->tot_len > 0) {
    const uint16_t buffer_left = BUF_SIZE;
    uint16_t room = (uint16_t)(BUF_LEN - state->buffer_len);
    uint16_t to_copy = p->tot_len > room ? room : p->tot_len;

    state->buffer_len += pbuf_copy_partial(p, state->buffer + state->buffer_len,
        to_copy, 0);
    
    size_t offset = 0;
    while (((size_t)state->buffer_len - offset) >= sizeof(header_t)){
      packet_t *pkt = (packet_t *)(state->buffer + offset);
      const size_t total_needed = sizeof(header_t) + pkt->header.length;

      if (total_needed > BUF_LEN){
        offset = state->buffer_len;
        break;
      }

      if (offset + total_needed > (size_t)state->buffer_len){
        break;
      }
      dispatch(pkt, total_needed, tpcb);

      offset += total_needed;
    }
 
    if (offset > 0) {
      if (offset < state->buffer_len) {
        memmove(state->buffer, state->buffer + offset, state->buffer_len - offset);
      }
      state->buffer_len -= offset;
    }

    tcp_recved(tpcb, p->tot_len);
  }

  if (p){
    pbuf_free(p);
  }

  return ERR_OK;
}

static bool
tcp_client_open(void *arg)
{
  TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
  state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->remote_addr));
  if (!state->tcp_pcb) {
    return false;
  }

  tcp_arg(state->tcp_pcb, state);
  tcp_poll(state->tcp_pcb, tcp_client_poll, POLL_TIME_S * 2);
  tcp_sent(state->tcp_pcb, tcp_client_sent);
  tcp_recv(state->tcp_pcb, tcp_client_recv);
  tcp_err(state->tcp_pcb, tcp_client_err);

  state->buffer_len = 0;

  cyw43_arch_lwip_begin();
  err_t err = tcp_connect(state->tcp_pcb, &state->remote_addr, TCP_PORT, tcp_client_connected);
  cyw43_arch_lwip_end();

  return err == ERR_OK;
}

static TCP_CLIENT_T *
tcp_client_init(const char* ip_addr, struct tcp_pcb **out_ptr)
{
  TCP_CLIENT_T *state = calloc(1, sizeof(TCP_CLIENT_T));
  if (!state) {
    return NULL;
  }

  ip4addr_aton(ip_addr, &state->remote_addr);
  state->out_tcp_ptr = out_ptr;
  return state;
}

bool
hal_network_init_and_connect(const char* ip_addr, hal_net_conn_t *out_ptr)
{
  
  TCP_CLIENT_T *state = tcp_client_init(ip_addr, (struct tcp_pcb **)out_ptr);
  if (!state) {
    return false;
  }

  uint32_t last_try_time = 0;
  while (!state->connected) {
    if (state->tcp_pcb == NULL) {
      uint32_t current_time = hal_system_get_time_us();

      if (current_time - last_try_time > 100000) {
            tcp_client_open(state);
            last_try_time = hal_system_get_time_us();
        }
    }
    hal_system_tight_loop();
  }

  return true;
}

void
hal_network_send(hal_net_conn_t conn, const void *data, uint16_t len)
{
    struct tcp_pcb *tpcb = (struct tcp_pcb *)conn;
  
    tcp_write(tpcb, data, len, TCP_WRITE_FLAG_MORE);
}

uint32_t
hal_network_get_ip_v4(void)
{
    struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];
    return netif_ip4_addr(n)->addr;
}

void
hal_network_lock(void)
{
    cyw43_arch_lwip_begin();
}

void
hal_network_unlock(void)
{
    cyw43_arch_lwip_end();
}

void
hal_network_flush(hal_net_conn_t conn)
{
    struct tcp_pcb *tpcb = (struct tcp_pcb *)conn;
    if (tpcb != NULL) {
        tcp_output(tpcb);
    }
}