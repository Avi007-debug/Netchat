# 📋 NetChat - Complete Features Documentation

This document provides a comprehensive list of ALL features in NetChat, from major functionalities to small UI details.

---

## 🎯 Core Chat Features

### 1. Multi-Room Support
- **Create Rooms**: Any user can create a new room by joining it
- **Join Rooms**: `/join <roomname>` command or UI button
- **Switch Rooms**: Leave current room and join another seamlessly
- **Room List**: View all active rooms with user/message counts
- **Current Room Indicator**: Always know which room you're in
- **Default Room**: All users start in `#general`
- **Room Isolation**: Messages only visible to users in same room

### 2. Real-time Messaging
- **Instant Delivery**: WebSocket for real-time message delivery
- **Message Broadcasting**: All users in room receive messages simultaneously
- **Sender Identification**: Each message shows username
- **Timestamp**: All messages timestamped with [HH:MM:SS]
- **Message History**: Persistent storage, viewable on rejoin
- **Room Context**: Messages tagged with room name
- **System Messages**: Join/leave notifications

### 3. User Authentication & Registration
- **Registration**: New users auto-registered on first login
- **Login**: Username + password authentication
- **Password Hashing**: Bcrypt (10 rounds) for web, plaintext for C server
- **Token-based Auth**: JWT with 24-hour expiry (web only)
- **File-based Storage**: 
  - C server: `users.txt` (username:password format)
  - Web server: `users.json` (JSON objects)
- **Validation**: 
  - Username: 3-30 characters
  - Password: Min 6 characters
  - Email: Valid format (web only)

### 4. Online User List
- **Real-time Updates**: User list updates on join/leave
- **Username Display**: All connected users shown
- **Room Filtering**: See users in current room only (`/users` command)
- **Status Indicators**: Visual indication of online status
- **Clickable Users**: Click 💬 button for private chat (web only)
- **User Count**: Total users shown in room info

### 5. Typing Indicators
- **Real-time Typing**: See when others are typing
- **Multi-user Support**: Shows all users currently typing
- **Auto-timeout**: Typing indicator clears after 3 seconds
- **Visual Feedback**: "User1, User2 are typing..." display
- **Non-intrusive**: Appears below message area

---

## 💬 Private Messaging System

### Click-to-Chat UI (Web - Primary Method)
- **💬 Button**: Next to each username in users list
- **Dedicated Modal**: Private chat window overlays main chat
- **Message History**: Stores PM history per user (localStorage)
- **Visual Distinction**: 
  - Your messages: Purple background
  - Their messages: White background
- **Desktop Notifications**: Browser notifications for new PMs
- **Timestamps**: Each PM shows time sent
- **Close Options**: × button or click outside modal
- **Unread Counts**: Badge shows unread PM count per user
- **Toast Notifications**: In-page alerts for new PMs
- **Persistent Storage**: PM history survives page refresh

### Command-based (C Server & Web)
- **Syntax**: `/pm <username> <message>`
- **Direct Delivery**: Socket-to-socket, no room broadcasting
- **Confirmation**: Sender sees their sent PM
- **Error Handling**: "User not found" if recipient offline
- **Logging**: All PMs logged to chat.log
- **Privacy**: Only sender and recipient see messages

---

## 🎮 C Server Commands Quick Reference

### Room Management Commands
- **`/join <roomname>`** - Switch to or create a new room
  - Auto-creates room if it doesn't exist
  - Notifies users of join/leave events
  - Max 5 concurrent rooms supported
  
- **`/room`** - Show your current room
  - Returns: "You are currently in room #roomname"
  
- **`/rooms`** - List all active rooms with user counts
  - Format: `• #roomname (N users)`
  - Real-time updates as users join/leave
  
- **`/users`** - List users in your current room
  - Format: `• username`
  - Shows only room members

### Messaging Commands
- **`/pm <username> <message>`** - Send private message
  - Queued for offline delivery (enhanced server)
  - Example: `/pm alice Hello there!`
  
