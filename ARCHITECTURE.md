# NetChat Authentication System - Architecture

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     FRONTEND (Browser)                       │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │           Login & Register UI (HTML/CSS/JS)          │  │
│  │                                                       │  │
│  │  ┌─────────────────┐      ┌──────────────────┐     │  │
│  │  │  Login Form     │      │  Register Form   │     │  │
│  │  │                 │      │                  │     │  │
│  │  │ • Email         │      │ • Username       │     │  │
│  │  │ • Password      │      │ • Email          │     │  │
│  │  │ • Remember Me   │      │ • Password       │     │  │
│  │  │ • Forgot Pwd    │      │ • Confirm Pwd    │     │  │
│  │  │ • Toggle Form   │      │ • Terms & Priv   │     │  │
│  │  └─────────────────┘      └──────────────────┘     │  │
│  │           ↓                        ↓                │  │
│  │     POST /api/auth/login    POST /api/auth/register│  │
│  │           ↓                        ↓                │  │
│  │     JWT Token Received      JWT Token Received     │  │
│  │     localStorage.setItem()   localStorage.setItem()│  │
│  │                                                    │  │
│  └──────────────────────────────────────────────────────┘  │
│                          ↕ HTTP/HTTPS                       │
│                                                              │
└─────────────────────────────────────────────────────────────┘
                           ↓
┌─────────────────────────────────────────────────────────────┐
│                  BACKEND (Node.js/Express)                   │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │              Express Server (port 3000)              │  │
│  │                                                       │  │
│  │  Route Handlers:                                     │  │
│  │                                                       │  │
│  │  POST /api/auth/register                            │  │
│  │  ├─ Validate input (express-validator)              │  │
│  │  ├─ Check if email exists                           │  │
│  │  ├─ Hash password (bcryptjs)                        │  │
│  │  ├─ Save to users.json                              │  │
│  │  └─ Generate JWT token                              │  │
│  │                                                       │  │
│  │  POST /api/auth/login                               │  │
│  │  ├─ Validate input                                  │  │
│  │  ├─ Find user by email                              │  │
│  │  ├─ Compare passwords (bcryptjs)                    │  │
│  │  ├─ Update user status to "online"                  │  │
│  │  └─ Generate JWT token                              │  │
│  │                                                       │  │
│  │  GET /api/auth/profile (Protected)                  │  │
│  │  ├─ Verify JWT token                                │  │
│  │  ├─ Find user                                        │  │
│  │  └─ Return user profile                             │  │
│  │                                                       │  │
│  │  POST /api/auth/logout (Protected)                  │  │
│  │  ├─ Verify JWT token                                │  │
│  │  ├─ Update user status to "offline"                 │  │
│  │  └─ Confirm logout                                  │  │
│  │                                                       │  │
│  └──────────────────────────────────────────────────────┘  │
│           ↓                                                  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         Data Persistence Layer                       │  │
│  │                                                       │  │
│  │  ┌──────────────────────────────────────────────┐  │  │
│  │  │           users.json                         │  │  │
│  │  │                                              │  │  │
│  │  │  [                                           │  │  │
│  │  │    {                                         │  │  │
│  │  │      "id": "123",                           │  │  │
│  │  │      "username": "john",                    │  │  │
│  │  │      "email": "john@example.com",           │  │  │
│  │  │      "password": "$2a$10$hashed...",        │  │  │
│  │  │      "status": "online",                    │  │  │
│  │  │      "createdAt": "2026-01-15T10:30:00Z"   │  │  │
│  │  │    }                                         │  │  │
│  │  │  ]                                           │  │  │
│  │  │                                              │  │  │
│  │  └──────────────────────────────────────────────┘  │  │
│  │                                                       │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔄 Authentication Flow

