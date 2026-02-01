# 💬 NetChat - Multi-threaded Chat Application

![C](https://img.shields.io/badge/c-%2300599C.svg?style=flat-square&logo=c&logoColor=white)
![Node.js](https://img.shields.io/badge/node.js-339933?style=flat-square&logo=nodedotjs&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat-square&logo=linux&logoColor=black)

A dual-mode chat application demonstrating OS concepts with both C socket server and Node.js web server.

---

## 🎯 Features

### C Server (OS Concepts Demo)
- **Multi-threading**: pthread-based concurrent client handling
- **TCP Sockets**: BSD socket programming with SOCK_STREAM
- **Mutex Synchronization**: Thread-safe shared resources
- **Signal Handling**: Graceful SIGINT shutdown
- **File I/O**: Persistent user authentication and message logging
- **Chat Rooms**: Multi-room support with room management
- **Private Messaging**: Direct user-to-user messaging
- **Resource Management**: Client admission control (max 10 clients)

### Web Server (Modern Interface)
- **WebSocket**: Real-time bidirectional communication
- **JWT Authentication**: Secure token-based auth
- **RESTful API**: Express.js backend
- **Modern UI**: Responsive HTML/CSS/JS frontend
- **Bcrypt Hashing**: Secure password storage

---

## 🚀 Quick Start

### Option 1: C Socket Server (Port 8080)

```bash
# Compile
make all

# Terminal 1 - Run C server
make run-server

# Terminal 2+ - Run C clients
make run-client
```

### Option 2: Web Server (Port 3000)

```bash
# Install dependencies
npm install

# Run web server
npm start
# Or: make web

# Open browser
# Visit: http://localhost:3000
```

---

## 📋 C Client Commands

| Command | Description |
|---------|-------------|
| `<message>` | Send to current room |
| `/pm <user> <msg>` | Private message |
| `/join <room>` | Join/create room |
| `/room` | Show current room |
| `/rooms` | List all rooms |
| `/users` | List room users |
| `/help` | Show help menu |

---

## 🏗️ OS Concepts Demonstrated

### 1. Multi-threading (pthreads)
```c
pthread_t tid;
pthread_create(&tid, NULL, handle_client, &client_fd);
pthread_detach(tid);  // Auto cleanup
```

### 2. Thread Synchronization
```c
pthread_mutex_t lock;
pthread_mutex_lock(&lock);
// Critical section: client_count, log_file
pthread_mutex_unlock(&lock);
```

### 3. Socket Programming
```c
// Server: socket() → bind() → listen() → accept()
// Client: socket() → connect() → send()/recv()
```

### 4. Signal Handling
```c
signal(SIGINT, handle_shutdown);  // Ctrl+C
void handle_shutdown(int sig) {
    broadcast_all("Server shutting down");
    cleanup_resources();
    exit(0);
}
```

### 5. File I/O & Persistence
- **users.txt**: User credentials storage
- **chat.log**: Message logging with timestamps
- Mutex-protected file writes

---

## 📁 Project Structure

```
Netchat/
├── server/
│   ├── server.c          # Multi-threaded C server
│   └── server            # Compiled binary (auto-generated)
├── client/
│   ├── client.c          # C client with receive thread
│   └── client            # Compiled binary (auto-generated)
├── public/
│   ├── index.html        # Landing page
│   ├── chat.html         # Chat interface
│   ├── chat.js           # WebSocket client
│   ├── chat.css          # Styles
│   └── script.js         # Auth logic
├── server.js             # Node.js web server
├── package.json          # Node dependencies
├── Makefile              # Build automation
├── .env                  # Environment config
├── .gitignore
├── LICENSE
└── README.md

Runtime Files (auto-generated):
├── users.txt             # C server user database
├── users.json            # Web server user database
└── chat.log              # Message history
```

---

## 🔧 Technical Details

### C Server
- **Port**: 8080
- **Max Clients**: 10
- **Buffer Size**: 1024 bytes
- **Default Room**: "general"
- **Auth**: File-based (users.txt)

### Web Server
- **Port**: 3000
- **Auth**: JWT + bcrypt
- **Transport**: Socket.IO (WebSocket)
- **Storage**: JSON file (users.json)

---

## 🛠️ Build Commands

```bash
make all         # Compile C server & client
make server      # Compile C server only
make client      # Compile C client only
make run-server  # Run C server
make run-client  # Run C client
make web         # Run Node.js web server
make clean       # Remove binaries & logs
make reset       # Clean + rebuild
make help        # Show all commands
```

---

## 🎓 Learning Outcomes

This project demonstrates:
- **Socket Programming**: TCP client-server architecture
- **Concurrency**: Multi-threading with pthreads
- **Synchronization**: Mutex locks, race condition prevention
- **IPC**: Inter-process communication via sockets
- **Signal Handling**: Graceful shutdown
- **File I/O**: Persistent storage, logging
- **Memory Management**: Buffer handling, resource cleanup
- **Modern Web**: WebSocket, JWT, REST APIs

---

## 📝 Example Session

```bash
# Terminal 1 - C Server
$ make run-server
Server running on port 8080...
Maximum clients: 10

# Terminal 2 - Client 1
$ make run-client
Enter your username: Alice
Enter password: pass123
✅ Authentication successful!
🏠 You are now in room: #general

Alice: Hello!
[Server]: Bob has joined #general
[19:45] [#general] Bob: Hi Alice!

# Terminal 3 - Client 2
$ make run-client
Enter your username: Bob
Enter password: pass456
Bob: Hi Alice!
```

---

## 📄 License

MIT License - See [LICENSE](LICENSE) file

---

## 🌟 Features Summary

| Feature | C Server | Web Server |
|---------|----------|------------|
| Multi-threading | ✅ pthreads | ✅ Node.js async |
| Authentication | ✅ File-based | ✅ JWT + bcrypt |
| Private Messages | ✅ | ✅ |
| Chat Rooms | ✅ | ✅ |
| Logging | ✅ chat.log | ✅ console |
| Max Clients | ✅ 10 | ✅ unlimited |
| Graceful Shutdown | ✅ SIGINT | ✅ process events |

---

Built with ❤️ to demonstrate Operating Systems concepts in C
