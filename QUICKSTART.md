# 🚀 NetChat Web - Quick Start Guide

## Installation & Running

### Step 1: Install Dependencies
```bash
cd /home/avishkar/Coding/Netchat
npm install
```

### Step 2: Start the Server
```bash
npm start
```

You should see:
```
🚀 NetChat Server is running on http://localhost:3000
📝 Register: POST /api/auth/register
🔑 Login: POST /api/auth/login
👤 Profile: GET /api/auth/profile (requires token)
🚪 Logout: POST /api/auth/logout (requires token)
```

### Step 3: Open Browser
Open your browser and navigate to:
```
http://localhost:3000
```

---

## 🎯 What's Included

### ✅ Backend (Node.js + Express)
- **Register Route** - Create new user accounts with validation
- **Login Route** - Authenticate users with JWT tokens
- **Profile Route** - Get authenticated user details
- **Logout Route** - Update user status to offline
- Password hashing with bcryptjs
- JWT token generation and validation
- Input validation with express-validator

### ✅ Frontend (HTML + CSS + JavaScript)
- **Login Form** - Email, password, remember me option
- **Register Form** - Username, email, password confirmation
- **Modern UI** - Beautiful gradient design, responsive layout
- **Form Validation** - Real-time error messages
- **Password Strength** - Visual indicator while typing
- **Loading States** - Spinner during submission
- **Success Modal** - Confirmation after registration/login

### ✅ Storage
- User data stored in `users.json`
- JWT tokens for session management
- localStorage for frontend token storage

---

## 🧪 Test Credentials (Create Your Own!)

After starting the server, you can:

1. **Create a new account** via the Register form on the website
2. **Login** with the credentials you created
3. **Check data** in `users.json` file

---

## 📊 Example API Calls

### Using curl (Terminal)

**Register:**
```bash
curl -X POST http://localhost:3000/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "alice",
    "email": "alice@example.com",
    "password": "Alice123!",
    "confirmPassword": "Alice123!"
  }'
```

**Login:**
```bash
curl -X POST http://localhost:3000/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "email": "alice@example.com",
    "password": "Alice123!"
  }'
```

---

## 🎨 Features Showcase

### Registration Page
- ✅ Real-time password strength indicator
- ✅ Confirm password matching
- ✅ Email validation
- ✅ Username length validation (3-30 chars)
- ✅ Terms & Privacy agreement checkbox
- ✅ Switch to login link

### Login Page  
- ✅ Email & password authentication
- ✅ Password visibility toggle
- ✅ Remember me checkbox
- ✅ Forgot password link (ready for feature)
- ✅ Switch to register link

### Visual Feedback
- ✅ Loading spinners during submission
- ✅ Error messages with red background
- ✅ Success modal after registration
- ✅ Password strength color coding (red/yellow/green)
- ✅ Responsive mobile design

---

## 📁 File Structure

```
Netchat/
├── server.js                 # Express backend with routes
├── package.json              # Dependencies & scripts
├── .env                      # Environment config
├── AUTH_SETUP.md            # Detailed API documentation
├── users.json               # User data (auto-created)
└── public/
    ├── index.html           # Login/Register UI
    ├── styles.css           # Beautiful styling
    └── script.js            # Form handling & validation
```

---

## 💡 Key Technologies

- **Backend:** Node.js, Express.js
- **Security:** bcryptjs, JWT, CORS
- **Validation:** express-validator
- **Frontend:** HTML5, CSS3, Vanilla JavaScript
- **Storage:** JSON file (local development)

---

## 🔒 Security Notes

✅ Passwords are hashed using bcryptjs  
✅ JWT tokens used for session management  
✅ CORS enabled for security  
✅ Input validation on both client & server  
✅ Email uniqueness enforced  
✅ Tokens expire in 24 hours  

For production:
- Use environment variables for secrets
- Migrate to MongoDB/PostgreSQL
- Implement refresh tokens
- Add rate limiting
- Use HTTPS only
- Add email verification

---

## 🛠️ Development Commands

```bash
# Install dependencies
npm install

# Start server
npm start

# Stop server
Ctrl+C

# For hot-reload (requires nodemon)
npm install --save-dev nodemon
npm run dev
```

---

## 📝 Notes

- Server runs on port 3000 (configurable in .env)
- User data saved to users.json in project root
- All passwords hashed before storage
- JWT tokens expire in 24 hours
- Frontend is fully responsive (mobile-friendly)

---

## 🎓 Learning Outcomes

This implementation demonstrates:
- ✅ RESTful API design
- ✅ User authentication flow
- ✅ Password hashing and security
- ✅ JWT token management
- ✅ Form validation
- ✅ Error handling
- ✅ Responsive UI design
- ✅ Client-server communication

---

## 🚀 Next Steps

1. Enhance with more features (password reset, email verification)
2. Add WebSocket for real-time chat
3. Integrate with the C-based chat server
4. Add user profiles and avatars
5. Implement friends/followers system

---

**Happy Chatting! 🎉**
