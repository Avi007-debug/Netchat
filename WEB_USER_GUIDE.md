# 📱 NetChat Web Version - User Guide

## 🚀 Getting Started

### 1. Access the Application
```bash
# Start the server
npm start

# Open browser
http://localhost:3000
```

### 2. Create Account
1. Click **"Sign Up"** on the homepage
2. Enter:
   - Username (3-30 characters)
   - Email address
   - Password (min 6 characters)
   - Confirm password
3. Click **Register**
4. You'll be automatically logged in

### 3. Login
1. Enter your email and password
2. Click **Login**
3. You'll be redirected to the chat interface

---

## 💬 Chat Features

### A. Chat Rooms

#### Join a Room
**Method 1:** Click on any room in the "Chat Rooms" list

**Method 2:** Create/Join new room
1. Type room name in the input box (e.g., "tech-talk")
2. Click **"Join/Create"**
3. If room exists → You join it
4. If room doesn't exist → New room is created

#### Current Room Info
- Look at the top of the chat area
- Shows: **Room Name · User Count · Message Count**
- Example: `general · 5 users · 23 messages`

#### Switch Rooms
- Simply click another room name in the sidebar
- You'll automatically leave current room and join the new one

---

### B. Sending Messages

#### Public Messages (to current room)
1. Make sure you've joined a room
2. Type your message in the input box at the bottom
3. Press **Enter** or click **Send**
4. Everyone in the room sees your message

#### Message Format
```
[19:45] [#general] YourName: Hello everyone!
         ↑         ↑         ↑
      timestamp   room    message
```

---

### C. Private Messaging (NEW!)

#### Method 1: Click to Chat (Easiest)
1. Look at **"Online Users"** in the sidebar
2. Find the user you want to message
3. Click the **💬 button** next to their name
4. A private chat window opens
5. Type your message and press Enter

#### Method 2: Command
In the main chat, type:
```
/pm <username> <your message>
```

**Example:**
```
/pm Alice Hey, how are you doing?
```

#### Private Chat Window
- **Header**: Shows who you're chatting with
- **Messages**: 
  - Your messages on the right (purple)
  - Their messages on the left (white)
- **Timestamps**: On each message
- **Close**: Click × or click outside the window

#### Privacy
✅ Only you and the recipient see these messages  
✅ Not visible in public rooms  
✅ Desktop notifications when you receive a PM  
✅ Message history saved per user  

---

### D. User List

#### What You See
```
🟢 YourName          ← You (green dot)
   📌 general        ← Current room

🔵 Alice             ← Other users (blue dot)
   📌 tech-talk      ← Their room
   [💬 Button]       ← Click to PM
```

#### Features
- **Green dot** (🟢) = You
- **Blue dot** (🔵) = Other online users
- **Room indicator** = Where they are currently
- **PM button** (💬) = Send private message

---

### E. Typing Indicator

When someone is typing:
```
📝 Alice is typing...
```

When multiple people are typing:
```
📝 Alice and Bob are typing...
📝 3 users are typing...
```

---

### F. System Messages

System messages appear in gray:
```
[Server]: Alice has joined #general
[Server]: Bob has left #general
```

---

## 🎯 All Commands Reference

| Command | What It Does | Example |
|---------|-------------|---------|
| Normal text | Send to current room | `Hello everyone!` |
| `/pm <user> <msg>` | Send private message | `/pm Alice Hi there!` |
| `/help` | Show help menu | `/help` |
| Click 💬 | Open private chat | Click button next to username |

---

## 🔒 Security Features

### 1. Single Session Enforcement
**What happens:**
- You can only be logged in from ONE location at a time
- If someone logs in with your credentials:
  - Your current session disconnects
  - You see: "⚠️ Your account is logged in from another location"
- You can reclaim your account by logging in again
  - Their session gets disconnected
  - You're back in control

**Why it matters:**
- Prevents account hijacking
- You always have control of your account
- Immediate notification of suspicious activity

### 2. Private Messages
- Direct socket-to-socket delivery
- No broadcasting to other users
- Cannot be seen in public rooms
- Encrypted in transit (WebSocket)

### 3. Authentication
- Passwords hashed with bcrypt
- JWT tokens (24-hour expiry)
- Automatic logout on token expiry
- Secure session management

