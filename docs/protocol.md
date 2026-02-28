# 应用层协议草案

> 本文档描述各 socket demo 所使用的简单应用层协议。后续 PR 补全实现时将以此为准。

---

## 概述

所有六个 demo 均使用相同的极简文本协议，以便横向对比不同 I/O 模型的实现差异，
而不是协议本身的复杂度。

---

## 消息格式

```
+--------+----------+---------+
| LEN(4) | TYPE(1)  | PAYLOAD |
+--------+----------+---------+
```

| 字段 | 大小 | 说明 |
|------|------|------|
| LEN | 4 字节，大端序 uint32 | PAYLOAD 的字节长度 |
| TYPE | 1 字节 | 消息类型（见下表） |
| PAYLOAD | LEN 字节 | UTF-8 文本 |

---

## 消息类型

| 值 | 名称 | 方向 | 说明 |
|----|------|------|------|
| `0x01` | HELLO | Client → Server | 握手，PAYLOAD 为客户端标识字符串 |
| `0x02` | HELLO_ACK | Server → Client | 握手应答，PAYLOAD 为 "OK" 或错误描述 |
| `0x03` | ECHO_REQ | Client → Server | 请求回显，PAYLOAD 为任意文本 |
| `0x04` | ECHO_RSP | Server → Client | 回显响应，PAYLOAD 与请求相同 |
| `0x05` | BYE | 双向 | 正常关闭，PAYLOAD 为空 |

---

## 会话流程

```
Client                          Server
  |                               |
  |── HELLO("demo-client") ──────>|
  |<── HELLO_ACK("OK") ──────────|
  |                               |
  |── ECHO_REQ("ping") ──────────>|
  |<── ECHO_RSP("ping") ─────────|
  |                               |
  |── BYE() ──────────────────────>|
  |<── BYE() ──────────────────── |
  |                               |
 [close]                        [close]
```

---

## 错误处理

- 收到未知 TYPE 时，Server 回复 `HELLO_ACK` 携带错误描述后关闭连接。
- 读取超时（默认 30 s）时，双方直接关闭连接。

---

## 后续扩展（暂不实现）

- 压缩：PAYLOAD 可选 zlib 压缩（TYPE 高位标志位）。
- TLS：在 TCP 之上叠加 TLS 1.3。
