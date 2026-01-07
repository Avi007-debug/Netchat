# 💬 NetChat - Advanced Multi-threaded Chat Application

<div align="center">

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![Threads](https://img.shields.io/badge/Multi--threaded-blue?style=for-the-badge)
![Sockets](https://img.shields.io/badge/BSD_Sockets-orange?style=for-the-badge)

**A feature-rich, enterprise-grade chat server demonstrating advanced OS and networking concepts**

[Features](#-features) • [Installation](#-installation) • [Usage](#-usage) • [Architecture](#-architecture) • [Viva Points](#-viva-points)

</div>

---

## 🎯 Features

### 🔥 **Level 1: Core Features** (Implemented ✅)

<table>
<tr>
<td width="50%">

#### 1️⃣ Join/Leave Notifications ⭐⭐⭐
```
[Server]: Santhosh has joined #general
[Server]: Movie has left #general
```
**Shows:** Server awareness, lifecycle management

</td>
<td width="50%">

#### 2️⃣ Message Timestamps ⭐⭐⭐
```
[19:45] [#general] Santhosh: Hello!
```
**Shows:** Time handling, formatting

</td>
</tr>

<tr>
<td>

#### 3️⃣ No Echo (Anti-Loop) ⭐⭐⭐
Sender doesn't see their own messages echoed back

**Shows:** Attention to detail, proper broadcast logic

</td>
<td>

#### 4️⃣ Max Client Limit ⭐⭐⭐⭐
```
Server full. Try again later.
```
**Shows:** Resource allocation, admission control

</td>
</tr>

<tr>
<td>

#### 5️⃣ Graceful Shutdown ⭐⭐⭐⭐
```bash
Ctrl+C → Notifies all clients → Clean exit
```
**Shows:** Signal handling, resource cleanup

</td>
<td>

#### 6️⃣ Message Logging ⭐⭐⭐⭐
All messages saved to `chat.log` with mutex protection

**Shows:** File I/O, thread-safe shared resource

</td>
</tr>
</table>

---

### 🚀 **Level 2: Advanced Features** (Implemented ✅)

<table>
<tr>
<td width="33%">

#### 7️⃣ Private Messaging ⭐⭐⭐⭐⭐
```bash
/pm Movie Hey, how are you?
```
**Output:**
```
[PM to Movie]: Hey, how are you?
[PM from Santhosh]: Hey, how are you?
```

**Demonstrates:**
- Command parsing
- Username → socket mapping
- Point-to-point communication

</td>
<td width="33%">

#### 8️⃣ User Authentication ⭐⭐⭐⭐⭐
```
Username: Santhosh
Password: chat123
✓ Authentication successful!
```

**Demonstrates:**
- Security awareness
- Session validation
- Access control
- Secure connection setup

</td>
<td width="33%">

#### 9️⃣ Chat Rooms/Channels ⭐⭐⭐⭐⭐
```bash
/join oslab
/join cnlab
/rooms
/users
```

**Demonstrates:**
- Multi-group communication
- Scalability architecture
- Slack/Discord-like design

</td>
</tr>
</table>

---

## 📋 Complete Command Reference

| Command | Description | Example |
|---------|-------------|---------|
| `/pm <user> <msg>` | Send private message | `/pm Alice Hello there!` |
| `/join <room>` | Join/switch chat room | `/join oslab` |
| `/rooms` | List all active rooms | `/rooms` |
| `/users` | List users in current room | `/users` |
| `Ctrl+C` (server) | Graceful shutdown | Notifies all clients |

---

## 🛠️ Installation

### Prerequisites
```bash
# Linux/Unix system with:
- GCC compiler
- pthread library
- POSIX sockets
```

### Compile

```bash
# Clone repository
git clone https://github.com/yourusername/Netchat.git
cd Netchat

# Compile server
gcc -o server/server server/server.c -lpthread

# Compile client
gcc -o client/client client/client.c -lpthread
```

---

## 🚀 Usage

### 1. Start Server
```bash
cd server
./server
```

**Output:**
```
Server running on port 8080...
Maximum clients: 10
Press Ctrl+C for graceful shutdown
```

### 2. Connect Clients
```bash
cd client
./client
```

**Login:**
```
=== NetChat Client ===
Enter your username: Santhosh
Enter password: chat123
Connected to server...
✓ Authentication successful! Welcome to NetChat!

Commands:
  /pm <user> <message> - Send private message
  /join <room> - Join a chat room
  /rooms - List available rooms
  /users - List users in current room
```

---

## 💡 Example Session

### Terminal 1 (Server)
```
Server running on port 8080...
[Server]: Santhosh has joined #general
[19:30] [#general] Santhosh: Hello everyone!
[Server]: Movie has joined #general
[19:31] [#general] Movie: Hey Santhosh!
[Server]: Santhosh has left #general
[Server]: Santhosh has joined #oslab
[PM] Santhosh -> Movie: Check out the oslab room
```

### Terminal 2 (Client - Santhosh)
```
Enter your username: Santhosh
Enter password: chat123
✓ Authentication successful!

[Server]: Santhosh has joined #general
Hello everyone!
[19:31] [#general] Movie: Hey Santhosh!
/join oslab
[Server]: You joined #oslab
/pm Movie Check out the oslab room
[PM to Movie]: Check out the oslab room
```

### Terminal 3 (Client - Movie)
```
Enter your username: Movie
Enter password: chat123
✓ Authentication successful!

[Server]: Movie has joined #general
[19:30] [#general] Santhosh: Hello everyone!
Hey Santhosh!
[Server]: Santhosh has left #general
[PM from Santhosh]: Check out the oslab room
```

---

## 🏗️ Architecture

### System Design
```
┌─────────────────────────────────────────────────────┐
│                   NetChat Server                     │
│  ┌───────────────────────────────────────────────┐  │
│  │  Main Thread (Accept Loop)                    │  │
│  │  • Listens on port 8080                       │  │
│  │  • Validates max client limit                 │  │
│  │  • Spawns worker thread per client            │  │
│  └───────────────────────────────────────────────┘  │
│                       │                              │
│  ┌────────────────────┴────────────────────────┐    │
│  │         Worker Threads (per client)          │    │
│  │  • Authenticates user                        │    │
│  │  • Parses commands (/pm, /join, etc.)       │    │
│  │  • Routes messages (room/private)            │    │
│  │  • Logs all activity                         │    │
│  └──────────────────────────────────────────────┘    │
│                                                       │
│  ┌───────────────────────────────────────────────┐  │
│  │  Shared Resources (Mutex Protected)          │  │
│  │  • Client array (fd, username, room)         │  │
│  │  • Log file (chat.log)                       │  │
│  │  • Client count                              │  │
│  └───────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
         │            │            │
         ▼            ▼            ▼
    Client 1      Client 2      Client 3
```

### Data Structures

```c
typedef struct {
    int fd;                      // Socket file descriptor
    char username[50];           // Authenticated username
    char password[50];           // Stored password
    int authenticated;           // Auth status flag
    char room[30];              // Current room name
} Client;
```

### Thread Synchronization

All shared resources protected by pthread mutex:
- **Client array modifications** (add/remove)
- **Message broadcasting** (room-based)
- **File logging** (append operations)
- **Private message routing** (username lookup)

---

## 🧠 OS & Networking Concepts Demonstrated

### Operating Systems
| Concept | Implementation | Location |
|---------|----------------|----------|
| **Multi-threading** | pthread_create() for concurrent clients | `handle_client()` |
| **Mutex Locks** | pthread_mutex for shared data | `broadcast()`, `log_message()` |
| **Signal Handling** | SIGINT for graceful shutdown | `handle_shutdown()` |
| **Resource Management** | Client limit enforcement | `main()` accept loop |
| **Thread Cleanup** | pthread_detach() for auto cleanup | `main()` |

### Computer Networks
| Concept | Implementation | Location |
|---------|----------------|----------|
| **TCP Sockets** | SOCK_STREAM connection-oriented | `socket()` |
| **Server-Client Model** | 1 server : N clients | Architecture |
| **Multiplexing** | Thread-per-client model | `handle_client()` |
| **Protocol Design** | Custom command protocol (/pm, /join) | Message parsing |
| **Authentication** | Username/password validation | `authenticate_user()` |

---

## 🎓 Viva/Interview Questions & Answers

<details>
<summary><b>1. How do you prevent race conditions in multi-threaded environment?</b></summary>

**Answer:** We use `pthread_mutex_lock()` and `pthread_mutex_unlock()` around all critical sections:
- Client array modifications (add/remove)
- Message broadcasting to prevent interleaved sends
- Log file writes to prevent corrupted output

Example:
```c
pthread_mutex_lock(&lock);
// Critical section: modify shared client array
for (int i = 0; i < client_count; i++) {
    send(clients[i].fd, message, strlen(message), 0);
}
pthread_mutex_unlock(&lock);
```
</details>

<details>
<summary><b>2. How does private messaging work internally?</b></summary>

**Answer:** 
1. Client sends `/pm <username> <message>`
2. Server parses command to extract target username and message
3. Server searches client array (mutex-protected) to find matching username
4. If found, sends message only to that client's socket FD
5. Sends confirmation back to sender

This demonstrates **O(n) lookup** (can optimize with hash table) and **point-to-point communication**.
</details>

<details>
<summary><b>3. Explain the graceful shutdown mechanism</b></summary>

**Answer:** 
1. User presses Ctrl+C → sends **SIGINT** signal
2. Our `signal(SIGINT, handle_shutdown)` handler catches it
3. Handler broadcasts shutdown message to all clients
4. Closes all client sockets (prevents orphan connections)
5. Closes log file (flushes buffer)
6. Destroys mutex
7. Exits with code 0

This prevents resource leaks and informs clients properly.
</details>

<details>
<summary><b>4. How do chat rooms work? Why is this scalable?</b></summary>

**Answer:** 
Each client has a `room` field. When broadcasting:
```c
void broadcast_room(char *msg, int sender_fd, const char *room) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].room, room) == 0 && clients[i].fd != sender_fd) {
            send(clients[i].fd, msg, strlen(msg), 0);
        }
    }
}
```
Only clients in matching room receive messages. **Scalable** because:
- No global broadcast (reduces network traffic)
- Logical separation without creating new sockets
- Similar to Slack/Discord architecture
</details>

<details>
<summary><b>5. What happens if log file write fails mid-operation?</b></summary>

**Answer:** 
```c
if (log_file) {
    fprintf(log_file, "%s", message);
    fflush(log_file);  // Immediate flush to disk
}
```
- We check if `log_file` is valid before writing
- `fflush()` ensures data is written immediately (not buffered)
- If write fails, chat continues (logging is non-critical)
- Could add error handling to reopen file or alert admin
</details>

<details>
<summary><b>6. Why use thread-per-client instead of select/poll?</b></summary>

**Answer:** 

**Thread-per-client (our approach):**
- ✅ Simple blocking I/O (easier to code)
- ✅ True parallelism on multi-core systems
- ✅ Each client has independent execution context
- ❌ Limited by system thread limit (~10K threads)

**select/poll/epoll:**
- ✅ Can handle 100K+ connections
- ✅ Single-threaded (no synchronization needed)
- ❌ More complex code (state machines)
- ❌ No CPU parallelism (one core)

For 10-100 clients (our use case), thread-per-client is perfect balance of simplicity and performance.
</details>

---

## 🔥 What Makes This Project Stand Out

### For Academic Viva/Exams:
✅ **9 major features** covering all OS/CN syllabus topics  
✅ **Thread-safe file I/O** (mutex + logging)  
✅ **Signal handling** (graceful shutdown)  
✅ **Resource management** (max client limit)  
✅ **Protocol design** (command parsing)  
✅ **Authentication** (security awareness)  
✅ **Multi-room architecture** (scalability)  

### For Professional Portfolio:
✅ **Production-quality code** (error handling, cleanup)  
✅ **Enterprise patterns** (similar to Slack/Discord)  
✅ **Comprehensive documentation** (this README!)  
✅ **Demonstrates system design** (not just syntax)  

---

## 📊 Technical Specifications

| Metric | Value |
|--------|-------|
| **Max Concurrent Clients** | 10 (configurable) |
| **Server Port** | 8080 |
| **Buffer Size** | 1024 bytes |
| **Max Rooms** | 5 active rooms |
| **Username Length** | 50 characters |
| **Password** | `chat123` (hardcoded demo) |
| **Threading Model** | POSIX threads (pthreads) |
| **Socket Type** | TCP (SOCK_STREAM) |

---

## 📁 Project Structure

```
Netchat/
├── server/
│   ├── server.c          # Main server logic (400+ lines)
│   └── server            # Compiled binary
├── client/
│   ├── client.c          # Client application
│   └── client            # Compiled binary
├── chat.log              # Generated log file
├── README.md             # Basic README
└── README_GITHUB.md      # This comprehensive guide
```

---

## 🔧 Configuration

Edit these `#define` values in `server.c`:

```c
#define PORT 8080              // Server listening port
#define MAX_CLIENTS 10         // Maximum concurrent connections
#define BUFFER_SIZE 1024       // Message buffer size
#define LOG_FILE "chat.log"    // Log file path
#define MAX_ROOMS 5            // Maximum simultaneous rooms
```

---

## 🐛 Error Handling

### Client Limit Reached
```c
if (client_count >= MAX_CLIENTS) {
    send(client_fd, "Server full. Try again later.\n", 31, 0);
    close(client_fd);
}
```

### Authentication Failure
```c
if (!authenticate_user(username, password)) {
    send(client_fd, "Authentication failed. Disconnecting...\n", 40, 0);
    close(client_fd);
}
```

### User Not Found (PM)
```c
if (!send_private_message(target, msg, sender)) {
    send(client_fd, "[Server]: User not found\n", 25, 0);
}
```

---

## 🚧 Future Enhancements

- [ ] Database integration (MySQL/PostgreSQL) for user accounts
- [ ] Encrypted communication (TLS/SSL)
- [ ] File transfer support
- [ ] GUI client (Qt/GTK)
- [ ] Message history retrieval
- [ ] User roles (admin/moderator/user)
- [ ] Profanity filter
- [ ] Rate limiting (anti-spam)
- [ ] Persistent chat rooms

---

## 📚 Learning Resources

- **POSIX Threads:** `man pthread_create`
- **Berkeley Sockets:** Beej's Guide to Network Programming
- **OS Concepts:** *Operating System Concepts* by Silberschatz (Chapter 4-5)
- **Network Programming:** *Unix Network Programming* by Stevens

---

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.

---

## 👨‍💻 Author

**Santhosh Kumar and Avishkar More**  
- GitHub: [@egvsanthoshkumarcy24-glitch](https://github.com/egvsanthoshkumarcy24-glitch)
- GitHub: [@Avi007-debug](https://github.com/Avi007-debug)
- Project: Academic demonstration of OS and CN concepts with websockets

---

## 🙏 Acknowledgments

- Computer Networks course materials
- POSIX/BSD socket documentation
- The open-source community

---

## 📞 Support

If you have questions or issues:
1. Check the [Wiki](https://github.com/yourusername/Netchat/wiki) (if available)
2. Open an [Issue](https://github.com/yourusername/Netchat/issues)
3. Read the code comments (heavily documented)

---

<div align="center">

**⭐ Star this repo if you found it helpful! ⭐**

Made with ❤️ for learning OS and Computer Networks

![Visitor Count](https://visitor-badge.laobi.icu/badge?page_id=yourusername.Netchat)

</div>
