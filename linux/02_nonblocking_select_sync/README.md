# linux/02_nonblocking_select_sync

Non-blocking `select()`-based TCP echo server and client.

## Model

- **Server**: single-threaded, all sockets set to non-blocking with `fcntl(O_NONBLOCK)`.  `select()` watches the listening socket and all connected clients in one call.  Handles multiple simultaneous clients (up to `MAX_CLIENTS`).  Exits when all clients have disconnected.
- **Client**: same echo protocol as demo 01; connects, sends `hello` / `ping` / `bye`, verifies echoes.

## Build

```bash
cmake -S ../.. -B ../../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../../build --parallel
```

## Run

**Terminal 1 – server:**
```bash
./linux/02_nonblocking_select_sync/linux02_server
# [server] listening on port 9002
# [server] client connected: 127.0.0.1
# [server] recv (fd=5): hello
# [server] recv (fd=5): ping
# [server] recv (fd=5): bye
# [server] done.
```

**Terminal 2 – client:**
```bash
./linux/02_nonblocking_select_sync/linux02_client
# [client] connected to 127.0.0.1:9002
# [client] echo: hello
# [client] echo: ping
# [client] echo: bye
# [client] done.
```

## Key Points

- All fds are put in non-blocking mode with `set_nonblocking()`.
- `select()` watches the full fd set; the loop re-builds the `fd_set` each iteration.
- Teaching point: `select` has a hard limit of `FD_SETSIZE` (typically 1 024) file descriptors.
- Port: **9002**
