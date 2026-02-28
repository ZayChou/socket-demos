cmake_minimum_required(VERSION 3.16)
project(socket-demos C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── CTest ───────────────────────────────────────────────────────────────────
enable_testing()

# ── Platform detection ──────────────────────────────────────────────────────
if(WIN32)
    message(STATUS "Platform: Windows")
    add_compile_definitions(_WIN32_WINNT=0x0601)
    set(SOCKET_LIBS ws2_32)
else()
    message(STATUS "Platform: Linux/Unix")
    set(SOCKET_LIBS "")
endif()

# ── Sub-directories ─────────────────────────────────────────────────────────
if(WIN32)
    add_subdirectory(windows/01_blocking_sync)
    add_subdirectory(windows/02_nonblocking_select_sync)
    add_subdirectory(windows/03_iocp_async)
else()
    add_subdirectory(linux/01_blocking_sync)
    add_subdirectory(linux/02_nonblocking_select_sync)
    add_subdirectory(linux/03_epoll)
endif()

add_subdirectory(tests)