### Registration Flow
```
User
  │
  ├─ Opens http://localhost:3000
  │
  ├─ Clicks "Create one" link
  │
  ├─ Fills register form:
  │  ├─ Username: alice
  │  ├─ Email: alice@example.com
  │  ├─ Password: SecurePass123
  │  └─ Confirm: SecurePass123
  │
  ├─ Clicks "Create Account" button
  │
  ├─ Frontend validates:
  │  ├─ Username 3-30 chars ✓
  │  ├─ Valid email format ✓
  │  ├─ Password 6+ chars ✓
  │  └─ Passwords match ✓
  │
  ├─ POST to /api/auth/register
  │  {
  │    "username": "alice",
  │    "email": "alice@example.com",
  │    "password": "SecurePass123",
  │    "confirmPassword": "SecurePass123"
  │  }
  │
  ├─ Server validates input
  │
  ├─ Server checks if email exists
  │  └─ If exists → Error: "Email already registered"
  │
  ├─ Server hashes password with bcryptjs
  │
  ├─ Server saves user to users.json
  │
  ├─ Server generates JWT token
  │  JWT payload: { userId, email, username }
  │  Expires: 24 hours
  │
  └─ Response:
     {
       "success": true,
       "token": "eyJhbGciOiJIUzI1NiIs...",
       "user": {
         "id": "1234567890",
         "username": "alice",
         "email": "alice@example.com"
       }
     }
     └─ Frontend stores token in localStorage
     └─ Shows success modal
     └─ User can proceed to chat
```

### Login Flow
```
User
  │
  ├─ Opens http://localhost:3000
  │
  ├─ Sees login form by default
  │
  ├─ Fills login form:
  │  ├─ Email: alice@example.com
  │  └─ Password: SecurePass123
  │
  ├─ Clicks "Sign In" button
  │
  ├─ POST to /api/auth/login
  │  {
  │    "email": "alice@example.com",
  │    "password": "SecurePass123"
  │  }
  │
  ├─ Server finds user by email
  │  └─ If not found → Error: "Invalid email or password"
  │
  ├─ Server compares password with hashed version
  │  └─ If doesn't match → Error: "Invalid email or password"
  │
  ├─ Server updates user status to "online"
  │
  ├─ Server generates JWT token
  │
  └─ Response:
     {
       "success": true,
       "token": "eyJhbGciOiJIUzI1NiIs...",
       "user": {
         "id": "1234567890",
         "username": "alice",
         "email": "alice@example.com"
       }
     }
     └─ Frontend stores token in localStorage
     └─ Shows success modal
     └─ User redirected to chat page
```

### Protected API Call Flow
```
Browser (has token in localStorage)
  │
  ├─ User requests GET /api/auth/profile
  │
  ├─ Frontend adds JWT token to header:
  │  Authorization: Bearer eyJhbGciOiJIUzI1NiIs...
  │
  ├─ Server receives request
  │
  ├─ Server extracts token from Authorization header
  │
  ├─ Server verifies token (checks signature & expiration)
  │  └─ If invalid/expired → Error: "Invalid token"
  │
  ├─ If valid, server decodes token to get user info
  │  └─ Extracts: userId, email, username
  │
  ├─ Server retrieves full user profile from users.json
  │
  └─ Response:
     {
       "success": true,
       "user": {
         "id": "1234567890",
         "username": "alice",
         "email": "alice@example.com",
         "status": "online",
         "createdAt": "2026-01-15T10:30:00.000Z"
       }
     }
```

---

## 🔐 Security Layers

### Layer 1: Input Validation
```
Frontend:
├─ Real-time validation as user types
├─ Email format validation
├─ Password length checking
├─ Username character validation
└─ Prevent submit on invalid input

Server:
├─ Re-validate all inputs
├─ express-validator middleware
├─ Sanitize strings
├─ Check password strength
└─ Validate email format
```

### Layer 2: Password Security
```
Registration:
├─ User enters password
├─ Frontend validates 6+ chars
└─ Server hashes with bcryptjs (10 salt rounds)
   Password: "SecurePass123"
   Hash: "$2a$10$aGV5...bw8Se"

Login:
├─ User enters password
├─ bcryptjs.compare() checks against hash
└─ Never stores plain passwords
```

