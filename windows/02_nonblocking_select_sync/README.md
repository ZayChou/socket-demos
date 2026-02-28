# windows/02_nonblocking_select_sync

Non-blocking `select()`-based TCP echo server and client (Winsock).

## Model

- **Server**: single-threaded, all sockets set to non-blocking with `ioctlsocket(FIONBIO, 1)`.  `select()` watches all fds.  Mirrors `linux/02_nonblocking_select_sync`.
- **Client**: same echo protocol.

## Build

```powershell
cmake -S ../.. -B ../../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../../build --config Release --parallel
```

## Run

**Terminal 1 – server:**
```powershell
.\build\windows\02_nonblocking_select_sync\Release\win02_server.exe
```

**Terminal 2 – client:**
```powershell
.\build\windows\02_nonblocking_select_sync\Release\win02_client.exe
```

## Key Points

- `ioctlsocket(s, FIONBIO, &mode)` sets non-blocking mode (no `fcntl` on Windows).
- On Windows, `select()` first argument is ignored (pass `0`).
- `WSAEWOULDBLOCK` is the Windows equivalent of `EAGAIN`.
- Port: **9002**
