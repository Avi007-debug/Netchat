// Chat Client - WebSocket Handler
const socket = io({
    auth: {
        token: localStorage.getItem('token')
    }
});

// DOM Elements
const currentUserEl = document.getElementById('currentUser');
const roomsList = document.getElementById('roomsList');
const usersList = document.getElementById('usersList');
const messagesContainer = document.getElementById('messagesContainer');
const messageInput = document.getElementById('messageInput');
const sendBtn = document.getElementById('sendBtn');
const createRoomBtn = document.getElementById('createRoomBtn');
const newRoomInput = document.getElementById('newRoomInput');
const logoutBtn = document.getElementById('logoutBtn');
const roomInfo = document.getElementById('roomInfo');
const typingIndicator = document.getElementById('typingIndicator');
const typingText = document.getElementById('typingText');

// State
let currentUser = null;
let currentRoom = null;
let connectedUsers = [];
let typingUsers = [];
let typingTimeout = null;

// Initialize
window.addEventListener('load', () => {
    // Check if user is logged in
    const token = localStorage.getItem('token');
    const user = localStorage.getItem('user');

    if (!token || !user) {
        // Redirect to login
        window.location.href = '/';
        return;
    }

    currentUser = JSON.parse(user);
    currentUserEl.textContent = `Welcome, ${currentUser.username}!`;

    // Get available rooms
    socket.emit('rooms:get');
});

// ===== Socket Events =====

// Connection
socket.on('connect', () => {
    console.log('✅ Connected to server');
});

// Connection Error
socket.on('connect_error', (error) => {
    console.error('❌ Connection error:', error.message);
    if (error.message === 'Authentication failed' || error.message === 'Invalid token') {
        // Token invalid, redirect to login
        localStorage.removeItem('token');
        localStorage.removeItem('user');
        window.location.href = '/';
    }
});

// Users Update
socket.on('users:update', (users) => {
    connectedUsers = users;
    updateUsersList();
});

// New Message
socket.on('message:new', (message) => {
    addMessageToUI(message);
    
    // Clear typing indicator
    typingUsers = typingUsers.filter(u => u !== message.username);
    updateTypingIndicator();
});

// Rooms List
socket.on('rooms:list', (data) => {
    const { rooms } = data;
    updateRoomsList(rooms);
});

// Room Info
socket.on('room:info', (data) => {
    const { name, users, messageCount } = data;
    updateRoomInfo(name, users, messageCount);
});

// Room Messages
socket.on('room:messages', (data) => {
    const { room, messages } = data;
    loadRoomMessages(room, messages);
});

// User Typing
socket.on('user:typing', (data) => {
    const { username, room } = data;
    if (!typingUsers.includes(username) && room === currentRoom) {
        typingUsers.push(username);
        updateTypingIndicator();
    }
});

// User Stop Typing
socket.on('user:stopTyping', (data) => {
    const { username, room } = data;
    typingUsers = typingUsers.filter(u => u !== username);
    updateTypingIndicator();
});

// Error
socket.on('error', (error) => {
    console.error('❌ Socket error:', error.message);
});

// ===== Message Functions =====

function addMessageToUI(message) {
    // Clear "no messages" state
    const noMessages = messagesContainer.querySelector('.no-messages');
    if (noMessages) {
        noMessages.remove();
    }

    const messageEl = document.createElement('div');
    messageEl.className = `message ${getMessageClass(message)}`;

    const timestamp = new Date(message.timestamp).toLocaleTimeString();

    if (message.type === 'system') {
        messageEl.innerHTML = `
            <div class="message-bubble">
                ${message.message}
            </div>
        `;
    } else {
        messageEl.innerHTML = `
            <div class="message-username">${message.username}</div>
            <div class="message-bubble">
                ${escapeHTML(message.message)}
                <div class="message-time">${timestamp}</div>
            </div>
        `;
    }

    messagesContainer.appendChild(messageEl);
    scrollToBottom();
}

function getMessageClass(message) {
    if (message.type === 'system') {
        return 'message-system';
    }
    if (message.userId === currentUser.id) {
        return 'message-own';
    }
    return 'message-other';
}

