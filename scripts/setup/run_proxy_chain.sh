#!/bin/bash

# Configuration
SERVER_ADDR="127.0.0.1"
SERVERPROXY_PORT=12345
CLIENTPROXY_PORT=12346
ECHOSERVER_PORT=12347

SERVERPROXY_ADDR="$SERVER_ADDR:$SERVERPROXY_PORT"
CLIENTPROXY_ADDR="$SERVER_ADDR:$CLIENTPROXY_PORT"
ECHOSERVER_ADDR="$SERVER_ADDR:$ECHOSERVER_PORT"

# Usage message
usage() {
    echo "Usage: $0 <binary_path> <server>"
    echo "  <server>: all | tcp_echo_server | tcpclientproxy | tcpserverproxy"
    echo "Example: $0 ./bin all"
    exit 1
}

# Arrays to store PIDs and their process names
PIDS=()
NAMES=()

# Trap cleanup on SIGINT and SIGTERM
trap cleanup SIGINT SIGTERM

# Check if a PID is alive
is_alive() {
    local pid=$1
    kill -0 "$pid" 2>/dev/null
}

# Get alive PIDs and their names
get_alive_pids_and_names() {
    local alive_pids=()
    local alive_names=()
    for i in "${!PIDS[@]}"; do
        pid=${PIDS[i]}
        if is_alive "$pid"; then
            alive_pids+=("$pid")
            alive_names+=("${NAMES[i]}")
        fi
    done

    # Return as space-separated strings with | as separator between pids and names
    echo "${alive_pids[*]}|${alive_names[*]}"
}

# Wait up to max_wait seconds for processes to exit early
wait_for_exit() {
    local max_wait=$1
    local waited=0

    while (( waited < max_wait )); do
        IFS='|' read -r alive_pids_str alive_names_str <<< "$(get_alive_pids_and_names)"
        read -ra alive_pids <<< "$alive_pids_str"
        if [ ${#alive_pids[@]} -eq 0 ]; then
            break
        fi
        sleep 1
        ((waited++))
    done

    # Return final alive pids and names
    echo "$alive_pids_str|$alive_names_str"
}

# Kill remaining stubborn processes with SIGKILL
kill_stubborn() {
    local pids=("$@")
    local names=("${NAMES[@]}") # use global NAMES as fallback

    if [ ${#pids[@]} -gt 0 ]; then
        echo "Sending SIGKILL to stubborn processes:"
        for i in "${!pids[@]}"; do
            echo "  PID ${pids[i]} (${names[i]})"
            kill -SIGKILL "${pids[i]}" 2>/dev/null
        done
    fi
}

# Cleanup function
cleanup() {
    echo ""
    echo "Shutting down..."
    echo "Waiting up to 3 seconds for processes to exit gracefully..."

    IFS='|' read -r alive_pids_str alive_names_str <<< "$(wait_for_exit 3)"
    read -ra alive_pids <<< "$alive_pids_str"
    read -ra alive_names <<< "$alive_names_str"

    if [ ${#alive_pids[@]} -gt 0 ]; then
        kill_stubborn "${alive_pids[@]}"
    fi

    wait "${PIDS[@]}" 2>/dev/null
    echo "Cleanup complete"
    exit 0
}

# Check input arguments
if [ $# -lt 2 ]; then
    usage
fi

BIN_PATH="$1"
SERVER_TO_RUN="$2"

# Validate binary path
if [ ! -d "$BIN_PATH" ]; then
    echo "Error: Binary path '$BIN_PATH' does not exist"
    exit 1
fi

# Check required binaries
REQUIRED_BINS=("tcpserverproxy" "tcpclientproxy" "tcp_echo_server")
for bin in "${REQUIRED_BINS[@]}"; do
    if [ ! -f "$BIN_PATH/$bin" ]; then
        echo "Error: Binary '$BIN_PATH/$bin' not found"
        exit 1
    fi
done

# Run functions
run_tcp_echo_server() {
    echo "Starting TCP Echo Server on $ECHOSERVER_ADDR..."
    "$BIN_PATH/tcp_echo_server" "$ECHOSERVER_ADDR" > tcp_echo_server.log 2>&1 &
    PIDS+=($!)
    NAMES+=("tcp_echo_server")
}

run_tcpclientproxy() {
    echo "Starting TCP Client Proxy on $CLIENTPROXY_ADDR, forwarding to $ECHOSERVER_ADDR..."
    "$BIN_PATH/tcpclientproxy" "$CLIENTPROXY_ADDR" "$ECHOSERVER_ADDR" > tcpclientproxy.log 2>&1 &
    PIDS+=($!)
    NAMES+=("tcpclientproxy")
}

run_tcpserverproxy() {
    echo "Starting TCP Server Proxy on $SERVERPROXY_ADDR, forwarding to $CLIENTPROXY_ADDR..."
    "$BIN_PATH/tcpserverproxy" "$SERVERPROXY_ADDR" "$CLIENTPROXY_ADDR" > tcpserverproxy.log 2>&1 &
    PIDS+=($!)
    NAMES+=("tcpserverproxy")
}

if [ "$SERVER_TO_RUN" = "all" ]; then
    echo "Starting proxy chain..."
    echo "Chain: TCP Client -> ServerProxy($SERVERPROXY_PORT) -> ClientProxy($CLIENTPROXY_PORT) -> EchoServer($ECHOSERVER_PORT)"
    echo ""

    run_tcp_echo_server

    run_tcpclientproxy

    run_tcpserverproxy

    echo ""
    echo "Proxy chain is running!"
    echo "TCP clients can now connect to $SERVERPROXY_ADDR"
    echo "Press Ctrl+C to stop all servers"
    echo ""
    echo "Summary of ports in use:"
    echo "  TCP Server Proxy:   $SERVERPROXY_ADDR (TCP)"
    echo "  TCP Client Proxy:   $CLIENTPROXY_ADDR (UDP/TCP as appropriate)"
    echo "  TCP Echo Server:    $ECHOSERVER_ADDR (TCP)"
    echo ""
    echo "To list the processes and ports, run:"
    echo "  lsof -iTCP:$SERVERPROXY_PORT -iUDP:$CLIENTPROXY_PORT -iTCP:$ECHOSERVER_PORT"
    echo ""

    # Tail logs in foreground so you see them live
    tail -f tcp_echo_server.log tcpclientproxy.log tcpserverproxy.log

else
    case "$SERVER_TO_RUN" in
        tcp_echo_server)
            run_tcp_echo_server
            ;;
        tcpclientproxy)
            run_tcpclientproxy
            ;;
        tcpserverproxy)
            run_tcpserverproxy
            ;;
        *)
            echo "Unknown server: $SERVER_TO_RUN"
            usage
            ;;
    esac

    echo ""
    echo "$SERVER_TO_RUN is running. Press Ctrl+C to stop it."

    wait "${PIDS[@]}"
fi
