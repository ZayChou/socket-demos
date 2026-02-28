# windows/01_blocking_sync

Blocking synchronous TCP echo server and client (Winsock).

## Model

- **Server**: single-threaded, `accept` one client, `recv`/`send` loop until `bye`, then exits.  Direct Winsock counterpart of `linux/01_blocking_sync`.
- **Client**: connects, sends `hello` / `ping` / `bye`, verifies echoes.

## Build

```powershell
cmake -S ../.. -B ../../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../../build --config Release --parallel
```

## Run

**Terminal 1 – server:**
```powershell
.\build\windows\01_blocking_sync\Release\win01_server.exe
# [server] listening on port 9001
# [server] client connected: 127.0.0.1
# [server] recv: hello
# [server] recv: ping
# [server] recv: bye
# [server] done.
```

**Terminal 2 – client:**
```powershell
.\build\windows\01_blocking_sync\Release\win01_client.exe
# [client] connected to 127.0.0.1:9001
# [client] echo: hello
# [client] echo: ping
# [client] echo: bye
# [client] done.
```

## Key Points

- Uses `WSAStartup` / `WSACleanup` (via `winsock_helpers.h`).
- `SOCKET` and `INVALID_SOCKET` instead of `int` / `-1`.
- `closesocket()` instead of `close()`.
- Port: **9001**
