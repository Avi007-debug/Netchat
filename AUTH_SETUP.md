# NetChat Authentication System - Setup Guide

## 🚀 Quick Start

### Prerequisites
- Node.js (v14 or higher)
- npm (comes with Node.js)

### Installation

1. **Install dependencies:**
```bash
npm install
```

2. **Configure environment variables:**
Edit `.env` file and update the JWT_SECRET:
```
JWT_SECRET=your-super-secret-key-here
PORT=3000
```

3. **Start the server:**
```bash
npm start
```

The server will run on `http://localhost:3000`

---

## 📋 API Endpoints

### 1. **Register New User**
**Endpoint:** `POST /api/auth/register`

**Request Body:**
```json
{
  "username": "john_doe",
  "email": "john@example.com",
  "password": "SecurePass123",
  "confirmPassword": "SecurePass123"
}
```

**Response (Success):**
```json
{
  "success": true,
  "message": "Registration successful!",
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "id": "1234567890",
    "username": "john_doe",
    "email": "john@example.com"
  }
}
```

**Response (Error - Email exists):**
```json
{
  "success": false,
  "message": "Email already registered. Please login or use a different email."
}
```

**Validation Rules:**
- Username: 3-30 characters
- Email: Valid email format
- Password: Minimum 6 characters
- Passwords must match

---

### 2. **Login User**
**Endpoint:** `POST /api/auth/login`

**Request Body:**
```json
{
  "email": "john@example.com",
  "password": "SecurePass123"
}
```

**Response (Success):**
```json
{
  "success": true,
  "message": "Login successful!",
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "user": {
    "id": "1234567890",
    "username": "john_doe",
    "email": "john@example.com"
  }
}
```

**Response (Error - Invalid credentials):**
```json
{
  "success": false,
  "message": "Invalid email or password"
}
```

---

### 3. **Get User Profile** (Protected Route)
**Endpoint:** `GET /api/auth/profile`

**Headers:**
```
Authorization: Bearer <token>
```

**Response:**
```json
{
  "success": true,
  "user": {
    "id": "1234567890",
    "username": "john_doe",
    "email": "john@example.com",
    "status": "online",
    "createdAt": "2026-01-15T10:30:00.000Z"
  }
}
```

---

### 4. **Logout User** (Protected Route)
**Endpoint:** `POST /api/auth/logout`

**Headers:**
```
Authorization: Bearer <token>
```

**Response:**
```json
{
  "success": true,
  "message": "Logged out successfully"
}
```

---

## 🎨 Frontend Features

### Login Form
- Email and password input
- Password visibility toggle
- "Remember me" option
- Forgot password link (ready for implementation)
- Switch to register form option

### Register Form
- Username input (3-30 characters)
- Email input (validation)
- Password input with strength indicator
- Confirm password field
- Terms of Service & Privacy Policy agreement
- Real-time password strength check
- Switch to login form option

### UI Features
- Beautiful gradient design (purple theme)
- Responsive mobile-friendly layout
- Loading spinners for form submission
- Real-time error messages
- Success modal with confirmation
- Password visibility toggle buttons
- Form validation with error messages

---

## 💾 Data Storage

### User Data Storage
Users are stored in `users.json` file with the following structure:

```json
[
  {
    "id": "1234567890",
    "username": "john_doe",
    "email": "john@example.com",
    "password": "$2a$10$hashed_password_here...",
    "createdAt": "2026-01-15T10:30:00.000Z",
    "status": "online",
    "lastLogin": "2026-01-15T14:25:00.000Z"
  }
]
```

**Note:** Passwords are hashed using bcryptjs before storing.

---

## 🔐 Security Features

✅ **Password Hashing** - Using bcryptjs with salt rounds  
✅ **JWT Tokens** - Secure token-based authentication  
✅ **Input Validation** - Both client-side and server-side  
✅ **CORS Protection** - Configured for security  
✅ **Password Confirmation** - Prevents typos during registration  
✅ **Email Uniqueness** - Prevents duplicate registrations  

---

## 📱 Client-Side Implementation

### Store Token After Login
```javascript
// Token and user data are automatically saved to localStorage
const token = localStorage.getItem('token');
const user = JSON.parse(localStorage.getItem('user'));
```

### Use Token in API Requests
```javascript
fetch('/api/auth/profile', {
  method: 'GET',
  headers: {
    'Authorization': `Bearer ${token}`
  }
})
.then(res => res.json())
.then(data => console.log(data));
```

### Logout
```javascript
// Clear stored data
localStorage.removeItem('token');
localStorage.removeItem('user');

// Call logout endpoint (optional)
fetch('/api/auth/logout', {
  method: 'POST',
  headers: {
    'Authorization': `Bearer ${token}`
  }
});
```

---

## 🛠️ Development Tips

### Testing with curl

**Register:**
```bash
curl -X POST http://localhost:3000/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","email":"test@example.com","password":"Test123","confirmPassword":"Test123"}'
```

**Login:**
```bash
curl -X POST http://localhost:3000/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"test@example.com","password":"Test123"}'
```

**Get Profile:**
```bash
curl -X GET http://localhost:3000/api/auth/profile \
  -H "Authorization: Bearer YOUR_TOKEN_HERE"
```

---

## 📂 Project Structure

```
Netchat/
├── server.js                 # Main Express server with routes
├── package.json              # Dependencies
├── .env                      # Environment variables
├── users.json               # User data storage (auto-created)
└── public/
    ├── index.html           # Login/Register UI
    ├── styles.css           # Styling
    └── script.js            # Frontend JavaScript
```

---

## 🚀 Next Steps

1. **Connect to Chat System:**
   - Redirect authenticated users to chat page
   - Use JWT tokens to verify chat participants

2. **Add More Features:**
   - Password reset functionality
   - Email verification
   - Two-factor authentication
   - Profile editing

3. **Database Migration:**
   - Migrate from JSON file to MongoDB/PostgreSQL for production

4. **Session Management:**
   - Add session timeout
   - Implement refresh tokens

---

## 📝 Notes

- Default server port: 3000
- JWT tokens expire in 24 hours
- All passwords are hashed before storage
- User data is stored locally in `users.json` (suitable for development)

For production deployment, consider using a proper database like MongoDB or PostgreSQL.

---

## 🐛 Troubleshooting

**Port already in use:**
```bash
# Change port in .env file or kill the process
# lsof -i :3000
# kill -9 <PID>
```

**CORS errors:**
Make sure the frontend is served from the same origin or add proper CORS headers.

**Token expired:**
Users need to log in again to get a new token.

---

Happy Coding! 🎉
