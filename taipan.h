#ifndef TAIPAN
#define TAIPAN

#define ANSI_COLOR_RED     "\x1b[31;1m"
#define ANSI_COLOR_YELLOW  "\x1b[33;1m"
#define ANSI_COLOR_BLUE    "\x1b[32;1m"
#define ANSI_COLOR_RESET   "\x1b[0m"

struct params {
  int fd[2];
  char **argv;
  char *host_ip;
  char *container_ip;
  char *container_gw;
  char *container_dns;
  char *container_cmd;
  char *container_rfs;
};

#define STACKSIZE (1024*1024)

void print_container_info();
void config_container_user_namespace(int pid);
void config_container_filesystem();
void config_container_network();
int prepare_container_cmd();
int container();
void config_host_network(int cmd_pid);
void write_file(char path[], char line[]);
void do_system_command(char *fmt, ...);
int get_user_defined_options(int argc, char **argv);
int main(int argc, char **argv);

#endif
