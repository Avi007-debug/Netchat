# NetChat WebSocket - Real-time Chat Documentation

## 🚀 Overview

WebSocket functionality has been added to NetChat, enabling real-time instant messaging with the following features:

- **Real-time messaging** in chat rooms
- **Multiple chat rooms** support
- **User presence tracking** (who's online)
- **Typing indicators** (see when users are typing)
- **User status management** (online/offline)
- **Message history** per room
- **System messages** for room events

---

## 🛠️ Installation & Setup

### Step 1: Update Dependencies
```bash
npm install socket.io@^4.6.1
```

### Step 2: Start the Server
```bash
npm start
```

### Step 3: Access Chat
1. Navigate to `http://localhost:3000`
2. Register or login
3. After authentication, you'll be redirected to `/chat.html`
4. Join or create a chat room
5. Start messaging in real-time!

---

## 📡 WebSocket Architecture

### Connection Flow
```
Browser (Client)
    ↓
Connect with JWT Token
    ↓
Socket.io Server
    ↓
Verify Token (JWT Middleware)
    ↓
Connection Established ✓
    ↓
Listen for Events
    ↓
Emit Events to Server
```

---

## 🎯 Socket Events

### Client → Server Events

#### 1. **room:join** - Join a Chat Room
Join an existing room or create a new one.

```javascript
socket.emit('room:join', {
  roomName: 'general'
});
```

**Response:**
- Room info is broadcast to all users in the room
- System message: "User joined the room"
- User is added to room's user list

---

#### 2. **message:send** - Send a Message
Send a message to the current room.

```javascript
socket.emit('message:send', {
  message: 'Hello, everyone!',
  room: 'general'
});
```

**Parameters:**
- `message` (string): The message content
- `room` (string): Room name

**Response:**
- Message is stored in room history
- Message is broadcast to all users in room
- Message is added to UI in real-time

---

#### 3. **room:leave** - Leave Current Room
Leave the current room.

```javascript
socket.emit('room:leave');
```

**Response:**
- System message: "User left the room"
- User is removed from room's user list
- User can join another room

---

#### 4. **user:typing** - Typing Indicator
Notify others that user is typing.

```javascript
socket.emit('user:typing');
```

**Auto-sent:** Triggered when user starts typing in message input

**Response:**
- Typing indicator shown to other users
- Auto-clears after 3 seconds of inactivity

---

#### 5. **user:stopTyping** - Stop Typing
Notify others that user stopped typing.

```javascript
socket.emit('user:stopTyping');
```

**Response:**
- Typing indicator removed for this user

---

#### 6. **rooms:get** - Get Available Rooms
Request list of all available chat rooms.

```javascript
socket.emit('rooms:get');
```

**Response:**
```javascript
{
  rooms: [
    {
      name: 'general',
      users: 5,
      messages: 42
    },
    {
      name: 'random',
      users: 2,
      messages: 18
    }
  ]
}
```

---

#### 7. **room:getMessages** - Get Room Message History
Retrieve all previous messages in a room.

```javascript
socket.emit('room:getMessages', {
  room: 'general'
});
```

**Response:**
```javascript
{
  room: 'general',
  messages: [
    {
      id: '1234567890',
      userId: 'user123',
      username: 'alice',
      message: 'Hello!',
      timestamp: '2026-01-15T10:30:00Z',
      room: 'general',
      type: 'user'
    }
  ]
}
```

---

### Server → Client Events

#### 1. **message:new** - New Message Received
Broadcast when a new message is sent to a room.

```javascript
socket.on('message:new', (message) => {
  // message object
  console.log(`${message.username}: ${message.message}`);
});
```

**Message Object:**
```javascript
{
  id: '1234567890',
  userId: 'user123',
  username: 'alice',
  message: 'Hello!',
  timestamp: '2026-01-15T10:30:00Z',
  room: 'general',
  type: 'user' // or 'system'
}
```

---

#### 2. **room:info** - Room Information Update
Broadcast when room information changes.

```javascript
socket.on('room:info', (data) => {
  console.log(`Users in room: ${data.users}`);
  console.log(`Total messages: ${data.messageCount}`);
});
```

**Data Object:**
```javascript
{
  name: 'general',
  users: ['alice', 'bob', 'charlie'],
  messageCount: 42
}
```

---

#### 3. **rooms:list** - Available Rooms List
Sent in response to `rooms:get` event.

```javascript
socket.on('rooms:list', (data) => {
  console.log(data.rooms);
});
```

---

#### 4. **users:update** - Online Users Update
Broadcast when users connect/disconnect.

```javascript
socket.on('users:update', (users) => {
  console.log(`Online users: ${users.length}`);
});
```

**Users Array:**
```javascript
[
  {
    socketId: 'socket123',
    username: 'alice',
    email: 'alice@example.com',
    room: 'general',
    connectedAt: '2026-01-15T10:30:00Z'
  }
]
```

---

#### 5. **user:typing** - User Typing Indicator
Sent when another user starts typing.

```javascript
socket.on('user:typing', (data) => {
  console.log(`${data.username} is typing...`);
});
```

---

#### 6. **user:stopTyping** - User Stop Typing
Sent when another user stops typing.

```javascript
socket.on('user:stopTyping', (data) => {
  console.log(`${data.username} stopped typing`);
});
```

---

#### 7. **room:messages** - Room Message History
Sent in response to `room:getMessages` event.

```javascript
socket.on('room:messages', (data) => {
  console.log(`Room: ${data.room}`);
  console.log(`Messages: ${data.messages.length}`);
});
```

---

## 🔐 Authentication

### Connection with Token
```javascript
const socket = io({
  auth: {
    token: localStorage.getItem('token')
  }
});
```

### Token Verification
The server verifies JWT token on connection:
```javascript
io.use((socket, next) => {
  const token = socket.handshake.auth.token;
  
  try {
    const decoded = jwt.verify(token, JWT_SECRET);
    socket.userId = decoded.userId;
    socket.username = decoded.username;
    next();
  } catch (error) {
    next(new Error('Invalid token'));
  }
});
```

### Invalid Token Handling
If token is invalid or expired:
- Connection is rejected
- User is redirected to login page
- Client clears stored credentials

---

## 💻 API Endpoints Reference

### REST Endpoints (HTTP)
| Method | Endpoint | Auth | Purpose |
|--------|----------|------|---------|
| POST | `/api/auth/register` | ❌ | Create account |
| POST | `/api/auth/login` | ❌ | Login & get token |
| GET | `/api/auth/profile` | ✅ | Get user profile |
| POST | `/api/auth/logout` | ✅ | Logout |

### WebSocket Endpoints
| Event | Type | Auth | Purpose |
|-------|------|------|---------|
| `room:join` | Emit | ✅ | Join room |
| `room:leave` | Emit | ✅ | Leave room |
| `message:send` | Emit | ✅ | Send message |
| `user:typing` | Emit | ✅ | Typing indicator |
| `rooms:get` | Emit | ✅ | Get rooms list |
| `message:new` | Listen | ✅ | Receive message |
| `room:info` | Listen | ✅ | Room info update |
| `users:update` | Listen | ✅ | Users list update |

---

## 📁 File Structure

```
public/
├── index.html         # Login/Register page
├── chat.html          # Chat interface
├── styles.css         # Auth page styles
├── script.js          # Auth page logic
├── chat.css           # Chat page styles
└── chat.js            # Chat page logic + Socket.io client
```

---

## 🎨 Frontend UI Features

### Login/Register Page (`index.html`)
- ✅ User registration with validation
- ✅ User login with JWT token
- ✅ Redirect to chat after login

### Chat Page (`chat.html`)
- ✅ Real-time messaging
- ✅ Multiple chat rooms
- ✅ Online users list
- ✅ Typing indicators
- ✅ Message history
- ✅ Room information
- ✅ Responsive design

### Features
1. **Message Interface**
   - Real-time message sending/receiving
   - Message history per room
   - System messages for events
   - Timestamps for each message

2. **Room Management**
   - Create new rooms
   - Join existing rooms
   - Leave rooms
   - View room info (users, message count)

3. **User Presence**
   - See who's online
   - View user's current room
   - Connection status indicators

4. **Typing Indicators**
   - See when users are typing
   - Auto-hide after 3 seconds
   - Handles multiple typing users

5. **Responsive Design**
   - Works on desktop, tablet, mobile
   - Sidebar with rooms and users
   - Full-width chat area
   - Touch-friendly buttons

---

## 🔧 Configuration

### Server Configuration (server.js)
```javascript
// Socket.io Configuration
const io = socketIO(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

// JWT Settings
const JWT_SECRET = process.env.JWT_SECRET;
const TOKEN_EXPIRY = '24h';

// Storage
const connectedUsers = new Map();  // Track connected users
const chatRooms = new Map();        // Store room data
```

### Client Configuration (chat.js)
```javascript
// Connect with authentication
const socket = io({
  auth: {
    token: localStorage.getItem('token')
  }
});

// Auto-refresh rooms every 5 seconds
setInterval(() => {
  socket.emit('rooms:get');
}, 5000);
```

---

## 📊 Data Models

### Message Object
```javascript
{
  id: string,              // Unique message ID
  userId: string,          // User who sent message
  username: string,        // Username display
  message: string,         // Message content
  timestamp: ISO8601,      // When message was sent
  room: string,            // Room name
  type: 'user' | 'system'  // Message type
}
```

### User Object
```javascript
{
  socketId: string,        // Socket.io connection ID
  userId: string,          // User ID from JWT
  username: string,        // Username
  email: string,           // User email
  room: string | null,     // Current room
  connectedAt: ISO8601     // Connection timestamp
}
```

### Room Object
```javascript
{
  name: string,            // Room name
  createdAt: ISO8601,      // When room was created
  messages: Message[],     // Array of messages
  users: string[]          // Array of usernames in room
}
```

---

## 🧪 Testing WebSocket

### Using Browser DevTools
```javascript
// Check socket connection
console.log(socket.connected);  // true/false

// Manually emit events
socket.emit('room:join', { roomName: 'test' });

// Listen for events
socket.on('message:new', (msg) => {
  console.log('New message:', msg);
});
```

### Testing Message Flow
1. Open chat in two browser windows
2. Login as different users
3. Join same room
4. Send message from first window
5. Message appears in real-time in second window

### Testing Typing Indicators
1. Open chat in two browser windows
2. Join same room
3. Start typing in message input
4. "User is typing..." appears in second window
5. Stop typing
6. Indicator disappears after 3 seconds

---

## 🚀 Production Considerations

### Scaling
- **In-memory storage**: Currently uses Map objects
  - For production: Use Redis for distributed sessions
  - Use database for persistent message storage

- **Multiple servers**: Socket.io requires session affinity
  - Use Socket.io-redis adapter for multi-server setup
  - Configure sticky sessions on load balancer

### Performance
- **Message rate limiting**: Add rate limiting per user
- **Room size limits**: Limit users per room to prevent memory issues
- **Message history**: Store only recent messages in memory

### Security
- **Message validation**: Sanitize all user inputs
- **Rate limiting**: Prevent message spam
- **Token refresh**: Implement refresh tokens for long sessions
- **Audit logging**: Log important events for security

### Monitoring
- **Connection metrics**: Track connected users, active rooms
- **Message throughput**: Monitor messages per second
- **Error tracking**: Log socket errors for debugging
- **Performance metrics**: Track latency, CPU, memory usage

---

## 📝 Example Implementation

### Basic Chat Flow
```javascript
// 1. User logs in and is redirected to chat.html
// 2. Socket connects with JWT token
socket.on('connect', () => {
  console.log('Connected to chat server');
});

// 3. User joins a room
socket.emit('room:join', { roomName: 'general' });

// 4. Room info is received
socket.on('room:info', (data) => {
  console.log(`Joined ${data.name} with ${data.users.length} users`);
});

// 5. User sends a message
socket.emit('message:send', {
  message: 'Hello everyone!',
  room: 'general'
});

// 6. Message is broadcast to all users
socket.on('message:new', (message) => {
  if (message.type === 'user') {
    console.log(`${message.username}: ${message.message}`);
  }
});

// 7. When user types
socket.emit('user:typing');

// 8. Other users see typing indicator
socket.on('user:typing', (data) => {
  console.log(`${data.username} is typing...`);
});
```

---

## 🐛 Troubleshooting

### Connection Issues
**Problem:** "Cannot connect to socket"
```
Solution: 
1. Check server is running (npm start)
2. Verify token is valid in localStorage
3. Check browser console for errors
```

**Problem:** "Authentication failed"
```
Solution:
1. Clear localStorage
2. Login again to get fresh token
3. Check server logs for JWT errors
```

### Message Not Sending
**Problem:** "Message not received by others"
```
Solution:
1. Verify you're in a room (room info should show)
2. Check message content is not empty
3. Ensure socket connection is active
4. Check server logs for errors
```

### Typing Indicator Issues
**Problem:** "Typing indicator not showing"
```
Solution:
1. Check other user is in same room
2. Verify socket events are emitted/received
3. Check browser console for JavaScript errors
```

---

## 📚 Learning Resources

- [Socket.io Documentation](https://socket.io/docs/)
- [WebSocket Protocol](https://tools.ietf.org/html/rfc6455)
- [Real-time Communication Patterns](https://developer.mozilla.org/en-US/docs/Web/API/WebSocket)

---

## 🎉 Summary

NetChat now includes **real-time chat functionality** with:

✅ **Instant messaging** in multiple rooms  
✅ **User presence tracking** (online/offline)  
✅ **Typing indicators** (see when users type)  
✅ **Message history** (view past messages)  
✅ **System messages** (join/leave events)  
✅ **Responsive UI** (works on all devices)  
✅ **JWT authentication** (secure connection)  
✅ **Error handling** (graceful failure handling)  

---

**Ready for real-time chatting! 🚀**
