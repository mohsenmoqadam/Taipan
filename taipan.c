#define _GNU_SOURCE

#include <stdio.h>
#include <getopt.h>
#include <sched.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <wait.h>
#include <memory.h>
#include <syscall.h>

#include "taipan.h"

char cmd_stack[STACKSIZE];
struct params params;

void print_container_info(void)
{
  printf(ANSI_COLOR_YELLOW"\n");
  printf("        ---_ ......._-_--.\n");
  printf("     ("ANSI_COLOR_RED"|"ANSI_COLOR_YELLOW"\\ /      / /"ANSI_COLOR_RED"|"ANSI_COLOR_YELLOW" \\  \\\n");
  printf("     /  /     .'  -=-'   `.\n");
  printf("    /  /    .'             )\n");
  printf("  _/  /   .'        _.)   /\n");
  printf(" / o   o        _.-' /  .'\n");
  printf(" \\          _.-'    / .'*|\n");
  printf("  \\______.-'//    .'.' \\*|\n");
  printf("   \\|  \\ | //   .'.' _ |*|\n");
  printf("    `   \\|//  .'.'_ _ _|*|\n");
  printf("     .  .// .'.' | _ _ \\*|\n");
  printf("     \\`-|\\_/ /    \\ _ _ \\*\n");
  printf("      `/'\\__\\/      \\ _ _ \\*\n");
  printf("     /^|            \\ _ _ \\*\n");
  printf("    '  `             \\ _ _ \\\n");
  printf("                      \\_ TAIPAN 1.0.0\n");
  printf(ANSI_COLOR_RESET"\n");
  printf(ANSI_COLOR_BLUE" Container-IP:"ANSI_COLOR_RESET"      %s\n", params.container_ip);
  printf(ANSI_COLOR_BLUE" Container-Gateway:"ANSI_COLOR_RESET" %s\n", params.container_gw);
  printf(ANSI_COLOR_BLUE" Container-DNS:"ANSI_COLOR_RESET"     %s\n", params.container_dns);
  printf(ANSI_COLOR_BLUE" Container-CMD:"ANSI_COLOR_RESET"     %s\n", params.container_cmd);
  printf(ANSI_COLOR_BLUE" Container-RFS:"ANSI_COLOR_RESET"     %s\n", params.container_rfs);
  printf("\n");
}

void config_container_user_namespace(int pid)
{
  /* @Reference:
     Link: http://man7.org/linux/man-pages/man7/user_namespaces.7.html
     Section: `Defining user and group ID mappings: writing to uid_map and gid_map`
     Last part: In the case of gid_map, use of the setgroups(2) system call must first 
	        be denied by writing "deny" to the /proc/[pid]/setgroups file (see below) 
	        before writing to gid_map. 
  */
  
  char path[40];
  char line[40];

  uid_t uid = 1000;
  gid_t gid = 1000;

  sprintf(path, "/proc/%d/uid_map", pid);
  sprintf(line, "0 %d 1\n", uid);
  write_file(path, line);

  sprintf(path, "/proc/%d/setgroups", pid);
  sprintf(line, "deny");
  write_file(path, line);

  sprintf(path, "/proc/%d/gid_map", pid);
  sprintf(line, "0 %d 1\n", gid);
  write_file(path, line);
}

void config_container_filesystem()
{
  const char *old_fs = ".old_fs";

  // Config ROOTFS
  mount(params.container_rfs, params.container_rfs, "ext4", MS_BIND, "");
  chdir(params.container_rfs);
  mkdir(old_fs, 0777);
  syscall(SYS_pivot_root, ".", old_fs);
  chdir("/");
  
  // Config PROCFS
  mkdir("/proc", 0555);
  mount("proc", "/proc", "proc", 0, "");

  umount2(old_fs, MNT_DETACH);
}

void config_container_network()
{
  do_system_command("ip link set lo up");
  do_system_command("ip link set eth0 up");
  do_system_command("ip addr add %s dev eth0", params.container_ip);
  do_system_command("route add default gw %s eth0", params.container_gw);
  do_system_command("echo nameserver %s > /etc/resolv.conf", params.container_dns);
}

int prepare_container_cmd ()
{
  char *token;
  int index = 0;
  char container_cmd[256];

  memset(container_cmd, 0, sizeof(container_cmd));
  memcpy(container_cmd, params.container_cmd, strlen(params.container_cmd));

  token = strtok (container_cmd, " ");
  params.argv[index] = token;
  while (token != NULL){
    token = strtok (NULL, " ");
    params.argv[++index] = token;
  }
  params.argv[++index] = NULL;
  
  return 0;
}

