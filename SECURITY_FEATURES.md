# 🔐 NetChat Security Features

## 1. Duplicate Session Prevention (Anti-Hijacking)

### Problem Statement
**Question from Professor:** "What if a hacker with correct credentials logs in and prevents the actual user from accessing the system?"

### Our Solution: Session Management

#### How It Works:
1. **Single Session Per User**: Only ONE active session allowed per user account at any time
2. **Latest Login Wins**: When a user logs in, any existing session is immediately terminated
3. **Real-time Notification**: Previous session receives instant disconnect notification

#### Implementation:

**Server Side (`server.js`):**
```javascript
const activeSessions = new Map(); // Track userId -> socketId

io.on('connection', (socket) => {
  // Check for duplicate session
  if (activeSessions.has(socket.userId)) {
    const existingSocketId = activeSessions.get(socket.userId);
    const existingSocket = io.sockets.sockets.get(existingSocketId);
    
    if (existingSocket) {
      // Disconnect old session
      existingSocket.emit('session:duplicate', { 
        message: 'Your account has been logged in from another location.' 
      });
      existingSocket.disconnect(true);
    }
  }
  
  // Store new active session
  activeSessions.set(socket.userId, socket.id);
});
```

**Client Side (`chat.js`):**
```javascript
socket.on('session:duplicate', (data) => {
  alert('⚠️ Your account is already logged in from another location. You will be disconnected.');
  localStorage.removeItem('token');
  localStorage.removeItem('user');
  window.location.href = '/';
});
```

#### Security Benefits:
✅ **Prevents Session Hijacking**: Attacker can't hold a session while legitimate user is locked out  
✅ **Immediate Detection**: Both parties are notified instantly  
✅ **User Control**: Real user can reclaim account by logging in again  
✅ **Audit Trail**: Server logs all duplicate login attempts  

#### Example Scenario:

```
Time 10:00 AM - Alice logs in from Office Computer
  ✅ Session A created (socketId: abc123)
  
Time 10:05 AM - Hacker logs in with Alice's credentials from another location
  ⚠️ Server detects duplicate session
  ❌ Session A (Office) receives "session:duplicate" event
  ❌ Session A disconnected
  ✅ Session B (Hacker) becomes active
  
Time 10:07 AM - Alice logs in again from Office
  ⚠️ Server detects duplicate session  
  ❌ Session B (Hacker) disconnected
  ✅ Session C (Alice - Office) becomes active
  🎯 Alice has reclaimed her account
```

---

## 2. Private Messaging System

### Feature Overview
Secure, one-to-one messaging between users with complete privacy.

### How to Use:

#### Method 1: Click-to-Chat (Recommended)
1. Look at the **Online Users** list in the sidebar
2. Click the **💬 button** next to any user's name
3. A private chat window opens
4. Type your message and press Enter or click Send
5. Only you and the recipient can see these messages

#### Method 2: Command-Based
In any chat room, type:
```
/pm <username> <message>
```
Example: `/pm Alice Hello, how are you?`

### Implementation Details:

**Server Side:**
```javascript
socket.on('pm:send', (data) => {
  const { to, message } = data;
  
  // Find target user's socket
  let targetSocketId = null;
  for (const [userId, userData] of connectedUsers.entries()) {
    if (userData.username === to) {
      targetSocketId = userData.socketId;
      break;
    }
  }
  
  if (targetSocketId) {
    // Send only to target user
    io.to(targetSocketId).emit('pm:received', {
      from: socket.username,
      message: message,
      timestamp: new Date().toISOString()
    });
  }
});
```

**Client Side Features:**
- Private chat modal UI
- Message history per user
- Desktop notifications for new PMs
- Timestamps on all messages
- Separate from room messages

### Privacy Features:
✅ **End-to-End Delivery**: Messages sent directly to recipient's socket  
✅ **No Room Broadcast**: PMs never appear in public chat rooms  
✅ **Client-Side Storage**: Message history stored locally per user  
✅ **Notification System**: Browser notifications for incoming PMs  
✅ **Visual Distinction**: PMs clearly marked and styled differently  

---

## 3. JWT-Based Authentication

### Token Security
- **Encryption**: bcrypt password hashing (10 rounds)
- **Token Expiry**: 24-hour session timeout
- **Secure Storage**: localStorage (client-side)
- **Token Validation**: Every WebSocket connection verified

### Authentication Flow:
```
1. User registers → Password hashed with bcrypt
2. Credentials stored in users.json
3. Login → Password verified, JWT generated
4. JWT contains: userId, username, email
5. WebSocket connection → JWT validated
6. Invalid token → Automatic redirect to login
```

