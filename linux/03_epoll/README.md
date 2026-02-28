# linux/03_epoll

epoll edge-triggered non-blocking TCP echo server and client.

## Model

- **Server**: uses `epoll_create1(EPOLL_CLOEXEC)` + `EPOLLET` (edge-triggered).  All fds are non-blocking.  On each readable event the code drains the fd in a tight `recv` loop until `EAGAIN`, then returns to `epoll_wait`.  Exits when the last client disconnects.
- **Client**: same echo protocol as demo 01 / 02.

## Build

```bash
cmake -S ../.. -B ../../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../../build --parallel
```

## Run

**Terminal 1 – server:**
```bash
./linux/03_epoll/linux03_server
# [server] listening on port 9003
# [server] client connected: 127.0.0.1
# [server] recv (fd=5): hello
# [server] recv (fd=5): ping
# [server] recv (fd=5): bye
# [server] client disconnected (fd=5)
# [server] done.
```

**Terminal 2 – client:**
```bash
./linux/03_epoll/linux03_client
# [client] connected to 127.0.0.1:9003
# [client] echo: hello
# [client] echo: ping
# [client] echo: bye
# [client] done.
```

## Key Points

- `epoll_create1(EPOLL_CLOEXEC)` – creates the epoll instance; `EPOLL_CLOEXEC` closes the fd in child processes.
- `EPOLLET` – **edge-triggered**: the kernel notifies only once when the fd transitions from not-ready to ready.  You **must** drain the fd completely on each event.
- `O(1)` event retrieval vs `O(n)` for `select`/`poll`.
- Port: **9003**
