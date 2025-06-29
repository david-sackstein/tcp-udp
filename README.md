# TCP-over-UDP Proxy Chain

## 1. Purpose

Implements a **TCP-over-UDP proxy chain**.

- **Main products:**
  - `tcpclientproxy` and `tcpserverproxy` — forward TCP connections over UDP, enabling TCP tunneling through UDP.
- **Other binaries:**
  - `tcp_echo_server`, `tcpclient`, `tcpserver`, `udpserver` — used only for testing and demonstration.

---

## 2. Prerequisites

- **OS:** Linux or macOS
- **ACE library:** Must be installed ([The ACE ORB](https://www.dre.vanderbilt.edu/~schmidt/ACE.html))
- **Build system:**
  - Update ACE include/library paths in `Makefile` and `CMakeLists.txt` as needed for your system.

---

## 3. Build & Run

**Build with Make:**
```sh
make
```
**Build with CMake:**
```sh
mkdir build && cd build
cmake ..
make
```

**Run the full proxy chain:**
```sh
./run_proxy_chain.sh ./bin all
```
- This launches:
  - `tcp_echo_server` (TCP echo backend)
  - `tcpclientproxy` (UDP middlebox)
  - `tcpserverproxy` (TCP frontend)
- Logs are written to `*.log` files in the repo root.

---

## 4. Usage

- **Connect your TCP client to the TCP Server Proxy** (default: `127.0.0.1:12345`).
- The proxy chain forwards the connection over UDP to the client proxy, which relays to the echo server.
- See `run_proxy_chain.sh` for port and address details.

---

## 5. Components

**Libraries:**
- `libacetools` — ACE utility wrappers
- `libtcp` — TCP server/client abstractions
- `libudp` — UDP server/client abstractions
- `libtcpclientproxy` — Implements the TCP client proxy logic
- `libtcpserverproxy` — Implements the TCP server proxy logic

**Executables:**
- `tcpclientproxy` — Listens on UDP, relays to TCP echo server
- `tcpserverproxy` — Listens on TCP, relays over UDP to client proxy
- `tcp_echo_server` — Simple TCP echo server (for testing)
- `tcpclient`, `tcpserver`, `udpserver` — Additional test/demo tools

**Relationships:**
- `tcpserverproxy` ⇄UDP⇄ `tcpclientproxy` ⇄TCP⇄ `tcp_echo_server`
- All core logic is in the libraries; executables are thin wrappers.

---

## 6. Tests

- **Unit/Integration tests:**
  - Written with [GoogleTest (gtest)](https://github.com/google/googletest)
  - Run with `make tests` or by executing `bin/tests`
  - Cover TCP/UDP session logic and proxy chain behavior
- **End-to-end test script:**
  - `run_proxy_chain.sh` launches the full proxy chain and echo server
  - You can connect a TCP client to the server proxy and verify end-to-end tunneling
  - Logs for each component are written to `*.log` files in the repo root 