int container()
{
  char buf[5];
  
  read(params.fd[0], buf, 5);

  config_container_filesystem();
  if (setgid(0) == -1) {
     printf("Failure: drop superuser privileges.\n");
     exit(EXIT_FAILURE);
  }
  if (setuid(0) == -1) {
    printf("Failure: drop superuser privileges.\n");
    exit(EXIT_FAILURE);
  }

  config_container_network();

  print_container_info();
  
  if (execvp(params.argv[0], params.argv) == -1) {
    printf("Failure: execute command: %s\n", params.container_cmd);
    exit(EXIT_FAILURE);
  }

  return 1;
}

void config_host_network(int cmd_pid)
{
  do_system_command("ip link add cable type veth peer name eth0");
  do_system_command("ip link set eth0 netns %d", cmd_pid);
  do_system_command("ip link set cable up");
  do_system_command("ip addr add %s dev cable", params.host_ip);
  do_system_command("echo 1 > /proc/sys/net/ipv4/ip_forward");
  do_system_command("iptables -t nat -A POSTROUTING -j MASQUERADE");
}

void write_file(char path[40], char line[40])
{
    FILE *f = fopen(path, "w");
    fwrite(line, 1, strlen(line), f);
    fclose(f);
}

void do_system_command(char *fmt, ...)
{
  va_list args;
  char *cmd;
  va_start(args, fmt);
  vasprintf(&cmd, fmt, args);
  va_end(args);
  system(cmd);
}

int get_user_defined_options(int argc, char **argv)
{
  int opt;
  static const struct option longopts[] =
    {{.name = "main_ip", .has_arg = no_argument, .val = 'm'},
     {.name = "container_ip", .has_arg = no_argument, .val = 'i'},
     {.name = "container_gw", .has_arg = no_argument, .val = 'g'},
     {.name = "container_dns", .has_arg = no_argument, .val = 'd'},
     {.name = "container_cmd", .has_arg = no_argument, .val = 'c'},
     {.name = "container_rfs", .has_arg = no_argument, .val = 'r'},
     {}
    };
  
  params.argv = argv;
  while ((opt = getopt_long(argc, argv, "igd", longopts, NULL)) != -1) {
    switch (opt) {
    case 'm':
      params.host_ip = argv[optind];
      break;
    case 'i':
      params.container_ip = argv[optind];
      break;
    case 'g':
      params.container_gw = argv[optind];
      break;
    case 'd':
      params.container_dns = argv[optind];
      break;
    case 'c':
      params.container_cmd = argv[optind];
      break;
    case 'r':
      params.container_rfs = argv[optind];
      break;
    default:
      // @TODO: write manual
      return -1;
    }
  }

  prepare_container_cmd();

  return 0;
}

int main(int argc, char **argv)
{
    // Initilize Default Values.
    memset(&params, 0, sizeof(struct params));
    params.host_ip = "192.168.1.1/30";
    params.container_ip = "192.168.1.2/30";
    params.container_gw = "192.168.1.1";
    params.container_dns = "4.2.2.4";
    params.container_cmd = "sh";
    params.container_rfs = "./taipan_rfs";

    // Get User Defined Options.
    if (get_user_defined_options(argc, argv) == -1)
      exit(EXIT_FAILURE);

    // Create pipe to communicate between main and container process.
    pipe(params.fd);
    
    // Clone container process.
    int clone_flags =
      SIGCHLD       |
      CLONE_NEWUTS  |
      CLONE_NEWUSER |
      CLONE_NEWNS   |
      CLONE_NEWNET  |
      CLONE_NEWPID;
    int cmdPid = clone(container,
		       cmd_stack + STACKSIZE,
		       clone_flags,
		       NULL);
    if(cmdPid == -1) {
      printf("Clone Error!\n");
      exit(EXIT_FAILURE);
    }
    
    // Prepare Pipe
    close(params.fd[0]);
    int pipe = params.fd[1];
    
    config_container_user_namespace(cmdPid);
    config_host_network(cmdPid);
    
    // Signal to the container process we're done with setup.
    if (write(pipe, "START", 5) != 5){
      printf("Pipe Write Error!\n");
      exit(EXIT_FAILURE);
    }
    if (close(pipe)){
      printf("Pipe Close Error!\n");
      exit(EXIT_FAILURE);
    }

    // Wait for container process.
    if (waitpid(cmdPid, NULL, 0) == -1){
      printf("Failed to Wait for cmd!\n");
      exit(EXIT_FAILURE);
    }

    // Remove NAT
    // @TODO: This command remove all NAT roules.
    do_system_command("iptables -t nat -F");
}