### Layer 3: Token Security
```
JWT Token Generation:
├─ Header: { "alg": "HS256", "typ": "JWT" }
├─ Payload: { userId, email, username, iat, exp }
├─ Signature: HMAC-SHA256(header.payload, secret)
└─ Expires in 24 hours

Token Storage:
├─ Frontend: localStorage (XSS vulnerable)
├─ Better for production: HTTP-only cookie
└─ Token sent in Authorization header

Token Verification:
├─ Server checks signature (prevents tampering)
├─ Checks expiration time
├─ Retrieves user data from payload
└─ Denies access if invalid/expired
```

### Layer 4: Data Protection
```
├─ Email uniqueness enforced
├─ Passwords never logged
├─ User status tracked (online/offline)
├─ Last login timestamp recorded
├─ CORS enabled for security
└─ Content-Type validation
```

---

## 📊 Data Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                        Client Side                          │
│                   (HTML + CSS + JavaScript)                 │
│                                                              │
│  Form Input → Validation → API Call → localStorage          │
│                                            ↓                │
│                                    Display Results          │
│                                            ↓                │
│                                    Show Success/Error       │
│                                                              │
└─────────────────────────────────────────────────────────────┘
              ↕ HTTP POST/GET (JSON)
┌─────────────────────────────────────────────────────────────┐
│                       Server Side                           │
│                  (Node.js + Express.js)                     │
│                                                              │
│  Receive → Parse → Validate → Process → Respond            │
│                        ↓                                    │
│                   Crypto Operations                         │
│                   (bcryptjs / JWT)                          │
│                        ↓                                    │
│                   File I/O                                  │
│                   (users.json)                              │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## 🚀 Deployment Architecture (Future)

```
┌──────────────────────────────────────────────────────────┐
│                   Production Setup                        │
│                                                          │
│  CDN (Static Assets)                                     │
│  ├─ HTML, CSS, JS                                        │
│  └─ Images, Fonts                                        │
│                                                          │
│  Load Balancer                                           │
│  ├─ Round-robin distribution                             │
│  └─ SSL/TLS termination                                  │
│                                                          │
│  Application Servers (multiple instances)                │
│  ├─ Node.js + Express                                    │
│  ├─ Authentication routes                                │
│  └─ WebSocket for chat                                   │
│                                                          │
│  Session Store (Redis)                                   │
│  ├─ Cache tokens                                         │
│  ├─ User sessions                                        │
│  └─ Rate limiting                                        │
│                                                          │
│  Database (PostgreSQL/MongoDB)                           │
│  ├─ User accounts                                        │
│  ├─ Messages                                             │
│  ├─ User profiles                                        │
│  └─ Audit logs                                           │
│                                                          │
│  Message Queue (RabbitMQ/Redis)                          │
│  ├─ Email notifications                                  │
│  ├─ Real-time updates                                    │
│  └─ Background jobs                                      │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

---

## 📈 Performance Considerations

### Current (Development)
- Single-threaded Express server
- File-based storage (users.json)
- No caching mechanism
- Suitable for: Learning, development, small teams

### Scalability Improvements (Production)
- Horizontal scaling with load balancer
- Redis for session storage
- Database for persistence
- CDN for static assets
- Message queue for async operations
- Rate limiting and throttling
- API pagination
- Query optimization

---

## 🔍 Monitoring & Logging

```
Current Implementation:
├─ Console.error() for errors
├─ users.json for data persistence
└─ No audit logging

Future Enhancements:
├─ Structured logging (Winston/Bunyan)
├─ Audit trail for security events
├─ Performance metrics
├─ Error tracking (Sentry)
├─ User activity logging
└─ API response time monitoring
```

---

## 📚 Related Documentation

- [QUICKSTART.md](QUICKSTART.md) - Quick start guide
- [AUTH_SETUP.md](AUTH_SETUP.md) - Detailed API reference
- [IMPLEMENTATION.md](IMPLEMENTATION.md) - Implementation summary

---

**Architecture designed for security, scalability, and maintainability! 🏗️**