---

## 🎨 Interface Tips

### Chat Messages
- **Your messages**: Right side, light purple background
- **Others' messages**: Left side, white background
- **System messages**: Center, gray background

### Responsive Design
- Works on desktop, tablet, and mobile
- Sidebar auto-adjusts on smaller screens
- Touch-friendly buttons

### Keyboard Shortcuts
- **Enter**: Send message (in any input box)
- **Shift+Enter**: Not supported yet (single line for now)

---

## 🐛 Troubleshooting

### "Not connected" or connection errors?
```bash
1. Check if server is running (npm start)
2. Refresh the page (F5)
3. Clear browser cache (Ctrl+Shift+Del)
4. Check console for errors (F12)
```

### Can't send messages?
```bash
✓ Make sure you've joined a room first
✓ Input box should NOT be disabled (grayed out)
✓ Check internet connection
```

### Private messages not working?
```bash
✓ User must be online (visible in users list)
✓ Check username spelling
✓ Click 💬 button to open chat window
✓ Type in the modal input, not main chat
```

### Disconnected unexpectedly?
```bash
Possible reasons:
1. Someone logged in with your credentials → Reclaim by logging in again
2. Token expired (24 hours) → Login again
3. Server restarted → Refresh page
4. Network issue → Check connection
```

---

## 📊 Example Session

```
1. Open http://localhost:3000
2. Register: username="Alice", email="alice@test.com", password="alice123"
3. You're auto-logged in and see the chat interface

4. Join a room:
   - Type "general" in room input
   - Click "Join/Create"
   - You're now in #general

5. Send a message:
   - Type "Hello everyone!"
   - Press Enter
   - Message appears: [19:45] [#general] Alice: Hello everyone!

6. Another user "Bob" joins:
   - System message: [Server]: Bob has joined #general
   - Bob appears in Online Users list

7. Send Bob a private message:
   - Click 💬 next to Bob's name
   - Private chat window opens
   - Type "Hey Bob, welcome!"
   - Press Enter
   - Message sent privately (only Bob sees it)

8. Bob replies:
   - You get a notification: "New PM from Bob"
   - His message appears in the private chat window

9. Switch rooms:
   - Type "tech-talk" in room input
   - Click "Join/Create"
   - You leave #general and join #tech-talk
   - System notifies both rooms

10. Logout:
    - Click "Logout" button
    - Redirected to login page
    - Your session ends
```

---

## 🎓 For Your Presentation/Viva

### Key Points to Highlight:

1. **Private Messaging**
   - Click-to-chat UI (easy to use)
   - Direct socket communication
   - No public broadcast
   - Desktop notifications

2. **Security**
   - Single session per user
   - Prevents account hijacking
   - User can reclaim account anytime
   - JWT authentication

3. **Real-time Features**
   - WebSocket for instant messaging
   - Typing indicators
   - Live user list
   - Room notifications

4. **OS Concepts**
   - Concurrency (session management)
   - IPC (WebSocket communication)
   - Resource management (connection tracking)
   - Event-driven architecture

### Demo Flow:
```
1. Show registration/login
2. Join a room, send public message
3. Open private chat with another user
4. Test duplicate login (security feature)
5. Show how real user can reclaim account
6. Demonstrate typing indicator
7. Switch between rooms
```

---

## 🚀 Quick Reference Card

```
╔════════════════════════════════════════════╗
║        NETCHAT WEB - QUICK GUIDE          ║
╠════════════════════════════════════════════╣
║                                            ║
║  🏠 JOIN ROOM                              ║
║     Type room name → Click "Join/Create"   ║
║                                            ║
║  💬 PUBLIC MESSAGE                         ║
║     Type in bottom box → Enter             ║
║                                            ║
║  💌 PRIVATE MESSAGE                        ║
║     Click 💬 next to username              ║
║     OR type: /pm username message          ║
║                                            ║
║  👥 SEE USERS                              ║
║     Check "Online Users" sidebar           ║
║                                            ║
║  🚪 LOGOUT                                 ║
║     Click "Logout" button (top right)      ║
║                                            ║
╚════════════════════════════════════════════╝
```

---

Need help? Check the console (F12) for error messages or contact support!