- **`/recent`** - View recent messages (Enhanced Server Only)
  - Shows last 20 messages from shared memory
  - Access full chat history

### Utility Commands
- **`/help`** - Display command menu
- **`/quit`** - Disconnect gracefully

### Message Format
All messages include timestamp and room prefix:
```
[HH:MM:SS] [#roomname] username: message
```

### Implementation Details
- **No Room Broadcast**: PMs never appear in public chat
- **Client-side Storage**: Web client stores PM history locally
- **Real-time Delivery**: Instant delivery when both users online
- **Offline Queuing**: Messages queued for offline users (enhanced C server)

---

## 📷 Image Sharing Feature

### User Workflow
1. Click **📷 camera button** in chat interface
2. File picker opens for image selection
3. **(Optional)** Type caption in message box
4. Image auto-uploads and sends to room
5. Appears as clickable thumbnail (max 300px × 300px)
6. Click thumbnail for full-screen modal view
7. Close modal with × or click outside

### Technical Specifications
- **Supported Formats**: JPEG, JPG, PNG, GIF (animated), WebP
- **Max File Size**: 5MB per image
- **Upload Method**: Multer middleware (multipart/form-data)
- **Authentication**: JWT token required for upload
- **Storage**: `/public/uploads/` directory
- **Filename**: Timestamped unique (prevents collisions)
- **URL Structure**: `/uploads/image-{timestamp}-{random}.{ext}`

### Client-Side Features
- **File Validation**: Size and type checked before upload
- **Progress Indicator**: Button shows ⏳ during upload
- **Error Messages**: Clear feedback for invalid files
- **Success Notification**: Toast message on successful upload
- **Thumbnail Display**: Auto-sized, centered images
- **Hover Effect**: Subtle scale animation on hover
- **Caption Support**: Optional text below image

