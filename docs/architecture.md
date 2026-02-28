# 系统架构草案

> 本文档描述 socket-demos 项目的整体架构与各模块关系。

---

## 架构总览

```
socket-demos/
│
├── [共享协议层]          (docs/protocol.md)
│     └── 消息格式、类型定义、会话状态机
│
├── [平台抽象层]          (cmake/ 中的宏/函数，后续 PR 补全)
│     ├── 地址族、套接字选项封装
│     └── 跨平台错误码转换
│
├── [Windows 示例]
│     ├── 01_blocking_sync       阻塞 accept/recv/send 单线程
│     ├── 02_nonblocking_select  select() 多路复用
│     └── 03_iocp_async          IOCP + 完成端口线程池
│
├── [Linux 示例]
│     ├── 01_blocking_sync       阻塞 accept/recv/send 单线程
│     ├── 02_nonblocking_select  select() 多路复用
│     └── 03_epoll               epoll 边缘触发 + 非阻塞 I/O
│
└── [测试层]
      ├── unit/        纯逻辑单元测试（不依赖网络）
      └── integration/ 端对端测试（后续 PR 补全）
```

---

## 各示例设计要点

### 01_blocking_sync（Windows & Linux 共同）

- 单线程：`accept` → 派生子进程/线程 → `recv`/`send` 循环
- 教学重点：最基础的 TCP 编程模型，易于理解，无法扩展到高并发

### 02_nonblocking_select_sync（Windows & Linux 共同）

- 单线程：将所有 fd 放入 `fd_set`，调用 `select()` 轮询
- 教学重点：I/O 多路复用初步，`select` 的 fd 上限（FD_SETSIZE = 1024）

### 03_iocp_async（Windows）

- 使用 `CreateIoCompletionPort` 绑定套接字
- 线程池调用 `GetQueuedCompletionStatus` 处理完成事件
- 教学重点：Windows 原生异步 I/O，零拷贝，吞吐量最优

### 03_epoll（Linux）

- `epoll_create1(EPOLL_CLOEXEC)` + `EPOLLET`（边缘触发）
- 配合非阻塞 fd，循环读直至 `EAGAIN`
- 教学重点：Linux 高性能事件驱动，O(1) 事件检索

---

## 构建矩阵

| Runner | 编译目标 | 测试 |
|--------|----------|------|
| ubuntu-latest | linux01-linux03, tests | ctest |
| windows-latest | win01-win03, tests | ctest |

---

## 后续扩展方向（本 PR 不实现）

1. 引入 `cmake/platform.cmake` 封装跨平台宏
2. 集成 Unity 或 CMocka 作为单元测试框架
3. 添加性能基准测试（吞吐 / 延迟对比）
4. 补全集成测试：自动启动 server、连接 client、验证 ECHO 回显
