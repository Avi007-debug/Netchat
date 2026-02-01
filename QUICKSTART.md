# NetChat Quick Start

## 🚀 Two Ways to Run

### Option 1: C Socket Server (Terminal-based)
```bash
# Build
make all

# Terminal 1 - Start server
make run-server

# Terminal 2, 3, etc - Connect clients
make run-client
```

**Default credentials**: Any username/password (auto-registers)

**Commands**:
- `/help` - Show commands
- `/join <room>` - Switch rooms
- `/pm <user> <msg>` - Private message
- `/users` - See who's online

---

### Option 2: Web Server (Browser-based)
```bash
# Install dependencies (first time only)
npm install

# Start server
npm start

# Open browser
http://localhost:3000
```

**Create account** on web interface, then login.

---

## 📋 OS Concepts Featured

### C Server Implementation
✅ **Multi-threading** - pthread for each client  
✅ **Mutex locks** - Thread-safe client list & log file  
✅ **TCP Sockets** - socket(), bind(), listen(), accept()  
✅ **Signal handling** - SIGINT (Ctrl+C) graceful shutdown  
✅ **File I/O** - Persistent auth (users.txt) & logging (chat.log)  
✅ **Resource management** - Max 10 clients, admission control  
✅ **Broadcast logic** - Room-based & private messaging  

### Architecture
```
Main Thread ──┬──> Client Thread 1 (Alice)
              ├──> Client Thread 2 (Bob)
              ├──> Client Thread 3 (Carol)
              └──> ...
                   │
                   ├──> Mutex Lock
                   ├──> Client List (shared)
                   └──> Log File (shared)
```

---

## 🔧 Make Commands

```bash
make help        # Show all commands
make all         # Build C server & client
make run-server  # Start C server
make run-client  # Start C client
make web         # Start web server
make clean       # Remove binaries & logs
make reset       # Clean + rebuild
```

---

## 📝 First Time Setup (Linux)

```bash
# 1. Make scripts executable
chmod +x setup.sh start.sh

# 2. Check dependencies
./setup.sh

# 3. Install Node.js deps (for web mode)
npm install

# 4. Run!
./start.sh
```

---

## 🐛 Troubleshooting

**Port already in use?**
```bash
# C server (port 8080)
lsof -ti:8080 | xargs kill -9

# Web server (port 3000)
lsof -ti:3000 | xargs kill -9
```

**Compilation errors?**
```bash
# Install build tools
sudo apt-get install build-essential

# Rebuild
make clean
make all
```

**Web server won't start?**
```bash
# Reinstall dependencies
rm -rf node_modules package-lock.json
npm install
```

---

## 📊 File Overview

| File | Purpose |
|------|---------|
| `server/server.c` | Multi-threaded C socket server |
| `client/client.c` | C client with receive thread |
| `server.js` | Node.js web server (Express + Socket.IO) |
| `public/` | Web UI (HTML/CSS/JS) |
| `Makefile` | Build automation |
| `start.sh` | Interactive launcher |
| `setup.sh` | Dependency checker |

---

**Project ready for Linux! 🐧**

Run `./start.sh` to get started.
