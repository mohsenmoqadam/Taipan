#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include "taipan.h"

static char container_stack[STACK_SIZE];
static struct params params;

static void report_error(const char *operation)
{
  fprintf(stderr, "Taipan: %s: %s\n", operation, strerror(errno));
}

static unsigned long environment_id(const char *name, unsigned long fallback)
{
  const char *value = getenv(name);
  char *end = NULL;
  unsigned long result;

  if (value == NULL || *value == '\0') {
    return fallback;
  }

  errno = 0;
  result = strtoul(value, &end, 10);
  if (errno != 0 || end == value || *end != '\0') {
    return fallback;
  }

  return result;
}

void print_container_info(void)
{
  printf(ANSI_COLOR_YELLOW "\n");
  printf("        " ANSI_COLOR_RED "---_" ANSI_COLOR_YELLOW " ......._-_--.\n");
  printf("     (" ANSI_COLOR_WHITE "|" ANSI_COLOR_YELLOW "\\ /      / /"
         ANSI_COLOR_WHITE "|" ANSI_COLOR_YELLOW " \\  \\\n");
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
  printf(ANSI_COLOR_RESET "\n");
  printf(ANSI_COLOR_GREEN " Container IP:" ANSI_COLOR_RESET "      %s\n",
         params.container_ip);
  printf(ANSI_COLOR_GREEN " Container gateway:" ANSI_COLOR_RESET " %s\n",
         params.container_gw);
  printf(ANSI_COLOR_GREEN " Container DNS:" ANSI_COLOR_RESET "     %s\n",
         params.container_dns);
  printf(ANSI_COLOR_GREEN " Container command:" ANSI_COLOR_RESET " %s\n",
         params.container_cmd);
  printf(ANSI_COLOR_GREEN " Container rootfs:" ANSI_COLOR_RESET "  %s\n\n",
         params.container_rfs);
}

int write_file(const char *path, const char *line)
{
  FILE *file = fopen(path, "w");

  if (file == NULL) {
    report_error(path);
    return -1;
  }

  if (fputs(line, file) == EOF) {
    report_error(path);
    fclose(file);
    return -1;
  }

  if (fclose(file) == EOF) {
    report_error(path);
    return -1;
  }

  return 0;
}

int run_command(const char *fmt, ...)
{
  va_list args;
  char *command = NULL;
  int status;

  va_start(args, fmt);
  if (vasprintf(&command, fmt, args) == -1) {
    va_end(args);
    report_error("format command");
    return -1;
  }
  va_end(args);

  status = system(command);
  if (status == -1) {
    report_error(command);
    free(command);
    return -1;
  }

  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    fprintf(stderr, "Taipan: command failed: %s\n", command);
    free(command);
    return -1;
  }

  free(command);
  return 0;
}

int config_container_user_namespace(pid_t pid)
{
  char path[64];
  char line[64];

  snprintf(path, sizeof(path), "/proc/%ld/uid_map", (long)pid);
  snprintf(line, sizeof(line), "0 %lu 1\n", (unsigned long)params.host_uid);
  if (write_file(path, line) == -1) {
    return -1;
  }

  snprintf(path, sizeof(path), "/proc/%ld/setgroups", (long)pid);
  if (write_file(path, "deny\n") == -1) {
    return -1;
  }

  snprintf(path, sizeof(path), "/proc/%ld/gid_map", (long)pid);
  snprintf(line, sizeof(line), "0 %lu 1\n", (unsigned long)params.host_gid);
  return write_file(path, line);
}

int config_container_filesystem(void)
{
  const char *old_root = ".old_root";

  /* A private mount tree prevents this demonstration from changing host mounts. */
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
    report_error("make mount tree private");
    return -1;
  }

  if (mount(params.container_rfs, params.container_rfs, NULL, MS_BIND, NULL) == -1) {
    report_error("bind container rootfs");
    return -1;
  }
  if (chdir(params.container_rfs) == -1) {
    report_error("enter container rootfs");
    return -1;
  }
  if (mkdir(old_root, 0700) == -1 && errno != EEXIST) {
    report_error("create old root directory");
    return -1;
  }
  if (syscall(SYS_pivot_root, ".", old_root) == -1) {
    report_error("pivot_root");
    return -1;
  }
  if (chdir("/") == -1) {
    report_error("enter new root");
    return -1;
  }

  if (mkdir("/proc", 0555) == -1 && errno != EEXIST) {
    report_error("create /proc");
    return -1;
  }
  if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
    report_error("mount /proc");
    return -1;
  }
  if (umount2("/.old_root", MNT_DETACH) == -1) {
    report_error("detach old root");
    return -1;
  }
  if (rmdir("/.old_root") == -1) {
    report_error("remove old root directory");
    return -1;
  }

  return 0;
}

