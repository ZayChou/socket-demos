# Verification Record

> Exact commands and results demonstrating a successful build and test run.

---

## Environment

| Item | Value |
|------|-------|
| OS | Ubuntu 24.04 (ubuntu-latest) |
| Compiler | GCC 13.3.0 |
| CMake | 3.28+ |
| ctest | bundled with CMake |

---

## Configure

```
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
-- The C compiler identification is GNU 13.3.0
-- Platform: Linux/Unix
-- Configuring done
-- Generating done
-- Build files have been written to: .../build
```

---

## Build

```
$ cmake --build build --parallel
[  5%] Building C object linux/01_blocking_sync/CMakeFiles/linux01_server.dir/server.c.o
[ 11%] Building C object linux/02_nonblocking_select_sync/CMakeFiles/linux02_server.dir/server.c.o
[ 16%] Building C object linux/01_blocking_sync/CMakeFiles/linux01_client.dir/client.c.o
[ 22%] Building C object linux/03_epoll/CMakeFiles/linux03_server.dir/server.c.o
[ 27%] Building C object linux/02_nonblocking_select_sync/CMakeFiles/linux02_client.dir/client.c.o
[ 33%] Building C object tests/unit/CMakeFiles/test_placeholder.dir/test_placeholder.c.o
[ 38%] Building C object linux/03_epoll/CMakeFiles/linux03_client.dir/client.c.o
[ 44%] Building C object tests/unit/CMakeFiles/test_echo_helpers.dir/test_echo_helpers.c.o
[ 50%] Linking C executable test_placeholder         -- Built target test_placeholder
[ 55%] Linking C executable test_echo_helpers        -- Built target test_echo_helpers
[ 61%] Linking C executable linux03_client
[ 66%] Linking C executable linux01_client
[ 72%] Linking C executable linux02_client
[ 77%] Linking C executable linux01_server
[ 83%] Linking C executable linux03_server
[ 88%] Linking C executable linux02_server
[ 94%] Building C object tests/integration/CMakeFiles/test_echo_integration.dir/test_echo_integration.c.o
[100%] Linking C executable test_echo_integration    -- Built target test_echo_integration
```

---

## ctest

```
$ cd build && ctest --output-on-failure
Test project .../build
1/3 Test #1: placeholder_unit_test ............   Passed    0.00 sec
2/3 Test #2: unit_echo_helpers ................   Passed    0.00 sec
3/3 Test #3: integration_echo .................   Passed    0.61 sec

100% tests passed, 0 tests failed out of 3
Total Test time (real) = 0.69 sec
```

### Integration test output (test 3)

```
[server] listening on port 9001
[server] client connected: 127.0.0.1
[server] recv: hello
[server] recv: ping
[server] recv: bye
[server] done.
[client] connected to 127.0.0.1:9001
[client] echo: hello
[client] echo: ping
[client] echo: bye
[client] done.
[integration] 01_blocking_sync PASSED

[server] listening on port 9002
[server] client connected: 127.0.0.1
[server] recv (fd=5): hello
[server] recv (fd=5): ping
[server] recv (fd=5): bye
[server] done.
[client] connected to 127.0.0.1:9002
[client] echo: hello
[client] echo: ping
[client] echo: bye
[client] done.
[integration] 02_nonblocking_select_sync PASSED

[server] listening on port 9003
[server] client connected: 127.0.0.1
[server] recv (fd=6): hello
[server] recv (fd=6): ping
[server] recv (fd=6): bye
[server] client disconnected (fd=6)
[server] done.
[client] connected to 127.0.0.1:9003
[client] echo: hello
[client] echo: ping
[client] echo: bye
[client] done.
[integration] 03_epoll PASSED

[integration] all tests PASSED
```

---

## Known Limitations

- Windows integration tests are not automated (require a Windows runner with two terminals).  Run `win0x_server.exe` then `win0x_client.exe` manually to verify.
- The IOCP server (demo 03) uses a fixed `Sleep(500)` to wait for the worker thread, which is sufficient for the echo demo but not production-quality.
- Each demo handles one client at a time for simplicity (blocking demos) or exits after the first client disconnects (select / epoll demos).