### Server-Side Features
- **Double Validation**: File type and size verified server-side
- **Secure Upload**: JWT authentication required
- **Error Handling**: Graceful multer error handling
- **File Size Limit**: Hard 5MB limit enforced
- **MIME Type Check**: Only image/* allowed
- **Directory Auto-creation**: Uploads folder created if missing

### Full-Screen Image Viewer
- **Modal Overlay**: Black background (90% opacity)
- **Centered Image**: Max 90vh/90vw sizing
- **Close Button**: × in top-right corner
- **Click Outside**: Close on background click
- **Smooth Animations**: Fade in (300ms)
- **Keyboard Support**: (Future: ESC to close)

### Error Handling
| Error | Message | Action |
|-------|---------|--------|
| File > 5MB | "Image too large. Maximum size is 5MB." | Upload rejected |
| Wrong type | "Invalid file type. Only JPEG, PNG, GIF, and WebP are allowed." | Upload rejected |
| Not authenticated | "Authentication required" | Redirect to login |
| Server error | "Failed to upload image" | Retry prompt |
| Not in room | Button disabled | Must join room first |

---

## 🔐 Message Encryption Feature

### Encryption Toggle
- **🔓/🔐 Button**: Next to message input
- **Visual Indicator**: 
  - 🔓 = Encryption OFF (default)
  - 🔐 = Encryption ON
- **Tooltip**: Shows current state
- **Per-Message**: Toggle on/off per message
- **Color Coding**: Encrypted messages have lock icon

### Technical Implementation
- **Algorithm**: AES-256-CBC
- **Key Derivation**: Scrypt (password-based)
- **IV Generation**: Random 16-byte IV per message
- **Storage Format**: `{iv}:{encrypted_hex}`
- **Server-side**: Encryption/decryption on server
- **Client-side**: Toggle and display logic

### Message Flow
1. User types message
2. Clicks encryption toggle 🔐
3. Message sent with `encrypted: true` flag
4. Server encrypts with AES-256-CBC
5. Encrypted message broadcast to room
6. Recipients see 🔐 icon and encrypted display
7. Click message to decrypt (authenticated users only)

### Decryption
- **On-Demand**: Messages stay encrypted until clicked
- **Authentication Required**: Must be room member to decrypt
- **Endpoint**: `POST /api/decrypt` with encrypted message
- **Display**: Decrypted text replaces encrypted text
- **Re-encryption**: Refresh page re-encrypts messages

### Security Features
- **Key Storage**: Server-side environment variable
- **No Client Storage**: Keys never exposed to client
- **HTTPS Required**: (Production deployment)
- **Token Validation**: JWT required for decrypt endpoint

---

## 🔒 Security Features

### 1. Duplicate Session Prevention (Anti-Hijacking)
**Problem**: What if attacker logs in with stolen credentials?

**Solution**: Single Session Enforcement
- **One Session Per User**: Only ONE active login allowed globally
- **Latest Login Wins**: New login disconnects old session
- **Immediate Notification**: Disconnected user sees alert
- **User Can Reclaim**: Real user can always log back in to kick out attacker
- **Audit Trail**: Server logs all duplicate login attempts
- **Socket Tracking**: Server maintains userId → socketId map

**Implementation**:
```javascript
activeSessions.set(userId, newSocketId);
if (existingSocket) {
  existingSocket.emit('session:duplicate');
  existingSocket.disconnect();
}
```

**User Experience**:
1. Alice logs in from office ✅
2. Attacker logs in with Alice's password ⚠️
3. Alice gets alert: "Account logged in from another location"
4. Alice immediately logs back in 🔄
5. Attacker gets disconnected 🚫

### 2. Password Security
- **Bcrypt Hashing**: 10 salt rounds
- **No Plaintext Storage**: Passwords never stored as plaintext (web)
- **Secure Comparison**: bcrypt.compare() for validation
- **Hash Verification**: Only hashes compared, never plaintext

### 3. Token Management
- **JWT Tokens**: JSON Web Tokens for stateless auth
- **24-Hour Expiry**: Tokens expire after 24 hours
- **Bearer Authentication**: Authorization: Bearer {token}
- **Auto-Redirect**: Expired tokens redirect to login
- **LocalStorage**: Secure client-side token storage
- **No Cookies**: Token-based, not session cookies

### 4. Input Validation & Sanitization
- **HTML Escaping**: All user input escaped (XSS prevention)
- **Username Validation**: 3-30 chars, alphanumeric
- **Email Validation**: Valid email format required
- **Password Strength**: Min 6 characters (configurable)
- **File Type Validation**: Only allowed image types
- **File Size Validation**: Max 5MB enforced
- **Path Traversal Prevention**: Filename sanitization
- **SQL Injection**: N/A (file-based storage)

### 5. Rate Limiting (Future)
- Connection throttling
- Message rate limiting
- Upload rate limiting

---

## 🎨 UI/UX Features

### 1. Responsive Design
- **Mobile**: Optimized for touch screens
- **Tablet**: Adaptive layout
- **Desktop**: Full-featured experience
- **Breakpoints**: CSS media queries
- **Touch-friendly**: Large tap targets

### 2. Modern Interface
- **Gradient Header**: Purple gradient (#667eea → #764ba2)
- **Card Layout**: Elevated chat containers
- **Rounded Corners**: Modern 8-12px border radius
- **Shadows**: Subtle box-shadows for depth
- **Smooth Animations**: CSS transitions (0.3s ease)
- **Hover Effects**: Interactive button feedback

### 3. Toast Notifications
- **Position**: Top-right corner
- **Auto-dismiss**: 6-second timeout
- **Dismissible**: × button to close
- **Pause on Hover**: Timer pauses on mouse over
- **Types**: Success (✅), Error (❌), Info (ℹ️)
- **Character Limit**: Truncated at 120 chars

### 4. Modal Windows
- **Private Chat Modal**: Dedicated PM window
- **Image Viewer Modal**: Full-screen image display
- **Backdrop**: Semi-transparent overlay
- **Close Options**: × button, click outside, ESC key
- **Smooth Animations**: Fade + scale effects
- **Z-index Management**: Proper stacking

### 5. Loading Indicators
- **Upload Progress**: ⏳ icon during upload
- **Message Loading**: "Loading messages..." text
- **Room Loading**: Spinner during room switch
- **Connection Status**: Visual connection indicator

### 6. Keyboard Shortcuts
- **Enter**: Send message
- **Shift+Enter**: New line (future)
- **ESC**: Close modals (future)
- **Ctrl+/**: Show shortcuts (future)

### 7. Auto-scroll
- **New Messages**: Auto-scroll to bottom
- **Smooth Behavior**: CSS scroll-behavior: smooth
- **Manual Scroll**: User can scroll up to view history
- **Smart Scroll**: Only auto-scroll if at bottom

### 8. Visual Feedback
- **Button Hover**: Scale + shadow effect
- **Active States**: Pressed button styling
- **Disabled States**: Grayed out, not clickable
- **Success States**: Green checkmark
- **Error States**: Red X icon

---

## 💻 C Server Commands

Complete command reference for C server clients:

### Help & Information
- **`/help`** - Display full command menu with descriptions
- **`/room`** - Show current room name
- **`/rooms`** - List all active rooms with user counts
- **`/users`** - Show all users in current room
- **`/recent`** - Display recent messages from shared memory (enhanced server)

### Room Management
- **`/join <roomname>`** - Join existing room or create new one
  - Example: `/join gaming`
  - Notifies old and new room of your move
  - Automatic room creation if doesn't exist

### Messaging
- **`<message>`** - Send message to current room
  - Just type normally and press Enter
  - Broadcast to all users in your room
- **`/pm <username> <message>`** - Send private message
  - Example: `/pm Alice Hey there!`
  - Direct user-to-user, no room broadcast
  - Shows confirmation to sender

---

## 📁 File System Features

### 1. User Database
**C Server** (`users.txt`):
```
alice:password123
bob:securepass456
charlie:qwerty789
```
- Format: `username:password\n`
- Newline-separated entries
- No encryption (educational project)
- Auto-created on first registration

**Web Server** (`users.json`):
```json
[
  {
    "id": "1675234567890",
    "username": "alice",
    "email": "alice@example.com",
    "password": "$2a$10$...",
    "createdAt": "2026-02-01T10:00:00.000Z",
    "status": "online"
  }
]
```
- JSON array of user objects
- Bcrypt hashed passwords
- Timestamps and status tracking

### 2. Chat Logging (`chat.log`)
- All messages logged with timestamps
- Join/leave events recorded
- Private messages logged (metadata only)
- File I/O with mutex protection
- Auto-flush for real-time logging
- Persistent across restarts

### 3. Image Storage (`public/uploads/`)
- Auto-created on server start
- Timestamped unique filenames
- Public accessibility via HTTP
- File cleanup (manual or scheduled)
- Organized by date (future)

---

## 🔄 Session Management

### 1. Persistent Login
- **LocalStorage**: Token stored client-side
- **Auto-login**: Valid token bypasses login
- **Session Persistence**: Survives page refresh
- **Logout**: Clears token and redirects

### 2. Connection Management
- **Auto-reconnect**: Socket.IO automatic reconnection
- **Graceful Disconnect**: Clean socket closure
- **State Tracking**: Connection status displayed
- **Error Handling**: Connection errors handled gracefully

### 3. Room State
- **Current Room**: Stored in server memory
- **Room Persistence**: Room state maintained
- **User Tracking**: Active users per room
- **Message History**: Per-room message storage

---

## 🛠️ Advanced OS Features (Enhanced C Server)

### 1. Shared Memory (IPC)
- **shmget()**: Allocate shared memory segment
- **shmat()**: Attach to process memory
- **Shared Buffer**: Recent 20 messages stored
- **Cross-Process**: Multiple processes can read
- **Mutex Protection**: Thread-safe access
- **Command**: `/recent` shows shared memory contents

### 2. Message Queues (POSIX)
- **mq_open()**: Create message queue
- **mq_send()**: Queue messages
- **mq_receive()**: Retrieve messages
- **Priority Support**: 0 = normal, 1 = urgent (PMs)
- **Offline Delivery**: Messages queued for offline users
- **Auto-delivery**: Delivered on user login

### 3. Process Forking
- **fork()**: Separate process per client
- **Process Isolation**: Independent memory space
- **Zombie Prevention**: SIGCHLD handling
- **Process IDs**: Tracked and logged
- **Resource Cleanup**: Proper wait() calls

### 4. Semaphores
- **sem_open()**: Named semaphore creation
- **sem_wait()**: Acquire connection slot
- **sem_post()**: Release connection slot
- **Max Connections**: Enforced via semaphore
- **Deadlock Prevention**: Proper cleanup

---

## 📊 Small But Important Features

### 1. Welcome Messages
- Formatted banner on login
- OS feature indicators (enhanced server)
- Command quick-reference
- Process ID display (forked server)

### 2. Message Metadata
- **Timestamps**: Every message timestamped
- **Usernames**: Sender always identified
- **Room Tags**: Messages tagged with room
- **Message IDs**: Unique ID per message

### 3. Error Messages
- **User-friendly**: Clear, actionable messages
- **Specific**: Exact problem described
- **Helpful**: Suggestions for resolution
- **Non-technical**: Avoid jargon

### 4. Success Feedback
- **Upload Success**: Green checkmark + toast
- **Login Success**: Welcome message
- **Room Join**: Confirmation message
- **PM Sent**: Delivery confirmation

### 5. Resource Indicators
- **Room Count**: Number in room info
- **Message Count**: Per-room message count
- **User Count**: Online users count
- **Upload Size**: File size shown (future)

### 6. Graceful Degradation
- **No JavaScript**: Basic HTML fallback (partial)
- **Slow Network**: Loading indicators
- **Connection Loss**: Reconnection attempts
- **Server Down**: Error page

---

## 🎯 Feature Matrix

| Feature | C Server | Web Server | Priority |
|---------|----------|------------|----------|
| Real-time Chat | ✅ | ✅ | High |
| Multi-threading | ✅ | ✅ (Node.js) | High |
| Authentication | ✅ | ✅ | High |
| Chat Rooms | ✅ | ✅ | High |
| Private Messages | ✅ | ✅ | High |
| Image Sharing | ❌ | ✅ | Medium |
| Encryption | ❌ | ✅ | Medium |
| Shared Memory | ✅ | ❌ | High (OS) |
| Message Queues | ✅ | ❌ | High (OS) |
| Process Forking | ✅ | ❌ | High (OS) |
| Semaphores | ✅ | ❌ | High (OS) |
| Web UI | ❌ | ✅ | Medium |
| Mobile Support | ❌ | ✅ | Low |

---

## 🚀 Feature Implementation Timeline

**Phase 1 (Completed)**:
- ✅ Basic chat with TCP sockets
- ✅ Multi-threading
- ✅ Authentication
- ✅ Chat rooms

**Phase 2 (Completed)**:
- ✅ Private messaging
- ✅ Web server with WebSockets
- ✅ Modern UI

**Phase 3 (Completed)**:
- ✅ Image sharing
- ✅ Session management
- ✅ JWT authentication

**Phase 4 (Completed)**:
- ✅ Shared memory (IPC)
- ✅ Message queues
- ✅ Process forking
- ✅ Semaphores
- ✅ Message encryption

**Future Phases**:
- ⏳ Video calling
- ⏳ File sharing (non-images)
- ⏳ Voice messages
- ⏳ Read receipts
- ⏳ Typing indicators for PMs
- ⏳ Message reactions
- ⏳ User profiles
- ⏳ Admin panel

---

**Total Features**: 100+ individual features and capabilities!

**See [SETUP.md](SETUP.md) for usage instructions and [README.md](README.md) for project overview.**
