# 💬 NetChat - Multi-threaded Chat Application

![C](https://img.shields.io/badge/c-%2300599C.svg?style=flat-square&logo=c&logoColor=white)
![Node.js](https://img.shields.io/badge/node.js-339933?style=flat-square&logo=nodedotjs&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat-square&logo=linux&logoColor=black)

A dual-mode chat application demonstrating operating system concepts with both C socket server and Node.js web server implementations. Perfect for 2nd year engineering OS projects.

---

## 🎯 Features Overview

### C Servers - Two Flavors

#### Standard C Server (`server.c`) - Simple Threading Demo
- ✅ **Multi-threading**: pthread-based concurrent client handling
- ✅ **TCP Sockets**: BSD socket programming (SOCK_STREAM)
- ✅ **Mutex Synchronization**: Thread-safe shared resources
- ✅ **Signal Handling**: Graceful SIGINT shutdown
- ✅ **File I/O**: Persistent user authentication and message logging
- ✅ **Chat Rooms**: Multi-room support with `/join`, `/room`, `/rooms`, `/users` commands
- ✅ **Private Messaging**: Direct user-to-user messaging via `/pm` command
- ✅ **Resource Management**: Client admission control (max 10 clients)
- ⚠️ **Port**: 8080
- ⚠️ **Process Model**: Single process, multiple threads
- ⚠️ **IPC**: File-based only

#### Enhanced C Server (`server_enhanced.c`) - Advanced OS Concepts
*All features from Standard Server, PLUS:*
- ✅ **IPC - Shared Memory**: Cross-process message buffer (shmget/shmat)
- ✅ **IPC - Message Queues**: Offline message delivery (POSIX mqueue)
- ✅ **Process Forking**: Separate process per client connection
- ✅ **Semaphores**: Named semaphores for resource control
- ✅ **Advanced Synchronization**: Complex process coordination
- ⚠️ **Port**: 5555
- ⚠️ **Process Model**: One process per client (forking architecture)
- ⚠️ **Best For**: OS learning, advanced IPC demonstrations

### Web Server (Modern Interface)
- ✅ **WebSocket**: Real-time bidirectional communication (Socket.IO)
- ✅ **JWT Authentication**: Secure token-based auth with 24hr expiry
- ✅ **RESTful API**: Express.js backend with input validation
- ✅ **Modern UI**: Responsive HTML/CSS/JS frontend with gradient design
- ✅ **Bcrypt Hashing**: Secure password storage (10 salt rounds)
- ✅ **Image Sharing**: Upload and share images (JPEG, PNG, GIF, WebP up to 5MB)
- ✅ **Private Messaging**: Click-to-chat modal with message history and notifications
- ✅ **Message Encryption**: Client-side toggle for AES-256-CBC encrypted messages
- ✅ **Session Management**: Anti-hijacking duplicate login prevention
- ✅ **Chat Rooms**: Create and join unlimited rooms
- ✅ **Typing Indicators**: Real-time typing status
- ✅ **Modern UI**: Responsive HTML/CSS/JS frontend
- ✅ **Bcrypt Hashing**: Secure password storage (10 rounds)
- ✅ **Image Sharing**: Upload/share images (JPEG, PNG, GIF, WebP up to 5MB)
- ✅ **Message Encryption**: AES-256-CBC encrypted chat option
- ✅ **Private Messaging**: Click-to-chat modal with history
- ✅ **Session Management**: Anti-hijacking duplicate login prevention

See [FEATURES.md](FEATURES.md) for complete feature list.

---

## 🚀 Quick Start

### Option 1: Web Server (Recommended for Most Users)

```bash
# 1. Install dependencies
npm install

# 2. Start server (port 3000)
npm start

# 3. Open browser
# Navigate to: http://localhost:3000
```

### Option 2: Standard C Server (Simple Threading Demo)

```bash
# 1. Compile
make server

# 2. Terminal 1 - Run server (port 8080)
make run-server

# 3. Terminal 2+ - Run clients
make run-client
```

### Option 3: Enhanced C Server (Advanced OS Concepts)

```bash
# 1. Compile
make enhanced

# 2. Terminal 1 - Run enhanced server (port 5555)
make run-enhanced

# 3. Terminal 2+ - Run clients
make run-client
```

For detailed setup instructions, see [SETUP.md](SETUP.md).

---

## 📁 Project Structure

```
Netchat/
├── server/
│   ├── server.c                # Standard C server (multi-threaded)
│   ├── server_enhanced.c       # Enhanced C server (multi-process, IPC, shared memory)
│   ├── server_enhanced         # Compiled enhanced binary
│   ├── server                  # Compiled standard binary
│   └── chat.log                # Server message logs
├── client/
│   ├── client.c                # C client implementation
│   └── client                  # Compiled client binary
├── public/
│   ├── index.html              # Login/register page
│   ├── chat.html               # Main chat interface
│   ├── chat.js                 # Client-side logic
│   ├── chat.css                # Styling
│   ├── script.js               # Auth logic
│   ├── styles.css              # Landing page styles
│   └── uploads/                # Image storage (auto-created)
├── server.js                   # Node.js web server
├── package.json                # Node.js dependencies
├── users.json                  # Web server user database
├── users.txt                   # C server user database
├── Makefile                    # Build automation (root directory)
├── README.md                   # This file
├── FEATURES.md                 # Complete feature documentation
├── SETUP.md                    # Installation & setup guide
└── .env                        # Environment configuration
```

---

## 🎓 OS Concepts Demonstrated

This project demonstrates essential operating system concepts across multiple implementations:

### Standard Server (`server.c`) - Fundamental Concepts

| Concept | Implementation | Used For |
|---------|---------------|----------|
| **Multi-threading** | pthread library for concurrent clients | Core threading model |
| **Socket Programming** | TCP/IP BSD sockets (SOCK_STREAM) | Network fundamentals |
| **Mutex Synchronization** | pthread_mutex for thread safety | Race condition prevention |
| **Shared Memory** | Global arrays protected by mutex | Thread-safe data sharing |
| **Signal Handling** | SIGINT for graceful shutdown | Process signal handling |
| **File I/O** | Read/write users.txt and chat.log | Persistent storage |

### Enhanced Server (`server_enhanced.c`) - Advanced Concepts

*All concepts from Standard Server, plus:*

| Concept | Implementation | Used For |
|---------|---------------|----------|
| **Process Forking** | fork() for separate client processes | Parent-child relationships |
| **Shared Memory (IPC)** | shmget/shmat for cross-process buffer | Inter-process communication |
| **Message Queues (IPC)** | POSIX mqueue for offline delivery | Async message passing |
| **Semaphores** | Named semaphores for resource control | Process synchronization |
| **Process Groups** | getpgrp() and signal handling | Process management |
| **Zombie Process Handling** | wait()/waitpid() cleanup | Process lifecycle |

### Web Server (`server.js`) - Modern Concepts

| Concept | Implementation | Used For |
|---------|---------------|----------|
| **Session Management** | Single-session enforcement | Security best practices |
| **Stream Processing** | Multer file streams for uploads | Efficient file handling |
| **Authentication** | JWT tokens with 24hr expiry | Secure communication |
| **Encryption** | AES-256-CBC message encryption | Data privacy |
| **WebSocket Protocol** | Socket.IO for real-time comm | Modern async I/O |
| **Hash Functions** | Bcrypt (10 rounds) for passwords | Cryptography fundamentals |

---

## 🛠️ Technology Stack

### Standard C Server (`server.c`)
- **Language**: C (C11 standard)
- **Architecture**: Multi-threaded (single process)
- **Threading**: POSIX threads (pthread)
- **Networking**: BSD Sockets (TCP/IP, port 8080)
- **Synchronization**: Mutexes only
- **IPC**: File-based (users.txt, chat.log)
- **Max Clients**: 10 concurrent
- **Best For**: Learning threading & synchronization
- **Compile**: `make server` or `make run-server`

### Enhanced C Server (`server_enhanced.c`)
- **Language**: C (C11 standard)
- **Architecture**: Multi-process (fork per client)
- **Process Management**: Parent-child, process groups
- **Networking**: BSD Sockets (TCP/IP, port 5555)
- **IPC**: Shared Memory + Message Queues + Semaphores
- **Synchronization**: Named semaphores
- **Max Clients**: 10 concurrent
- **Best For**: Learning advanced OS concepts (IPC, process management)
- **Compile**: `make enhanced` or `make run-enhanced`
- **Dependencies**: librt (real-time POSIX library)

### Web Server (`server.js`)
- **Runtime**: Node.js (v14+)
- **Framework**: Express.js
- **Real-time**: Socket.IO (WebSockets)
- **Authentication**: JWT + Bcrypt (10 salt rounds)
- **File Upload**: Multer (5MB limit)
- **Port**: 3000
- **Message Encryption**: AES-256-CBC (optional)
- **Session Management**: Anti-hijacking duplicate login prevention
- **Encryption**: Node.js crypto (AES-256-CBC)
- **Port**: 3000
- **Transport**: HTTP + WebSocket

---

## 📊 Build Commands

| Command | Description |
|---------|-------------|
| `make all` | Compile standard server and client (default) |
| `make server` | Compile standard server only |
| `make client` | Compile client only |
| `make enhanced` | Compile enhanced server with OS features |
| `make debug` | Compile enhanced server with debug symbols |
| `make run-server` | Compile and run standard C server (port 8080) |
| `make run-enhanced` | Compile and run enhanced C server (port 5555) |
| `make run-client` | Compile and run C client |
| `make web` | Start Node.js web server (port 3000) |
| `make install` | Install enhanced server to `/usr/local/bin` |
| `make clean` | Remove all binaries, logs, and temp files |
| `make reset` | Clean and rebuild all targets |
| `make help` | Show detailed help with examples |

---

## 🎯 Feature Comparison

| Feature | C Server | Web Server |
|---------|----------|------------|
| **Real-time Chat** | ✅ TCP Sockets | ✅ WebSockets |
| **Authentication** | ✅ File-based | ✅ JWT + Bcrypt |
| **Multi-threading** | ✅ pthreads | ✅ Node.js event loop |
| **Chat Rooms** | ✅ `/join` command | ✅ UI buttons |
| **Private Messages** | ✅ `/pm` command | ✅ Click-to-chat UI |
| **Image Sharing** | ❌ | ✅ Upload + preview |
| **Encryption** | ❌ | ✅ AES-256-CBC |
| **Web Interface** | ❌ Terminal only | ✅ Modern UI |
| **Shared Memory** | ✅ shmget/shmat | ❌ |
| **Message Queues** | ✅ POSIX mqueue | ❌ |
| **Process Forking** | ✅ fork() | ❌ |
| **Semaphores** | ✅ sem_open | ❌ |

---

## 🎬 Demo Flow

**1. Web Server Demo (3 minutes)**
- Register two users
- Join same room
- Send text messages
- **Toggle encryption 🔐** and send encrypted message
- Share an image with caption
- Open private chat
- Demonstrate duplicate session prevention
- Show duplicate login prevention

**2. C Server Demo (2 minutes)**
- Connect two clients
- Show pthread logs
- Demonstrate room commands
- Show shared memory buffer
- Display process forking
- Graceful shutdown (Ctrl+C)

See [SETUP.md](SETUP.md) for detailed demo script.

---

## 🔧 Development

### Running in Development Mode
```bash
# Node.js server with auto-restart
npm run dev

# C server with debug info
make clean && make all && ./server/server
```

### Debugging
```bash
# C server with GDB
gdb ./server/server

# Check for memory leaks
valgrind --leak-check=full ./server/server

# Monitor system calls
strace ./server/server
```

---

## 🐛 Troubleshooting

| Issue | Solution |
|-------|----------|
| Port already in use | `lsof -ti:3000 \| xargs kill -9` or change PORT in .env |
| Cannot connect | Check firewall settings |
| Image upload fails | Verify `public/uploads/` exists |
| C compilation error | `sudo apt-get install build-essential` |
| Module not found | `npm install` |

See [SETUP.md](SETUP.md) for comprehensive troubleshooting guide.

---

## 📝 License

MIT License - See [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **POSIX Threads**: Multi-threading implementation
- **Socket.IO**: Real-time communication
- **Express.js**: Web framework
- **Multer**: File upload handling
- Inspired by classic chat applications and OS textbooks

---

## 📧 Support

For issues, questions, or contributions:
1. Check [FEATURES.md](FEATURES.md) for feature documentation
2. Read [SETUP.md](SETUP.md) for setup help
3. Review existing GitHub issues
4. Create a new issue with detailed description

---

**Built with ❤️ for Operating Systems course project**

**Status**: ✅ Production ready | 🎓 Educational project | 🚀 Actively maintained

---

## 🚀 Future Enhancements

Potential improvements for advanced projects:
- Load balancing across multiple servers
- Redis for distributed session management
- Database integration (PostgreSQL/MongoDB)
- End-to-end encryption
- Voice/video chat
- Docker containerization
- Kubernetes deployment
- CI/CD pipeline

See [IMPROVEMENT_SUGGESTIONS.md](IMPROVEMENT_SUGGESTIONS.md) for detailed implementation guides.