int config_container_network(void)
{
  char resolver[128];

  if (run_command("ip link set lo up") == -1 ||
      run_command("ip link set eth0 up") == -1 ||
      run_command("ip address add %s dev eth0", params.container_ip) == -1 ||
      run_command("ip route add default via %s dev eth0", params.container_gw) == -1) {
    return -1;
  }

  snprintf(resolver, sizeof(resolver), "nameserver %s\n", params.container_dns);
  return write_file("/etc/resolv.conf", resolver);
}

int prepare_container_cmd(void)
{
  char *save_pointer = NULL;
  char *token;
  size_t argument_count = 0;

  if (snprintf(params.command_buffer, sizeof(params.command_buffer), "%s",
               params.container_cmd) >= (int)sizeof(params.command_buffer)) {
    fprintf(stderr, "Taipan: container command is too long.\n");
    return -1;
  }

  token = strtok_r(params.command_buffer, " \t", &save_pointer);
  while (token != NULL) {
    if (argument_count + 1U >= MAX_COMMAND_ARGUMENTS) {
      fprintf(stderr, "Taipan: container command has too many arguments.\n");
      return -1;
    }
    params.argv[argument_count++] = token;
    token = strtok_r(NULL, " \t", &save_pointer);
  }
  params.argv[argument_count] = NULL;

  if (argument_count == 0U) {
    fprintf(stderr, "Taipan: container command cannot be empty.\n");
    return -1;
  }

  return 0;
}

int container(void *unused)
{
  char start_signal[5];
  ssize_t bytes_read;

  (void)unused;
  close(params.fd[1]);
  bytes_read = read(params.fd[0], start_signal, sizeof(start_signal));
  close(params.fd[0]);
  if (bytes_read != (ssize_t)sizeof(start_signal) ||
      memcmp(start_signal, "START", sizeof(start_signal)) != 0) {
    fprintf(stderr, "Taipan: parent setup did not complete.\n");
    return EXIT_FAILURE;
  }

  if (config_container_filesystem() == -1) {
    return EXIT_FAILURE;
  }
  if (setgid(0) == -1) {
    report_error("set container group identity");
    return EXIT_FAILURE;
  }
  if (setuid(0) == -1) {
    report_error("set container user identity");
    return EXIT_FAILURE;
  }
  if (config_container_network() == -1) {
    return EXIT_FAILURE;
  }

  print_container_info();
  execvp(params.argv[0], params.argv);
  report_error(params.container_cmd);
  return EXIT_FAILURE;
}

int config_host_network(pid_t container_pid)
{
  FILE *ip_forward;

  if (run_command("ip link add cable type veth peer name eth0") == -1) {
    return -1;
  }
  params.veth_created = true;

  if (run_command("ip link set eth0 netns %ld", (long)container_pid) == -1 ||
      run_command("ip link set cable up") == -1 ||
      run_command("ip address add %s dev cable", params.host_ip) == -1) {
    return -1;
  }

  if (system("command -v iptables >/dev/null 2>&1") == 0) {
    ip_forward = fopen("/proc/sys/net/ipv4/ip_forward", "r");
    if (ip_forward == NULL || fscanf(ip_forward, "%d", &params.original_ip_forward) != 1) {
      if (ip_forward != NULL) {
        fclose(ip_forward);
      }
      fprintf(stderr, "Taipan: cannot read the IPv4 forwarding state.\n");
      return -1;
    }
    fclose(ip_forward);

    if (params.original_ip_forward != 1) {
      if (write_file("/proc/sys/net/ipv4/ip_forward", "1\n") == -1) {
        return -1;
      }
      params.ip_forward_changed = true;
    }

    if (run_command("iptables -t nat -A POSTROUTING -s %s -j MASQUERADE",
                    params.container_ip) == -1) {
      return -1;
    }
    params.nat_rule_added = true;
  } else {
    fprintf(stderr,
            "Taipan: iptables is not installed; host/container networking works, "
            "but outbound NAT is disabled.\n");
  }

  return 0;
}

void cleanup_host_network(void)
{
  char forwarding_state[4];

  if (params.nat_rule_added) {
    (void)run_command("iptables -t nat -D POSTROUTING -s %s -j MASQUERADE",
                      params.container_ip);
  }
  if (params.ip_forward_changed) {
    snprintf(forwarding_state, sizeof(forwarding_state), "%d\n",
             params.original_ip_forward);
    (void)write_file("/proc/sys/net/ipv4/ip_forward", forwarding_state);
  }
  if (params.veth_created) {
    (void)system("ip link delete cable >/dev/null 2>&1");
  }
}

