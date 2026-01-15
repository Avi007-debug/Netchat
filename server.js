const express = require('express');
const cors = require('cors');
const path = require('path');
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const { body, validationResult } = require('express-validator');
const fs = require('fs');
const http = require('http');
const socketIO = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = socketIO(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

const PORT = process.env.PORT || 3000;
const JWT_SECRET = process.env.JWT_SECRET || 'your-secret-key-change-in-production';

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));

// Users storage file
const USERS_FILE = path.join(__dirname, 'users.json');

// Initialize users file if it doesn't exist
if (!fs.existsSync(USERS_FILE)) {
  fs.writeFileSync(USERS_FILE, JSON.stringify([], null, 2));
}

// Helper functions
function readUsers() {
  try {
    const data = fs.readFileSync(USERS_FILE, 'utf8');
    return JSON.parse(data);
  } catch (error) {
    console.error('Error reading users file:', error);
    return [];
  }
}

function writeUsers(users) {
  try {
    fs.writeFileSync(USERS_FILE, JSON.stringify(users, null, 2));
    return true;
  } catch (error) {
    console.error('Error writing users file:', error);
    return false;
  }
}

function userExists(email) {
  const users = readUsers();
  return users.some(user => user.email === email);
}

function getUserByEmail(email) {
  const users = readUsers();
  return users.find(user => user.email === email);
}

// ===== REGISTER ROUTE =====
app.post(
  '/api/auth/register',
  [
    body('username').trim().isLength({ min: 3, max: 30 }).withMessage('Username must be 3-30 characters'),
    body('email').isEmail().withMessage('Invalid email format'),
    body('password').isLength({ min: 6 }).withMessage('Password must be at least 6 characters'),
    body('confirmPassword').custom((value, { req }) => {
      if (value !== req.body.password) {
        throw new Error('Passwords do not match');
      }
      return true;
    })
  ],
  async (req, res) => {
    // Check validation errors
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({ success: false, errors: errors.array() });
    }

    const { username, email, password } = req.body;

    try {
      // Check if user already exists
      if (userExists(email)) {
        return res.status(409).json({ 
          success: false, 
          message: 'Email already registered. Please login or use a different email.' 
        });
      }

      // Hash password
      const hashedPassword = await bcrypt.hash(password, 10);

      // Create new user
      const newUser = {
        id: Date.now().toString(),
        username,
        email,
        password: hashedPassword,
        createdAt: new Date().toISOString(),
        status: 'offline'
      };

      // Save to file
      const users = readUsers();
      users.push(newUser);
      writeUsers(users);

      // Generate JWT token
      const token = jwt.sign(
        { userId: newUser.id, email: newUser.email, username: newUser.username },
        JWT_SECRET,
        { expiresIn: '24h' }
      );

      res.status(201).json({
        success: true,
        message: 'Registration successful!',
        token,
        user: {
          id: newUser.id,
          username: newUser.username,
          email: newUser.email
        }
      });
    } catch (error) {
      console.error('Registration error:', error);
      res.status(500).json({ 
        success: false, 
        message: 'Server error during registration' 
      });
    }
  }
);

// ===== LOGIN ROUTE =====
app.post(
  '/api/auth/login',
  [
    body('email').isEmail().withMessage('Invalid email format'),
    body('password').isLength({ min: 6 }).withMessage('Invalid credentials')
  ],
  async (req, res) => {
    // Check validation errors
    const errors = validationResult(req);
    if (!errors.isEmpty()) {
      return res.status(400).json({ success: false, errors: errors.array() });
    }

    const { email, password } = req.body;

    try {
      // Find user by email
      const user = getUserByEmail(email);
      if (!user) {
        return res.status(401).json({
          success: false,
          message: 'Invalid email or password'
        });
      }

      // Compare passwords
      const isPasswordValid = await bcrypt.compare(password, user.password);
      if (!isPasswordValid) {
        return res.status(401).json({
          success: false,
          message: 'Invalid email or password'
        });
      }

      // Generate JWT token
      const token = jwt.sign(
        { userId: user.id, email: user.email, username: user.username },
        JWT_SECRET,
        { expiresIn: '24h' }
      );

      // Update user status
      const users = readUsers();
      const userIndex = users.findIndex(u => u.email === email);
      if (userIndex !== -1) {
        users[userIndex].status = 'online';
        users[userIndex].lastLogin = new Date().toISOString();
        writeUsers(users);
      }

      res.status(200).json({
        success: true,
        message: 'Login successful!',
        token,
        user: {
          id: user.id,
          username: user.username,
          email: user.email
        }
      });
    } catch (error) {
      console.error('Login error:', error);
      res.status(500).json({
        success: false,
        message: 'Server error during login'
      });
    }
  }
);

