# Taipan

Taipan is a small educational Linux container launcher. It keeps the mechanism
visible: one process is cloned into new namespaces, a user mapping is installed,
the root filesystem is changed with `pivot_root`, and a `veth` pair connects the
new network namespace to the host.

Taipan is intentionally not a production container runtime. It does not attempt
to implement OCI images, cgroups, seccomp policies, layered filesystems, a daemon,
or lifecycle management.

## Requirements

- Linux with user, mount, PID, UTS, and network namespace support
- A C17-capable compiler and `make`
- `iproute2` on both the host and in the supplied root filesystem
- `iptables` only if outbound NAT is required
- Root privileges for the host network setup

## Build and run

```console
git clone https://github.com/mohsenmoqadam/Taipan.git
cd Taipan
make
sudo ./taipan
```

Show the available options:

```console
./taipan --help
```

Run a different command inside the container:

```console
sudo ./taipan --container-cmd "sh -l"
```

The command parser deliberately supports a simple whitespace-separated command,
which keeps the process-replacement step easy to inspect. It is not a shell
parser.

If `iptables` is unavailable, Taipan still creates the host-to-container link but
prints a warning and skips outbound NAT.