void print_usage(const char *program_name)
{
  printf("Usage: %s [options]\n\n", program_name);
  printf("  -m, --host-ip ADDRESS       Host end of the veth pair\n");
  printf("  -i, --container-ip ADDRESS  Container address with prefix\n");
  printf("  -g, --container-gw ADDRESS  Container default gateway\n");
  printf("  -d, --container-dns ADDRESS Container DNS server\n");
  printf("  -c, --container-cmd COMMAND Command to run inside the container\n");
  printf("  -r, --container-rfs PATH    Path to the container root filesystem\n");
  printf("  -h, --help                   Show this help\n");
}

int get_user_defined_options(int argc, char **argv)
{
  int option;
  static const struct option long_options[] = {
    {"host-ip", required_argument, NULL, 'm'},
    {"container-ip", required_argument, NULL, 'i'},
    {"container-gw", required_argument, NULL, 'g'},
    {"container-dns", required_argument, NULL, 'd'},
    {"container-cmd", required_argument, NULL, 'c'},
    {"container-rfs", required_argument, NULL, 'r'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
  };

  while ((option = getopt_long(argc, argv, "m:i:g:d:c:r:h", long_options, NULL)) != -1) {
    switch (option) {
      case 'm': params.host_ip = optarg; break;
      case 'i': params.container_ip = optarg; break;
      case 'g': params.container_gw = optarg; break;
      case 'd': params.container_dns = optarg; break;
      case 'c': params.container_cmd = optarg; break;
      case 'r': params.container_rfs = optarg; break;
      case 'h':
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
      default:
        print_usage(argv[0]);
        return -1;
    }
  }

  if (optind != argc) {
    fprintf(stderr, "Taipan: unexpected positional argument: %s\n", argv[optind]);
    return -1;
  }

  return prepare_container_cmd();
}

int main(int argc, char **argv)
{
  pid_t container_pid;
  int clone_flags;
  int child_status = EXIT_FAILURE;

  memset(&params, 0, sizeof(params));
  params.host_ip = "192.168.1.1/30";
  params.container_ip = "192.168.1.2/30";
  params.container_gw = "192.168.1.1";
  params.container_dns = "1.1.1.1";
  params.container_cmd = "sh";
  params.container_rfs = "./taipan_rfs";
  params.host_uid = (uid_t)environment_id("SUDO_UID", (unsigned long)getuid());
  params.host_gid = (gid_t)environment_id("SUDO_GID", (unsigned long)getgid());

  if (get_user_defined_options(argc, argv) == -1) {
    return EXIT_FAILURE;
  }
  if (geteuid() != 0) {
    fprintf(stderr, "Taipan: run this demonstration with sudo.\n");
    return EXIT_FAILURE;
  }
  if (pipe(params.fd) == -1) {
    report_error("create synchronization pipe");
    return EXIT_FAILURE;
  }

  clone_flags = SIGCHLD | CLONE_NEWUTS | CLONE_NEWUSER | CLONE_NEWNS |
                CLONE_NEWNET | CLONE_NEWPID;
  container_pid = clone(container, container_stack + STACK_SIZE, clone_flags, NULL);
  if (container_pid == -1) {
    report_error("clone container process");
    close(params.fd[0]);
    close(params.fd[1]);
    return EXIT_FAILURE;
  }

  close(params.fd[0]);
  if (config_container_user_namespace(container_pid) == -1 ||
      config_host_network(container_pid) == -1) {
    close(params.fd[1]);
    (void)waitpid(container_pid, NULL, 0);
    cleanup_host_network();
    return EXIT_FAILURE;
  }
  if (write(params.fd[1], "START", 5) != 5) {
    report_error("signal container process");
    close(params.fd[1]);
    (void)waitpid(container_pid, NULL, 0);
    cleanup_host_network();
    return EXIT_FAILURE;
  }
  close(params.fd[1]);

  if (waitpid(container_pid, &child_status, 0) == -1) {
    report_error("wait for container process");
    cleanup_host_network();
    return EXIT_FAILURE;
  }

  cleanup_host_network();
  if (!WIFEXITED(child_status)) {
    fprintf(stderr, "Taipan: container process ended unexpectedly.\n");
    return EXIT_FAILURE;
  }
  return WEXITSTATUS(child_status);
}