// Middleware to verify JWT token
function verifyToken(req, res, next) {
  const token = req.headers['authorization']?.split(' ')[1];
  if (!token) {
    return res.status(401).json({ success: false, message: 'No token provided' });
  }

  try {
    const decoded = jwt.verify(token, JWT_SECRET);
    req.user = decoded;
    next();
  } catch (error) {
    res.status(401).json({ success: false, message: 'Invalid token' });
  }
}

// ===== PROTECTED ROUTE: Get User Profile =====
app.get('/api/auth/profile', verifyToken, (req, res) => {
  const user = getUserByEmail(req.user.email);
  if (!user) {
    return res.status(404).json({ success: false, message: 'User not found' });
  }

  res.json({
    success: true,
    user: {
      id: user.id,
      username: user.username,
      email: user.email,
      status: user.status,
      createdAt: user.createdAt
    }
  });
});

// ===== LOGOUT ROUTE =====
app.post('/api/auth/logout', verifyToken, (req, res) => {
  try {
    const users = readUsers();
    const userIndex = users.findIndex(u => u.id === req.user.userId);
    if (userIndex !== -1) {
      users[userIndex].status = 'offline';
      writeUsers(users);
    }

    res.json({
      success: true,
      message: 'Logged out successfully'
    });
  } catch (error) {
    console.error('Logout error:', error);
    res.status(500).json({
      success: false,
      message: 'Server error during logout'
    });
  }
});

// Serve index.html for root path
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Serve chat.html for authenticated users
app.get('/chat.html', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'chat.html'));
});

// ===== WEBSOCKET CONFIGURATION =====
// Store connected users
const connectedUsers = new Map();
const chatRooms = new Map();

// Middleware to verify JWT token for WebSocket connections
io.use((socket, next) => {
  const token = socket.handshake.auth.token;
  if (!token) {
    return next(new Error('Authentication failed'));
  }

  try {
    const decoded = jwt.verify(token, JWT_SECRET);
    socket.userId = decoded.userId;
    socket.username = decoded.username;
    socket.email = decoded.email;
    next();
  } catch (error) {
    next(new Error('Invalid token'));
  }
});

