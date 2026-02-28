# 系统架构

> 本文档描述 socket-demos 项目的整体架构与各模块关系。

---

## 架构总览

```
socket-demos/
│
├── [共享协议层]          (docs/protocol.md)
│     └── 行式文本 echo 协议，统一用于所有 6 个 demo
│
├── [平台公共层]
│     ├── linux/common/sock_helpers.h
│     │     die()、set_nonblocking()、write_all()
│     └── windows/common/winsock_helpers.h
│           winsock_init()、winsock_cleanup()、die_wsa()、send_all()
│
├── [Windows 示例]
│     ├── 01_blocking_sync       阻塞 accept/recv/send 单线程
│     ├── 02_nonblocking_select  select() + ioctlsocket(FIONBIO)
│     └── 03_iocp_async          IOCP + WSARecv/WSASend + 工作线程
│
├── [Linux 示例]
│     ├── 01_blocking_sync       阻塞 accept/recv/send 单线程
│     ├── 02_nonblocking_select  select() + fcntl(O_NONBLOCK)
│     └── 03_epoll               epoll 边缘触发 + 非阻塞 I/O
│
└── [测试层]
      ├── unit/        test_placeholder.c — 基本断言
      │                test_echo_helpers.c — bye 检测、write_all 管道
      └── integration/ test_echo_integration.c — Linux fork+exec 端到端测试
```

---

## 各示例设计要点

### 01_blocking_sync（Windows & Linux 共同）

- 单线程：`accept` → `recv` / `send` 循环直到 `bye` → 退出
- 教学重点：最基础的 TCP 编程模型，易于理解

### 02_nonblocking_select_sync（Windows & Linux 共同）

- 单线程：所有 fd 非阻塞，`select()` 轮询就绪事件
- Linux：`fcntl(O_NONBLOCK)`；Windows：`ioctlsocket(FIONBIO)`
- 教学重点：I/O 多路复用初步，`select` 的 fd 上限（`FD_SETSIZE`）

### 03_iocp_async（Windows）

- `CreateIoCompletionPort` 绑定套接字
- 工作线程调用 `GetQueuedCompletionStatus` 处理完成事件
- 教学重点：Windows 原生异步 I/O，零拷贝，高吞吐

### 03_epoll（Linux）

- `epoll_create1(EPOLL_CLOEXEC)` + `EPOLLET`（边缘触发）
- 每次事件必须完全排空 fd（循环读直至 `EAGAIN`）
- 教学重点：Linux 高性能事件驱动，O(1) 事件检索

---

## 构建矩阵

| Runner | 编译目标 | 测试 |
|--------|----------|------|
| ubuntu-latest | linux01–linux03, unit tests, integration test | ctest (3 tests) |
| windows-latest | win01–win03, unit tests | ctest (2 tests) |

---

## 平台公共助手

| 文件 | 平台 | 提供 |
|------|------|------|
| `linux/common/sock_helpers.h` | Linux | `die`, `set_nonblocking`, `write_all` |
| `windows/common/winsock_helpers.h` | Windows | `winsock_init`, `winsock_cleanup`, `die_wsa`, `send_all` |

两个头文件均为 **header-only**，直接 `#include` 使用，无需链接额外库。