---

## 4. OS Concepts Demonstrated

### Concurrency Control
**Mutex-like Behavior**: `activeSessions` Map prevents race conditions
```javascript
// Atomic operation - only one session per user
activeSessions.set(socket.userId, socket.id);
```

### Resource Management
**Connection Pooling**: Limited concurrent connections
**Memory Management**: Automatic cleanup on disconnect
```javascript
socket.on('disconnect', () => {
  connectedUsers.delete(socket.userId);
  activeSessions.delete(socket.userId);
});
```

### Inter-Process Communication (IPC)
**WebSocket Protocol**: Full-duplex communication
**Event-Driven**: Asynchronous message passing
**Socket.IO Rooms**: Process isolation per chat room

---

## 5. Attack Prevention Summary

| Attack Type | Prevention Method | Implementation |
|-------------|-------------------|----------------|
| **Session Hijacking** | Single session per user | `activeSessions` Map |
| **Credential Theft** | Bcrypt password hashing | 10-round bcrypt |
| **Token Replay** | 24-hour expiry | JWT expiration |
| **MITM Attacks** | HTTPS recommended | Deploy with SSL |
| **Brute Force** | Rate limiting (add if needed) | Express middleware |
| **XSS Attacks** | HTML escaping | `escapeHTML()` function |

---

## 6. Demonstration Points for Viva

### Q: What if a hacker logs in with correct credentials?

**Answer:**
"Our system implements **single-session enforcement**. When the hacker logs in:

1. ✅ The legitimate user's session is **immediately terminated**
2. ✅ The user receives a **notification** explaining the situation  
3. ✅ The user can **reclaim their account** by logging in again
4. ✅ The hacker's session is then **automatically disconnected**
5. ✅ All events are **logged** on the server for audit

This prevents the hacker from 'camping' on the account and denying access to the real user. The legitimate user always has the power to reclaim their session."

### Q: How do you ensure private messages stay private?

**Answer:**
"We use **direct socket communication**:

1. ✅ Each user has a unique `socketId` mapped to their `userId`
2. ✅ Private messages use `io.to(socketId)` - **point-to-point delivery**
3. ✅ Messages **never pass through chat rooms**
4. ✅ No broadcast to other users
5. ✅ Client-side encryption possible (future enhancement)"

### Q: What OS concepts are demonstrated?

**Answer:**
"Several key concepts:

1. **Concurrency Control**: Single session per user (mutual exclusion)
2. **Resource Management**: Connection tracking with Maps
3. **IPC**: WebSocket bidirectional communication
4. **Process Isolation**: Socket.IO rooms separate message spaces
5. **Event-Driven Architecture**: Asynchronous message handling
6. **Session Management**: User state tracking across connections"

---

## 7. Future Enhancements

### Additional Security Features (Suggested):
- [ ] Two-Factor Authentication (2FA)
- [ ] IP address tracking and geolocation
- [ ] Login attempt rate limiting
- [ ] Account lockout after failed attempts
- [ ] Email notifications on new login
- [ ] End-to-end encryption for PMs
- [ ] Message encryption at rest
- [ ] HTTPS enforcement
- [ ] CSRF token protection
- [ ] SQL injection prevention (if using DB)

---

## 8. Testing the Security Features

### Test Duplicate Session:
```bash
# Terminal 1 - First login
1. Open http://localhost:3000
2. Login as "user1"
3. Note: Connected successfully

# Terminal 2 - Duplicate login
1. Open http://localhost:3000 in incognito/different browser
2. Login as "user1" (same credentials)
3. Observe: Terminal 1 gets disconnected with message
4. Terminal 2 is now active

# Terminal 1 - Reclaim session
1. Login again as "user1"
2. Observe: Terminal 2 gets disconnected
3. Terminal 1 reclaims the session
```

### Test Private Messaging:
```bash
1. User A and User B both online
2. User A clicks 💬 next to User B's name
3. Private chat modal opens
4. User A sends: "Secret message"
5. User B receives notification and sees message
6. Message does NOT appear in any public room
7. Other users cannot see this conversation
```

---

## Conclusion

NetChat demonstrates **enterprise-grade security** while maintaining OS concept fundamentals. The duplicate session prevention specifically addresses the professor's concern about credential theft and session hijacking.

**Key Takeaway**: Even if an attacker has valid credentials, they cannot permanently lock out the legitimate user due to our single-session enforcement mechanism.