// Handle WebSocket connections
io.on('connection', (socket) => {
  console.log(`\n👤 User connected: ${socket.username} (${socket.id})`);
  
  // Add user to connected users map
  connectedUsers.set(socket.userId, {
    socketId: socket.id,
    username: socket.username,
    email: socket.email,
    room: null,
    connectedAt: new Date()
  });

  // Broadcast updated user list to all connected clients
  io.emit('users:update', Array.from(connectedUsers.values()));

  // ===== Join Room =====
  socket.on('room:join', (data) => {
    const { roomName } = data;
    
    // Leave previous room if in one
    if (connectedUsers.get(socket.userId)?.room) {
      socket.leave(connectedUsers.get(socket.userId).room);
      io.to(connectedUsers.get(socket.userId).room).emit('message:new', {
        type: 'system',
        username: 'System',
        message: `${socket.username} left the room`,
        timestamp: new Date().toISOString(),
        room: connectedUsers.get(socket.userId).room
      });
    }

    // Join new room
    socket.join(roomName);
    connectedUsers.get(socket.userId).room = roomName;

    // Initialize room if doesn't exist
    if (!chatRooms.has(roomName)) {
      chatRooms.set(roomName, {
        name: roomName,
        createdAt: new Date(),
        messages: [],
        users: []
      });
    }

    // Add user to room
    const room = chatRooms.get(roomName);
    if (!room.users.includes(socket.username)) {
      room.users.push(socket.username);
    }

    // Notify room that user joined
    io.to(roomName).emit('message:new', {
      type: 'system',
      username: 'System',
      message: `${socket.username} joined the room`,
      timestamp: new Date().toISOString(),
      room: roomName
    });

    // Send room info to all users in room
    io.to(roomName).emit('room:info', {
      name: roomName,
      users: room.users,
      messageCount: room.messages.length
    });

    console.log(`📌 ${socket.username} joined room: ${roomName}`);
  });

  // ===== Send Message =====
  socket.on('message:send', (data) => {
    const { message, room } = data;
    const user = connectedUsers.get(socket.userId);

    if (!user || !user.room) {
      socket.emit('error', { message: 'Not in a room' });
      return;
    }

    const messageObj = {
      id: Date.now().toString(),
      userId: socket.userId,
      username: socket.username,
      message: message.trim(),
      timestamp: new Date().toISOString(),
      room: user.room,
      type: 'user'
    };

    // Store message in room
    if (chatRooms.has(user.room)) {
      chatRooms.get(user.room).messages.push(messageObj);
    }

    // Broadcast message to room
    io.to(user.room).emit('message:new', messageObj);
    console.log(`💬 ${socket.username} in ${user.room}: ${message}`);
  });

  // ===== Get Room Messages =====
  socket.on('room:getMessages', (data) => {
    const { room } = data;
    
    if (chatRooms.has(room)) {
      const messages = chatRooms.get(room).messages;
      socket.emit('room:messages', { room, messages });
    } else {
      socket.emit('room:messages', { room, messages: [] });
    }
  });

  // ===== Get Available Rooms =====
  socket.on('rooms:get', () => {
    const rooms = Array.from(chatRooms.keys()).map(roomName => {
      const room = chatRooms.get(roomName);
      return {
        name: roomName,
        users: room.users.length,
        messages: room.messages.length
      };
    });
    socket.emit('rooms:list', { rooms });
  });

  // ===== Leave Room =====
  socket.on('room:leave', () => {
    const user = connectedUsers.get(socket.userId);
    
    if (user && user.room) {
      const room = user.room;
      socket.leave(room);

      io.to(room).emit('message:new', {
        type: 'system',
        username: 'System',
        message: `${socket.username} left the room`,
        timestamp: new Date().toISOString(),
        room: room
      });

      // Update room users list
      if (chatRooms.has(room)) {
        const roomData = chatRooms.get(room);
        roomData.users = roomData.users.filter(u => u !== socket.username);
        
        io.to(room).emit('room:info', {
          name: room,
          users: roomData.users,
          messageCount: roomData.messages.length
        });
      }

      user.room = null;
      console.log(`📌 ${socket.username} left room: ${room}`);
    }
  });

  // ===== User Typing =====
  socket.on('user:typing', () => {
    const user = connectedUsers.get(socket.userId);
    if (user && user.room) {
      socket.to(user.room).emit('user:typing', {
        username: socket.username,
        room: user.room
      });
    }
  });

  // ===== User Stopped Typing =====
  socket.on('user:stopTyping', () => {
    const user = connectedUsers.get(socket.userId);
    if (user && user.room) {
      socket.to(user.room).emit('user:stopTyping', {
        username: socket.username,
        room: user.room
      });
    }
  });

  // ===== Disconnect =====
  socket.on('disconnect', () => {
    const user = connectedUsers.get(socket.userId);
    
    if (user) {
      // Notify room that user left
      if (user.room && chatRooms.has(user.room)) {
        const room = chatRooms.get(user.room);
        room.users = room.users.filter(u => u !== socket.username);

        io.to(user.room).emit('message:new', {
          type: 'system',
          username: 'System',
          message: `${socket.username} disconnected`,
          timestamp: new Date().toISOString(),
          room: user.room
        });

        io.to(user.room).emit('room:info', {
          name: user.room,
          users: room.users,
          messageCount: room.messages.length
        });
      }

      // Remove user from connected users
      connectedUsers.delete(socket.userId);
      io.emit('users:update', Array.from(connectedUsers.values()));

      console.log(`👤 User disconnected: ${socket.username}`);
    }
  });

  // ===== Error Handling =====
  socket.on('error', (error) => {
    console.error(`Socket error for ${socket.username}:`, error);
  });
});

// 404 handler
app.use((req, res) => {
  res.status(404).json({ success: false, message: 'Route not found' });
});

// Error handler
app.use((err, req, res, next) => {
  console.error('Error:', err);
  res.status(500).json({ 
    success: false, 
    message: 'Internal server error' 
  });
});

app.listen(PORT, () => {
  console.log(`\n🚀 NetChat Server is running on http://localhost:${PORT}`);
  console.log(`📝 Register: POST /api/auth/register`);
  console.log(`🔑 Login: POST /api/auth/login`);
  console.log(`👤 Profile: GET /api/auth/profile (requires token)`);
  console.log(`🚪 Logout: POST /api/auth/logout (requires token)`);
  console.log(`💬 WebSocket: /socket.io (real-time chat)\n`);
});
