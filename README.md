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
├── CMakeLists.txt              # 顶层构建文件
├── cmake/                      # 公共 CMake 模块（预留）
├── windows/
│   ├── common/                 # Winsock 公共助手头文件
│   │   └── winsock_helpers.h
│   ├── 01_blocking_sync/       # server.c  client.c  README.md
│   ├── 02_nonblocking_select_sync/
│   └── 03_iocp_async/
├── linux/
│   ├── common/                 # Linux socket 公共助手头文件
│   │   └── sock_helpers.h
│   ├── 01_blocking_sync/       # server.c  client.c  README.md
│   ├── 02_nonblocking_select_sync/
│   └── 03_epoll/
├── tests/
│   ├── unit/                   # 单元测试（协议逻辑、write_all 等）
│   └── integration/            # 集成测试（Linux：fork + exec）
└── docs/
    ├── protocol.md             # 应用层协议说明
    ├── architecture.md         # 整体架构说明
    └── verification.md         # 构建 / 运行 / 测试验证记录
```

---

## 构建命令

### Linux

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

## 运行示例（以 Linux 01 为例）

```bash
# Terminal 1 — server
./build/linux/01_blocking_sync/linux01_server
# [server] listening on port 9001

# Terminal 2 — client
./build/linux/01_blocking_sync/linux01_client
# [client] connected to 127.0.0.1:9001
# [client] echo: hello
# [client] echo: ping
# [client] echo: bye
# [client] done.
```

端口分配：

| Demo | Port |
|------|------|
| `**/01_blocking_sync` | 9001 |
| `**/02_nonblocking_select_sync` | 9002 |
| `**/03_iocp_async` / `03_epoll` | 9003 |

---

## 测试命令

```bash
cd build
ctest --output-on-failure
```

测试项：
- `placeholder_unit_test` — 基本算术和字符串断言
- `unit_echo_helpers` — 协议 bye 检测、`write_all` 管道测试
- `integration_echo` — 三个 Linux demo 的端到端 echo 验证（Linux 专用）

详细验证结果见 [docs/verification.md](docs/verification.md)。

---

## CI 状态

每次向 `main` 分支推送或发起 Pull Request，GitHub Actions 将自动在
`ubuntu-latest` 和 `windows-latest` 上执行 configure → build → ctest。

---

## 发布流程（CD Release）

推送一个符合 `v*.*.*` 格式的 Git tag 即可触发自动发布流程：

```bash
# 1. 确保本地 main 已是最新
git checkout main
git pull

# 2. 创建版本 tag（语义化版本）
git tag v1.0.0

# 3. 推送 tag 到远端，触发 CD 工作流
git push origin v1.0.0
```

工作流将自动完成：

1. 在 `ubuntu-latest` 和 `windows-latest` 上构建并运行所有测试。
2. 将 Linux 可执行文件打包为 `socket-demos-linux-v1.0.0.tar.gz`。
3. 将 Windows 可执行文件打包为 `socket-demos-windows-v1.0.0.zip`。
4. 在 GitHub Releases 页面创建对应版本的 Release，并上传两个资产。

> **注意**：tag 格式必须严格匹配 `v<major>.<minor>.<patch>`（例如 `v1.0.0`、`v2.3.1`），
> 其他格式不会触发发布流程。

---

## 协议

所有 demo 使用统一的**行式文本 echo 协议**：

- 客户端发送一行文本（以 `\n` 结尾）
- 服务端原样回显
- 客户端发送 `bye\n` 触发连接关闭

详见 [docs/protocol.md](docs/protocol.md)。

