# windows/03_iocp_async

I/O Completion Port (IOCP) asynchronous TCP echo server and client.

## Model

- **Server**:
  - Creates an IOCP with `CreateIoCompletionPort`.
  - Spawns **one worker thread** that calls `GetQueuedCompletionStatus` in a loop.
  - Main thread accepts one client, associates the socket with the IOCP, posts an initial `WSARecv`.
  - Worker: on recv completion → `WSASend` the echo; on send completion → post next `WSARecv`; on `bye` → close socket.
- **Client**: same echo protocol as demos 01 / 02.

## Build

```powershell
cmake -S ../.. -B ../../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../../build --config Release --parallel
```

## Run

**Terminal 1 – server:**
```powershell
.\build\windows\03_iocp_async\Release\win03_server.exe
```

**Terminal 2 – client:**
```powershell
.\build\windows\03_iocp_async\Release\win03_client.exe
```

## Key Points

- `CreateIoCompletionPort` — creates the IOCP and associates a socket.
- `WSARecv` / `WSASend` with `OVERLAPPED` — initiate async I/O; completion is signalled via the IOCP.
- `GetQueuedCompletionStatus` — dequeue a completed I/O operation.
- The `OVERLAPPED` structure must be the **first** field of the per-socket context so the pointer cast works.
- Port: **9003**
