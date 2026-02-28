# socket-demos

> 通过六个渐进式示例，对比演示 Windows 与 Linux 平台上不同的 socket I/O 模型。

---

## 项目目标

| # | 目录 | I/O 模型 | 平台 |
|---|------|----------|------|
| 1 | `windows/01_blocking_sync` | 阻塞同步 | Windows |
| 2 | `windows/02_nonblocking_select_sync` | 非阻塞 select | Windows |
| 3 | `windows/03_iocp_async` | IOCP 异步 | Windows |
| 4 | `linux/01_blocking_sync` | 阻塞同步 | Linux |
| 5 | `linux/02_nonblocking_select_sync` | 非阻塞 select | Linux |
| 6 | `linux/03_epoll` | epoll 边缘触发 | Linux |

每个示例均包含一对 `server.c` / `client.c`，可独立编译运行。

---

## 目录结构

```
socket-demos/
├── CMakeLists.txt          # 顶层构建文件
├── cmake/                  # 公共 CMake 模块（预留）
├── windows/
│   ├── 01_blocking_sync/
│   ├── 02_nonblocking_select_sync/
│   └── 03_iocp_async/
├── linux/
│   ├── 01_blocking_sync/
│   ├── 02_nonblocking_select_sync/
│   └── 03_epoll/
├── tests/
│   ├── unit/               # 单元测试
│   └── integration/        # 集成测试（后续 PR 补全）
└── docs/
    ├── protocol.md         # 应用层协议草案
    └── architecture.md     # 整体架构草案
```

---

## 构建命令

### Linux / macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Windows（Developer Command Prompt 或 PowerShell）

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

---

## 测试命令

```bash
cd build
ctest --output-on-failure
```

---

## CI 状态

每次向 `main` 分支推送或发起 Pull Request，GitHub Actions 将自动在
`ubuntu-latest` 和 `windows-latest` 上执行 configure → build → ctest。

---

## 后续计划

后续 PR 将逐步补全六个 socket 示例的完整业务逻辑：

- [ ] PR-2：`linux/01_blocking_sync` 完整实现
- [ ] PR-3：`linux/02_nonblocking_select_sync` 完整实现
- [ ] PR-4：`linux/03_epoll` 完整实现
- [ ] PR-5：`windows/01_blocking_sync` 完整实现
- [ ] PR-6：`windows/02_nonblocking_select_sync` 完整实现
- [ ] PR-7：`windows/03_iocp_async` 完整实现
