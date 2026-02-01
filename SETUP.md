# 🚀 NetChat - Installation & Setup Guide

Complete guide for installing, running, and testing NetChat.

---

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [Configuration](#configuration)
4. [Running the Application](#running-the-application)
5. [First-Time Setup](#first-time-setup)
6. [Testing Procedures](#testing-procedures)
7. [Troubleshooting](#troubleshooting)
8. [Directory Structure](#directory-structure)
9. [Development Tools](#development-tools)
10. [Demo Script](#demo-script)

---

## ✅ Prerequisites

### Required Software

**For All Users**:
- **Node.js**: v14.0.0 or higher
  - Check: `node --version`
  - Download: https://nodejs.org/

**For C Server Users** (Linux/macOS/WSL):
- **GCC Compiler**: Latest version
  - Linux: `sudo apt install build-essential`
  - macOS: `xcode-select --install`
  - WSL: `sudo apt install gcc make`
- **POSIX Libraries**: 
  - `pthread` (usually pre-installed)
  - `librt` (for message queues - enhanced server only)

**For Windows Users**:
- **WSL** (Windows Subsystem for Linux) or **MinGW** for C server
- **Git Bash** or **PowerShell** for Node.js server

### System Requirements
- **RAM**: Minimum 2GB (4GB recommended)
- **Disk Space**: 100MB for Node.js dependencies
- **Ports**: 3000 (web), 5555 (C server)
- **Network**: Internet for npm packages

---

## 📦 Installation

### Step 1: Clone the Repository

```bash
cd c:\Coding\Animation\Netchat
# Repository already exists at this location
```

### Step 2: Install Node.js Dependencies

```bash
npm install
```

This installs:
- express (4.18.2)
- socket.io (4.6.1)
- bcryptjs (2.4.3)
- jsonwebtoken (9.0.2)
- multer (1.4.5-lts.1)
- crypto (1.0.1)
- nodemon (dev dependency)

### Step 3: Verify Installation

```bash
# Check package.json
cat package.json

# Verify node_modules folder
ls node_modules/
```

Expected output:
```
node_modules/
  bcryptjs/
  express/
  jsonwebtoken/
  multer/
  socket.io/
  ...
```

### Step 4: Compile C Server (Optional)

**Standard Server**:
```bash
cd server
gcc server.c -o server -lpthread
```

**Enhanced Server** (with OS features):
```bash
cd server
gcc server_enhanced.c -o server_enhanced -lpthread -lrt
```

**Using Makefile** (if available):
```bash
cd server
make
make enhanced
```

---

## ⚙️ Configuration

### 1. Environment Variables

Create `.env` file in project root:

```env
# Server Configuration
PORT=3000
NODE_ENV=development

# JWT Configuration
JWT_SECRET=your_super_secret_key_change_in_production

# Encryption Configuration
ENCRYPTION_KEY=your_encryption_key_min_32_chars_long

# C Server Configuration
C_SERVER_HOST=localhost
C_SERVER_PORT=5555
```

**Security Note**: Change `JWT_SECRET` and `ENCRYPTION_KEY` in production!

### 2. Create Required Directories

```bash
# From project root
mkdir -p public/uploads
mkdir -p server
mkdir -p logs
```

### 3. Verify File Permissions

```bash
# Make C server executable
chmod +x server/server
chmod +x server/server_enhanced  # If using enhanced server

# Make uploads directory writable
chmod 755 public/uploads
```

---

## ▶️ Running the Application

### Option 1: Web Server Only (Recommended)

**Production Mode**:
```bash
npm start
```

**Development Mode** (auto-restart on changes):
```bash
npm run dev
```

Server runs at: `http://localhost:3000`

### Option 2: C Server Only (Terminal Chat)

**Standard Server**:
```bash
cd server
./server
```

**Enhanced Server** (with OS features):
```bash
cd server
./server_enhanced
```

Server listens on: `localhost:5555`

**Connect as Client**:
```bash
telnet localhost 5555
# OR
nc localhost 5555
```

### Option 3: Both Servers (Full Experience)

**Terminal 1** (C Server):
```bash
cd server
./server_enhanced
```

**Terminal 2** (Web Server):
```bash
npm start
```

**Browser**: Navigate to `http://localhost:3000`

---

## 🎬 First-Time Setup

### 1. Start the Server

```bash
npm start
```

### 2. Open in Browser

Navigate to: `http://localhost:3000`

### 3. Create First User

1. Click **"Don't have an account? Register here"**
2. Fill in:
   - Username: `alice`
   - Email: `alice@example.com`
   - Password: `password123`
3. Click **Register**
4. Automatically logged in

### 4. Explore the Interface

- Default room: `#general`
- Type a message and press Enter
- Click 💬 to see online users
- Click 📷 to test image upload

---

## 🧪 Testing Procedures

### Test 1: Image Upload - Basic

**Objective**: Verify image upload works with valid file

**Steps**:
1. Login as any user
2. Click **📷** camera button
3. Select image file (JPEG, PNG, GIF, or WebP)
4. **(Optional)** Type caption: "Test image"
5. Press Enter or wait for auto-upload

**Expected Result**:
- ✅ Image appears as thumbnail in chat
- ✅ Toast notification: "Image uploaded successfully"
- ✅ Image clickable for full-screen view
- ✅ Caption displays below image (if provided)

**Verification**:
```bash
# Check file was saved
ls -lh public/uploads/
# Should show image-{timestamp}-*.{ext}
```

---

### Test 2: Image Upload - With Caption

**Objective**: Verify caption functionality

**Steps**:
1. Type caption in message box: "Check out this sunset!"
2. Click **📷** and select image
3. Wait for upload

**Expected Result**:
- ✅ Image displayed with caption below
- ✅ Caption text: "Check out this sunset!"
- ✅ Caption styling matches message text

---

### Test 3: Image Upload - Multi-user

**Objective**: Verify image sharing across users

**Setup**:
1. Open two browser windows (incognito + normal)
2. Login as `alice` and `bob`
3. Both in same room (#general)

**Steps**:
1. As Alice: Upload image with caption "Hello from Alice"
2. As Bob: Watch chat window

**Expected Result**:
- ✅ Bob sees Alice's image in real-time
- ✅ Image appears with Alice's username
- ✅ Caption displays correctly for Bob
- ✅ Bob can click to view full-screen

---

### Test 4: Image Upload - File Size Validation

**Objective**: Verify 5MB limit enforcement

**Steps**:
1. Create large image > 5MB
   ```bash
   # Generate 6MB test image (ImageMagick)
   convert -size 3000x3000 xc:blue large_image.jpg
   ```
2. Attempt upload

**Expected Result**:
- ❌ Upload rejected immediately
- ❌ Toast error: "Image too large. Maximum size is 5MB."
- ❌ Button remains 📷 (not ⏳)

**Verification**:
```bash
# Confirm no file saved
ls -lh public/uploads/ | grep large_image
# Should return empty
```

---

### Test 5: Image Upload - File Type Validation

**Objective**: Verify only images accepted

**Steps**:
1. Attempt to upload:
   - PDF file
   - Text file (.txt)
   - ZIP file
   - MP4 video

**Expected Result** (for each):
- ❌ File picker may filter (browser-dependent)
- ❌ If selected, toast error: "Invalid file type. Only JPEG, PNG, GIF, and WebP are allowed."
- ❌ No upload attempted

---

### Test 6: Image Upload - Authentication Check

**Objective**: Verify JWT protection

**Steps**:
1. Open DevTools → Console
2. Clear localStorage: `localStorage.clear()`
3. Reload page (should redirect to login)
4. **Without logging in**, attempt API call:
   ```javascript
   fetch('http://localhost:3000/api/upload/image', {
     method: 'POST',
     body: new FormData()
   }).then(r => r.json()).then(console.log)
   ```

**Expected Result**:
- ❌ Response: `{error: "No token provided"}`
- ❌ Status: 401 Unauthorized
- ❌ Upload fails

---

### Test 7: Private Messaging - Click-to-Chat

**Objective**: Test UI-based private messaging

**Setup**: Two users (Alice, Bob) logged in

**Steps**:
1. As Alice: Click 💬 next to Bob's name
2. Type message: "Hey Bob, this is private"
3. Send
4. As Bob: Check for notification

**Expected Result**:
- ✅ Alice's PM modal opens with Bob's name
- ✅ Message appears in Alice's modal (purple background)
- ✅ Bob receives desktop notification (if allowed)
- ✅ Toast notification for Bob: "New message from Alice"
- ✅ Bob clicks 💬 next to Alice → sees message history

---

### Test 8: Duplicate Session Prevention

**Objective**: Verify anti-hijacking feature

**Setup**: One user account (Alice)

**Steps**:
1. **Window 1**: Login as Alice
2. **Window 2** (incognito): Login as Alice again
3. Check Window 1

**Expected Result**:
- ✅ Window 1 disconnects
- ✅ Alert: "Your account has been logged in from another location"
- ✅ Window 1 redirected to login
- ✅ Window 2 remains connected

**Re-login Test**:
4. **Window 1**: Login as Alice again
5. Check Window 2

**Expected Result**:
- ✅ Window 2 disconnects
- ✅ Window 1 stays connected

---

## 🐛 Troubleshooting

### Issue: Port 3000 Already in Use

**Error**:
```
Error: listen EADDRINUSE: address already in use :::3000
```

**Solution 1** - Kill existing process:
```bash
# Find process on port 3000
netstat -ano | findstr :3000      # Windows
lsof -i :3000                     # Linux/Mac

# Kill process
taskkill /PID <PID> /F            # Windows
kill -9 <PID>                     # Linux/Mac
```

**Solution 2** - Change port:
```javascript
// server.js - Line 15
const PORT = process.env.PORT || 4000;  // Changed from 3000
```

---

### Issue: C Server Compilation Errors

**Error**:
```
undefined reference to `pthread_create'
```

**Solution**: Link pthread library
```bash
gcc server.c -o server -lpthread
```

**Error**:
```
undefined reference to `mq_open'
```

**Solution**: Link realtime library (enhanced server)
```bash
gcc server_enhanced.c -o server_enhanced -lpthread -lrt
```

**Error**:
```
fatal error: pthread.h: No such file or directory
```

**Solution**: Install build tools
```bash
# Ubuntu/Debian
sudo apt install build-essential

# macOS
xcode-select --install
```

---

### Issue: Module Not Found Errors

**Error**:
```
Error: Cannot find module 'multer'
```

**Solution**: Reinstall dependencies
```bash
# Delete node_modules and package-lock.json
rm -rf node_modules package-lock.json

# Reinstall
npm install
```

---

### Issue: Image Upload Fails Silently

**Diagnostic Steps**:

1. **Check server logs**:
   ```bash
   # Look for errors in terminal
   ```

2. **Verify uploads directory**:
   ```bash
   ls -la public/uploads/
   # Should be writable
   ```

3. **Check browser console**:
   ```javascript
   // DevTools → Console
   // Look for 401 (auth) or 500 (server) errors
   ```

4. **Test authentication**:
   ```javascript
   console.log(localStorage.getItem('token'))
   // Should show JWT token
   ```

**Common Fixes**:
- Recreate uploads directory: `mkdir -p public/uploads`
- Check file permissions: `chmod 755 public/uploads`
- Verify JWT token valid (check expiry)

---

### Issue: Encryption Not Working

**Error**: Messages don't encrypt

**Diagnostic**:
1. Check environment variable:
   ```bash
   echo $ENCRYPTION_KEY
   # Should show your key (min 32 chars)
   ```

2. Verify server.js loaded encryption:
   ```javascript
   // Check server.js top section
   const crypto = require('crypto');
   console.log(crypto); // Should not be undefined
   ```

**Solutions**:
- Set `ENCRYPTION_KEY` in `.env` file
- Restart server after `.env` changes
- Verify key length ≥ 32 characters

---

### Issue: Private Messages Not Appearing

**Diagnostic**:
1. Check both users online
2. Verify correct username spelling
3. Check browser console for errors

**Solution**:
- Refresh both browser windows
- Re-login both users
- Check server logs for socket errors

---

## 📂 Directory Structure

```
c:\Coding\Animation\Netchat\
│
├── server/                      # C server source code
│   ├── server.c                 # Standard multi-threaded server
│   ├── server_enhanced.c        # Enhanced with OS features
│   ├── server                   # Compiled binary (standard)
│   ├── server_enhanced          # Compiled binary (enhanced)
│   └── users.txt                # C server user database
│
├── public/                      # Static web files
│   ├── index.html               # Login page
│   ├── register.html            # Registration page
│   ├── chat.html                # Main chat interface
│   ├── chat.js                  # Client-side chat logic
│   ├── chat.css                 # Chat styling
│   └── uploads/                 # Uploaded images
│       └── image-*.{jpg|png|gif|webp}
│
├── server.js                    # Node.js web server
├── package.json                 # Node dependencies
├── package-lock.json            # Locked versions
├── users.json                   # Web server user database
├── .env                         # Environment variables (create manually)
│
├── README.md                    # Project overview
├── FEATURES.md                  # Complete features list
├── SETUP.md                     # This file
│
└── chat.log                     # Chat message logs (auto-created)
```

---

## 🛠️ Development Tools

### Makefile Commands

Create `server/Makefile`:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lpthread -lrt

all: server

server: server.c
	$(CC) $(CFLAGS) server.c -o server -lpthread

enhanced: server_enhanced.c
	$(CC) $(CFLAGS) server_enhanced.c -o server_enhanced $(LDFLAGS)

run: server
	./server

run-enhanced: enhanced
	./server_enhanced

clean:
	rm -f server server_enhanced *.o

.PHONY: all enhanced run run-enhanced clean
```

**Usage**:
```bash
make               # Compile standard server
make enhanced      # Compile enhanced server
make run           # Compile & run standard
make run-enhanced  # Compile & run enhanced
make clean         # Remove binaries
```

### NPM Scripts

Already configured in `package.json`:

```json
{
  "scripts": {
    "start": "node server.js",
    "dev": "nodemon server.js"
  }
}
```

**Usage**:
```bash
npm start     # Production mode
npm run dev   # Development mode (auto-restart)
```

---

## 🎭 Demo Script

Use this script for live demonstrations or testing.

### Scenario: Feature Showcase

**Characters**:
- **Alice**: Demo presenter
- **Bob**: Second user for multi-user features

---

#### Part 1: Registration & Login (2 minutes)

**Narrator**: "Let's see how easy it is to get started..."

1. Navigate to `http://localhost:3000`
2. Click **"Don't have an account? Register here"**
3. Register as Alice:
   - Username: `alice`
   - Email: `alice@demo.com`
   - Password: `demo123`
4. Auto-redirected to chat

**Narrator**: "Notice instant login after registration."

---

#### Part 2: Basic Chat (1 minute)

**Narrator**: "Alice joins the default #general room..."

5. Type: "Hello everyone!"
6. Press Enter
7. Message appears with timestamp

**Narrator**: "Messages include sender name and time."

---

#### Part 3: Image Sharing (2 minutes)

**Narrator**: "Let's share an image..."

8. Type caption: "Check out this cool feature!"
9. Click **📷** button
10. Select image file
11. Wait for upload (observe ⏳ indicator)
12. Image appears as thumbnail
13. Click thumbnail → full-screen modal
14. Close modal

**Narrator**: "Images upload instantly and support captions."

---

#### Part 4: Private Messaging (3 minutes)

**Setup**: Open incognito window, register/login as Bob

**Narrator**: "Now let's test private messaging between users..."

15. **As Bob**: Type "Hi Alice" in public chat
16. **As Alice**: See Bob in users list
17. Click 💬 next to Bob's name
18. PM modal opens
19. Type: "Hey Bob, this is a private message!"
20. Send

**Narrator**: "Bob receives a notification..."

21. **As Bob**: See desktop notification (if allowed)
22. See toast: "New message from Alice"
23. Click 💬 next to Alice
24. See PM history

**Narrator**: "Private messages are completely separate from public chat."

---

#### Part 5: Multi-Room Chat (2 minutes)

**Narrator**: "Users can create and join different rooms..."

25. **As Alice**: Type `/join gaming`
26. Room switches to #gaming
27. Type: "Anyone here play chess?"
28. **As Bob**: Still in #general (doesn't see Alice's message)
29. **As Bob**: Type `/join gaming`
30. **As Bob**: See Alice's message

**Narrator**: "Rooms isolate conversations by topic."

---

#### Part 6: Duplicate Session Prevention (2 minutes)

**Narrator**: "Let's test our anti-hijacking security..."

31. **As Alice** (original window): Currently logged in
32. **New Window** (incognito): Login as Alice again
33. **Original Window**: Alert appears: "Account logged in from another location"
34. **Original Window**: Disconnected and redirected
35. **New Window**: Remains connected

**Narrator**: "Only one active session per user - security first!"

36. **Original Window**: Login as Alice again
37. **New Window**: Gets disconnected

**Narrator**: "Real user can always reclaim their account."

---

#### Part 7: C Server Demo (3 minutes)

**Narrator**: "Let's connect with our enhanced C server..."

38. Open terminal
39. Run: `telnet localhost 5555`
40. Login/register as Charlie
41. See welcome banner with OS features:
   ```
   ═══════════════════════════════════════════════
       ENHANCED NETCHAT - OS CONCEPTS EDITION
   ═══════════════════════════════════════════════
   Features:
     🔹 Shared Memory (IPC)
     🔹 Message Queues
     🔹 Process Forking (PID: 12345)
     🔹 Semaphore Control
   ═══════════════════════════════════════════════
   ```

42. Type `/help` → See full command list
43. Type `/recent` → See recent messages from shared memory
44. Type `/rooms` → See active rooms
45. Type `/join gaming` → Join room
46. Chat with Alice and Bob

**Narrator**: "The C server showcases advanced OS concepts like shared memory and process forking."

---

### Success Criteria Checklist

After demo, verify:

- [ ] Registration works
- [ ] Login persists on refresh
- [ ] Messages appear in real-time
- [ ] Images upload successfully
- [ ] Images display as thumbnails
- [ ] Full-screen image viewer works
- [ ] Private messages deliver instantly
- [ ] PM modal shows history
- [ ] Desktop notifications work (if permission granted)
- [ ] Room switching works
- [ ] Messages isolated per room
- [ ] Duplicate session kicks old session
- [ ] C server connects successfully
- [ ] C server commands work (`/help`, `/recent`, `/rooms`)
- [ ] Shared memory displays recent messages
- [ ] No console errors in browser
- [ ] No server crashes

---

## 🎓 Performance Testing

### Load Testing with Multiple Users

**Tool**: Artillery.io (install globally)

```bash
npm install -g artillery

# Create artillery-test.yml
artillery quick --count 10 --num 50 http://localhost:3000
```

**Expected**: Server handles 10 concurrent users with 50 requests each

### Memory Leak Testing

**Tool**: Node.js built-in

```bash
node --inspect server.js
# Open Chrome DevTools
# Monitor memory over 30 minutes
```

**Expected**: Memory usage stable, no continuous growth

---

## 📞 Support & Further Help

**Documentation**:
- [README.md](README.md) - Project overview
- [FEATURES.md](FEATURES.md) - Complete features list

**Common Commands**:
```bash
# Quick start
npm install && npm start

# Debug mode
DEBUG=* npm start

# Check logs
tail -f chat.log

# Monitor server
htop  # Linux/Mac
```

**Video Tutorial**: [Coming Soon]

**Report Issues**: Document bugs with:
1. Steps to reproduce
2. Expected vs actual behavior
3. Browser/OS versions
4. Console errors
5. Server logs

---

**Happy Chatting! 🎉**
