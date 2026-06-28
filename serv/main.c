#include <stdio.h>
#include <poll.h>
#include <argp.h>
#include <getopt.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "recver.h"
#include "serv.h"
#include "logger.h"
#include "controller.h"
#include "cloud.h"

#define dbg_printf printf

struct argp_option options[] = {
    { "verbose", 'v', "LEVEL", 0, "Set verbosity level (0=errors only, 1=warnings, 2=info, 3=debug, 4=trace)" },
    { "help", 'h', 0, 0, "Display this help message" },
    {0}
};


const char *argp_program_version = "watering server v0.0.0";
const char *argp_program_bug_addr = "koruddl@gmail.com";
static char args_doc[] = "[OPTIONS]...";
char doc[] = "Server for embedded watering service";

error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = (struct arguments *)state->input;
  int val;

  switch (key) {
    case 'v':
      val = atoi(arg);
      if (set_log_lvl(val) != 0) {
        fprintf(stderr, "Error: Verbosity level must be between 0 and 4\n");
        return ARGP_ERR_UNKNOWN;
      }
      break;

    case 'h':
      argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

struct argp argp = { options, parse_opt, args_doc, doc };

  int
main(int argc, char **argv)
{
  argp_parse(&argp, argc, argv, 0, 0, NULL);

  log_info("Welcome in watering server\n");
  log_info("Version:\t%u.%u.%u\n",
      PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);
  log_info("Commit:\t\t%s\t%s\n", GIT_COMMIT_HASH, GIT_DIRTY);

  pthread_t recv_thread;
  pthread_t serv_thread;
  pthread_t control_thread;
  pthread_t cloud_thread;

  pthread_create(&serv_thread, NULL, &serv_main, NULL);
  pthread_create(&recv_thread, NULL, &recv_main, NULL);
  pthread_create(&control_thread, NULL, &main_controller, NULL);
  pthread_create(&cloud_thread, NULL, &cloud_main, NULL);

  pthread_join(recv_thread, NULL);
  pthread_join(serv_thread, NULL);
  pthread_join(control_thread, NULL);
  pthread_join(cloud_thread, NULL);
}
