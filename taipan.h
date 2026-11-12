#ifndef TAIPAN_H
#define TAIPAN_H

#include <stdbool.h>
#include <sys/types.h>

#define ANSI_COLOR_RED    "\x1b[31;1m"
#define ANSI_COLOR_YELLOW "\x1b[33;1m"
#define ANSI_COLOR_GREEN  "\x1b[32;1m"
#define ANSI_COLOR_WHITE  "\x1b[37;1m"
#define ANSI_COLOR_RESET  "\x1b[0m"

#define STACK_SIZE (1024U * 1024U)
#define MAX_COMMAND_LENGTH 256U
#define MAX_COMMAND_ARGUMENTS 32U

struct params {
  int fd[2];
  char *argv[MAX_COMMAND_ARGUMENTS];
  char command_buffer[MAX_COMMAND_LENGTH];
  const char *host_ip;
  const char *container_ip;
  const char *container_gw;
  const char *container_dns;
  const char *container_cmd;
  const char *container_rfs;
  uid_t host_uid;
  gid_t host_gid;
  int original_ip_forward;
  bool ip_forward_changed;
  bool veth_created;
  bool nat_rule_added;
};

void print_container_info(void);
int config_container_user_namespace(pid_t pid);
int config_container_filesystem(void);
int config_container_network(void);
int prepare_container_cmd(void);
int container(void *unused);
int config_host_network(pid_t container_pid);
int write_file(const char *path, const char *line);
int run_command(const char *fmt, ...);
int get_user_defined_options(int argc, char **argv);
void cleanup_host_network(void);
void print_usage(const char *program_name);

#endif
