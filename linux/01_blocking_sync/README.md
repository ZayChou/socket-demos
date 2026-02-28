# linux/01_blocking_sync

Blocking synchronous TCP echo server and client.

## Model

- **Server**: single-threaded, `accept` one client at a time, `recv`/`send` in a loop until the client sends `bye`, then exits.
- **Client**: connects, sends `hello`, `ping`, and `bye`, verifies each echo, then disconnects.

## Build

```bash
cmake -S ../.. -B ../../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../../build --parallel
```

## Run

Open two terminals from the build directory:

**Terminal 1 – server:**
```bash
./linux/01_blocking_sync/linux01_server
# [server] listening on port 9001
# [server] client connected: 127.0.0.1
# [server] recv: hello
# [server] recv: ping
# [server] recv: bye
# [server] done.
```

**Terminal 2 – client:**
```bash
./linux/01_blocking_sync/linux01_client
# [client] connected to 127.0.0.1:9001
# [client] echo: hello
# [client] echo: ping
# [client] echo: bye
# [client] done.
```

## Key Points

- Simplest TCP model: fully blocking `socket` / `bind` / `listen` / `accept` / `recv` / `send`.
- Only one client is served at a time; the server exits after that client disconnects.
- Port: **9001**