function escapeHTML(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

function scrollToBottom() {
    messagesContainer.scrollTop = messagesContainer.scrollHeight;
}

function loadRoomMessages(room, messages) {
    messagesContainer.innerHTML = '';
    
    if (messages.length === 0) {
        messagesContainer.innerHTML = '<div class="no-messages"><p>No messages yet. Say something!</p></div>';
    } else {
        messages.forEach(msg => {
            addMessageToUI(msg);
        });
    }
}

// ===== Room Functions =====

function updateRoomsList(rooms) {
    roomsList.innerHTML = '';
    
    if (rooms.length === 0) {
        roomsList.innerHTML = '<div class="loading">No rooms yet</div>';
        return;
    }

    rooms.forEach(room => {
        const roomEl = document.createElement('div');
        roomEl.className = `room-item ${room.name === currentRoom ? 'active' : ''}`;
        roomEl.innerHTML = `
            <div>${room.name}</div>
            <div class="room-item-info">
                <span>👥 ${room.users}</span>
                <span>💬 ${room.messages}</span>
            </div>
        `;
        roomEl.onclick = () => joinRoom(room.name);
        roomsList.appendChild(roomEl);
    });
}

function updateRoomInfo(name, users, messageCount) {
    roomInfo.innerHTML = `
        <div style="display: flex; justify-content: space-between; align-items: center;">
            <div>
                <strong>${name}</strong> · ${users.length} users · ${messageCount} messages
            </div>
        </div>
    `;
}

function joinRoom(roomName) {
    if (currentRoom === roomName) return;
    
    currentRoom = roomName;
    socket.emit('room:join', { roomName });
    
    // Clear messages and load new room messages
    messagesContainer.innerHTML = '<div class="loading">Loading messages...</div>';
    socket.emit('room:getMessages', { room: roomName });

    // Enable message input
    messageInput.disabled = false;
    sendBtn.disabled = false;
    messageInput.focus();

    // Update rooms list UI
    document.querySelectorAll('.room-item').forEach(el => {
        el.classList.remove('active');
    });
    event.target.closest('.room-item').classList.add('active');

    console.log(`📌 Joined room: ${roomName}`);
}

// ===== Users Functions =====

function updateUsersList() {
    usersList.innerHTML = '';
    
    if (connectedUsers.length === 0) {
        usersList.innerHTML = '<div class="loading">No users online</div>';
        return;
    }

    connectedUsers.forEach(user => {
        const userEl = document.createElement('div');
        userEl.className = 'user-item';
        const statusDot = user.id === currentUser.id ? '🟢' : '🔵';
        userEl.innerHTML = `
            ${statusDot} ${user.username}
            ${user.room ? `<div style="font-size: 11px; opacity: 0.7; margin-top: 3px;">📌 ${user.room}</div>` : ''}
        `;
        usersList.appendChild(userEl);
    });
}

// ===== Input Handling =====

sendBtn.onclick = sendMessage;

messageInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
        e.preventDefault();
        sendMessage();
    }
});

function sendMessage() {
    const message = messageInput.value.trim();
    
    if (!message) return;
    if (!currentRoom) {
        alert('Please join a room first');
        return;
    }

    socket.emit('message:send', {
        message: message,
        room: currentRoom
    });

    messageInput.value = '';
    socket.emit('user:stopTyping');
    clearTimeout(typingTimeout);
}

// Typing Indicator
messageInput.addEventListener('input', () => {
    if (!currentRoom) return;

    socket.emit('user:typing');

    clearTimeout(typingTimeout);
    typingTimeout = setTimeout(() => {
        socket.emit('user:stopTyping');
    }, 3000);
});

function updateTypingIndicator() {
    const otherTypingUsers = typingUsers.filter(u => u !== currentUser.username);
    
    if (otherTypingUsers.length === 0) {
        typingIndicator.style.display = 'none';
        return;
    }

    typingIndicator.style.display = 'block';
    if (otherTypingUsers.length === 1) {
        typingText.textContent = `${otherTypingUsers[0]} is typing...`;
    } else if (otherTypingUsers.length === 2) {
        typingText.textContent = `${otherTypingUsers.join(' and ')} are typing...`;
    } else {
        typingText.textContent = `${otherTypingUsers.length} users are typing...`;
    }
}

// Create/Join Room
createRoomBtn.onclick = () => {
    const roomName = newRoomInput.value.trim();
    
    if (!roomName) {
        alert('Please enter a room name');
        return;
    }

    joinRoom(roomName);
    newRoomInput.value = '';
    socket.emit('rooms:get');
};

newRoomInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
        createRoomBtn.click();
    }
});

// Logout
logoutBtn.onclick = () => {
    // Get token
    const token = localStorage.getItem('token');

    // Call logout endpoint
    if (token) {
        fetch('/api/auth/logout', {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${token}`
            }
        }).then(() => {
            // Clear storage
            localStorage.removeItem('token');
            localStorage.removeItem('user');
            
            // Disconnect socket
            socket.disconnect();
            
            // Redirect to login
            window.location.href = '/';
        });
    } else {
        // No token, just redirect
        localStorage.removeItem('token');
        localStorage.removeItem('user');
        socket.disconnect();
        window.location.href = '/';
    }
};

// Periodically refresh rooms list
setInterval(() => {
    socket.emit('rooms:get');
}, 5000);

console.log('💬 Chat client initialized');
