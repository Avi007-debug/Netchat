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
const leaveRoomBtn = document.getElementById('leaveRoomBtn');
const typingIndicator = document.getElementById('typingIndicator');
const typingText = document.getElementById('typingText');
const pmModal = document.getElementById('pmModal');
const pmUsername = document.getElementById('pmUsername');
const pmUsernameInfo = document.getElementById('pmUsernameInfo');
const pmMessagesContainer = document.getElementById('pmMessages');
const pmInput = document.getElementById('pmInput');
const pmSendBtn = document.getElementById('pmSendBtn');
const pmCloseBtn = document.getElementById('pmCloseBtn');
const pmEncryptBtn = document.getElementById('pmEncryptBtn');
const pmImageBtn = document.getElementById('pmImageBtn');
const pmImageInput = document.getElementById('pmImageInput');
const imageBtn = document.getElementById('imageBtn');
const imageInput = document.getElementById('imageInput');
const encryptBtn = document.getElementById('encryptBtn');

// State
let currentUser = null;
let currentRoom = null;
let connectedUsers = [];
let typingUsers = [];
let typingTimeout = null;
let currentPMUser = null;
let pmMessagesMap = new Map(); // Store PM history
// Persisted unread counts: { username: number }
let unreadCounts = {};
let isEncryptionEnabled = false; // Encryption toggle state
let encryptionPassword = null; // Password for encryption for rooms
let isPMEncryptionEnabled = false; // Encryption toggle state for private messages
let pmEncryptionPassword = null; // Password for PM encryption
let pmImageUrl = null; // Temporary storage for PM image

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

    // Load persisted unread counts
    try {
        const raw = localStorage.getItem('netchat_unread');
        unreadCounts = raw ? JSON.parse(raw) : {};
    } catch (e) {
        unreadCounts = {};
    }

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

// Duplicate Session Detected
socket.on('session:duplicate', (data) => {
    alert('⚠️ Your account is already logged in from another location. You will be disconnected.');
    localStorage.removeItem('token');
    localStorage.removeItem('user');
    window.location.href = '/';
});

// Private Message Received (single consolidated handler)
socket.on('pm:received', (data) => {
    const { from, message, encrypted, imageUrl, timestamp } = data;
    
    // Don't auto-decrypt - let user click to decrypt like room messages
    let displayMessage = message;

    // Browser notification
    const notifMsg = imageUrl ? `New PM from ${from} (image)` : `New PM from ${from}${encrypted && message ? ' (encrypted)' : ''}`;
    showNotification(notifMsg);

    // Store PM locally
    if (!pmMessagesMap.has(from)) pmMessagesMap.set(from, []);
    pmMessagesMap.get(from).push({ from, message: displayMessage, imageUrl: imageUrl || null, timestamp, encrypted: encrypted || false, type: 'received' });

    // If PM chat is open with this user, show immediately and do not mark unread
    if (currentPMUser === from) {
        if (imageUrl) {
            addPMImageToUI(from, imageUrl, displayMessage, timestamp, 'received');
        } else if (encrypted) {
            // Show as encrypted for PM - user must click to decrypt
            addEncryptedPMToUI(from, displayMessage, timestamp, 'received');
        } else {
            addPMToUI(from, displayMessage, timestamp, 'received');
        }
    } else {
        // Increment unread count and persist
        unreadCounts[from] = (unreadCounts[from] || 0) + 1;
        try { localStorage.setItem('netchat_unread', JSON.stringify(unreadCounts)); } catch (e) {}

        // Add unread badge to PM button next to user
        const userBtn = document.querySelector(`.user-item[data-username="${from}"] .pm-btn`);
        if (userBtn) {
            userBtn.classList.add('pm-unread');
        }

        // Show in-page toast notification (improved)
        const preview = imageUrl ? '📷 Image shared' : 
                       encrypted ? '🔐 Encrypted message' : 
                       displayMessage.substring(0, 50) + (displayMessage.length > 50 ? '...' : '');
        const toastMsg = `💬 PM from <strong>${from}</strong>:<br>${preview}`;
        showToast(toastMsg, 5000);
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

// ===== Encryption Functions (Client-side) =====

// Simple client-side encryption using XOR cipher (for browser compatibility)
function encryptMessage(text, password) {
    try {
        if (!password) throw new Error('Password required');
        
        // Create a more reliable hash from password
        let hash = 0;
        for (let i = 0; i < password.length; i++) {
            hash = ((hash << 5) - hash) + password.charCodeAt(i);
            hash = hash & hash; // Convert to 32bit integer
        }
        
        // Use Math.abs to ensure positive value for XOR
        hash = Math.abs(hash);
        
        // Simple XOR cipher on original text
        let encrypted = '';
        for (let i = 0; i < text.length; i++) {
            const charCode = text.charCodeAt(i);
            const keyByte = (hash >> ((i * 7) % 32)) & 0xFF;
            encrypted += String.fromCharCode(charCode ^ keyByte);
        }
        
        // Base64 encode the encrypted result for safe transmission
        return btoa(encrypted);
    } catch (error) {
        console.error('Encryption error:', error);
        throw new Error('Encryption failed');
    }
}

function decryptMessage(encryptedText, password) {
    try {
        if (!password) throw new Error('Password required');
        
        // Recreate the same hash
        let hash = 0;
        for (let i = 0; i < password.length; i++) {
            hash = ((hash << 5) - hash) + password.charCodeAt(i);
            hash = hash & hash; // Convert to 32bit integer
        }
        
        // Use Math.abs to ensure positive value for XOR
        hash = Math.abs(hash);
        
        // Base64 decode the encrypted text
        const encrypted = atob(encryptedText);
        
        // Simple XOR decipher (XOR is symmetric)
        let decrypted = '';
        for (let i = 0; i < encrypted.length; i++) {
            const charCode = encrypted.charCodeAt(i);
            const keyByte = (hash >> ((i * 7) % 32)) & 0xFF;
            decrypted += String.fromCharCode(charCode ^ keyByte);
        }
        
        return decrypted;
    } catch (error) {
        console.error('Decryption error:', error);
        throw new Error('Decryption failed - wrong password or corrupted message');
    }
}

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
        // Check if message contains an image
        if (message.imageUrl) {
            messageEl.innerHTML = `
                <div class="message-username">${message.username}</div>
                <div class="message-bubble">
                    <div class="message-image-container">
                        <img src="${escapeHTML(message.imageUrl)}" alt="Shared image" class="message-image" onclick="openImageModal(this.src)">
                    </div>
                    ${message.message ? `<div class="image-caption">${escapeHTML(message.message)}</div>` : ''}
                    <div class="message-time">${timestamp}</div>
                </div>
            `;
        } else {
            if (message.encrypted) {
                // Encrypted message - clickable to decrypt
                messageEl.innerHTML = `
                    <div class="message-username">${message.username}</div>
                    <div class="message-bubble encrypted-message" data-encrypted-text="${escapeHTML(message.message)}" onclick="decryptMessage(this)">
                        <span class="lock-icon">🔐</span>
                        <span class="encrypted-text">Encrypted message - Click to decrypt</span>
                        <div class="message-time">${timestamp}</div>
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
        }
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
    // Show leave room button when in a room
    leaveRoomBtn.style.display = 'block';
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
    imageBtn.disabled = false;
    encryptBtn.disabled = false;
    messageInput.focus;

    // Update rooms list UI
    document.querySelectorAll('.room-item').forEach(el => {
        el.classList.remove('active');
    });
    event.target.closest('.room-item').classList.add('active');

    console.log(`📌 Joined room: ${roomName}`);
}

// Leave room
leaveRoomBtn.onclick = () => {
    if (currentRoom) {
        socket.emit('room:leave');
        currentRoom = null;
        leaveRoomBtn.style.display = 'none';
        messagesContainer.innerHTML = '<div class="no-messages"><p>No messages yet. Say something!</p></div>';
        roomInfo.innerHTML = '<p>Select a room to start chatting</p>';
        messageInput.disabled = true;
        sendBtn.disabled = true;
        imageBtn.disabled = true;
        encryptBtn.disabled = true;
        // Deselect room in UI
        document.querySelectorAll('.room-item').forEach(el => {
            el.classList.remove('active');
        });
        socket.emit('rooms:get');
        console.log(`🚪 Left room: ${currentRoom}`);
    }
};

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
        userEl.dataset.username = user.username;
        const statusDot = user.id === currentUser.id ? '🟢' : '🔵';
        userEl.innerHTML = `
            <div class="user-info">
                ${statusDot} ${user.username}
                ${user.room ? `<div style="font-size: 11px; opacity: 0.7; margin-top: 3px;">📌 ${user.room}</div>` : ''}
            </div>
            ${user.username !== currentUser.username ? '<button class="pm-btn" title="Send private message">💬</button>' : ''}
        `;
        
        // Add click handler for PM button
        if (user.username !== currentUser.username) {
            const pmBtn = userEl.querySelector('.pm-btn');
            if (pmBtn) {
                pmBtn.dataset.username = user.username;
                pmBtn.onclick = (e) => {
                    e.stopPropagation();
                    openPMChat(user.username);
                };

                // Apply unread badge if persisted count exists
                if (unreadCounts[user.username] && unreadCounts[user.username] > 0) {
                    pmBtn.classList.add('pm-unread');
                }
            }
        }
        
        usersList.appendChild(userEl);
    });
}

// ===== Input Handling =====

// Image Upload Functions
async function uploadImage(file) {
    const formData = new FormData();
    formData.append('image', file);

    const token = localStorage.getItem('token');
    if (!token) {
        showToast('❌ Authentication required');
        return null;
    }

    try {
        const response = await fetch('/api/upload/image', {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${token}`
            },
            body: formData
        });

        const data = await response.json();
        if (data.success) {
            return data.imageUrl;
        } else {
            showToast(`❌ ${data.message}`);
            return null;
        }
    } catch (error) {
        console.error('Image upload error:', error);
        showToast('❌ Failed to upload image');
        return null;
    }
}

function openImageModal(src) {
    const modal = document.createElement('div');
    modal.className = 'image-modal';
    modal.innerHTML = `
        <div class="image-modal-content">
            <span class="image-modal-close">&times;</span>
            <img src="${src}" alt="Full size image">
        </div>
    `;
    
    document.body.appendChild(modal);
    
    const closeBtn = modal.querySelector('.image-modal-close');
    closeBtn.onclick = () => modal.remove();
    modal.onclick = (e) => {
        if (e.target === modal) {
            modal.remove();
        }
    };
}

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
        room: currentRoom,
        encrypted: isEncryptionEnabled,
        encryptionPassword: encryptionPassword
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

// ===== Private Messaging Functions =====

// ===== Image Upload Functions =====

async function uploadImage(file) {
    const formData = new FormData();
    formData.append('image', file);

    const token = localStorage.getItem('token');
    if (!token) {
        showToast('❌ Authentication required');
        return null;
    }

    try {
        const response = await fetch('/api/upload/image', {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${token}`
            },
            body: formData
        });

        const data = await response.json();
        if (data.success) {
            return data.imageUrl;
        } else {
            showToast(`❌ ${data.message}`);
            return null;
        }
    } catch (error) {
        console.error('Image upload error:', error);
        showToast('❌ Failed to upload image');
        return null;
    }
}

function openImageModal(src) {
    const modal = document.createElement('div');
    modal.className = 'image-modal';
    modal.innerHTML = `
        <div class="image-modal-content">
            <span class="image-modal-close">&times;</span>
            <img src="${src}" alt="Full size image">
        </div>
    `;
    
    document.body.appendChild(modal);
    
    const closeBtn = modal.querySelector('.image-modal-close');
    closeBtn.onclick = () => modal.remove();
    modal.onclick = (e) => {
        if (e.target === modal) {
            modal.remove();
        }
    };
}

// ===== Private Message Functions =====

function openPMChat(username) {
    currentPMUser = username;
    pmUsername.textContent = username;
    pmUsernameInfo.textContent = username;
    pmModal.style.display = 'flex';
    pmImageUrl = null; // Reset image
    
    // Reset encryption for new PM chat
    isPMEncryptionEnabled = false;
    pmEncryptionPassword = null;
    if (pmEncryptBtn) {
        pmEncryptBtn.textContent = '🔓';
        pmEncryptBtn.title = 'PM Encryption OFF - Messages sent as plain text. Click to enable.';
        pmEncryptBtn.classList.remove('active');
    }
    
    // Load PM history
    loadPMHistory(username);
    pmInput.focus();
    // Clear unread badge on PM button
    const userBtn = document.querySelector(`.user-item[data-username="${username}"] .pm-btn`);
    if (userBtn) {
        userBtn.classList.remove('pm-unread');
    }
    // Clear persisted unread count for this user
    if (unreadCounts[username]) {
        unreadCounts[username] = 0;
        try { localStorage.setItem('netchat_unread', JSON.stringify(unreadCounts)); } catch (e) {}
    }
}

function closePMChat() {
    pmModal.style.display = 'none';
    currentPMUser = null;
}

function loadPMHistory(username) {
    const container = pmMessagesContainer;
    container.innerHTML = `<div class="pm-info">Private messages are only visible to you and ${username}</div>`;
    
    if (pmMessagesMap.has(username)) {
        pmMessagesMap.get(username).forEach(pm => {
            if (pm.imageUrl) {
                addPMImageToUI(username, pm.imageUrl, pm.message, pm.timestamp, pm.type);
            } else if (pm.encrypted) {
                addEncryptedPMToUI(username, pm.message, pm.timestamp, pm.type);
            } else {
                addPMToUI(username, pm.message, pm.timestamp, pm.type);
            }
        });
    }
}

function addPMToUI(username, message, timestamp, type) {
    const container = pmMessagesContainer;
    const msgEl = document.createElement('div');
    msgEl.className = `pm-message pm-${type}`;
    
    const time = new Date(timestamp).toLocaleTimeString();
    const label = type === 'sent' ? 'You' : username;
    
    msgEl.innerHTML = `
        <div class="pm-msg-label">${label}</div>
        <div class="pm-msg-text">${escapeHTML(message)}</div>
        <div class="pm-msg-time">${time}</div>
    `;
    
    container.appendChild(msgEl);
    container.scrollTop = container.scrollHeight;
}

function addEncryptedPMToUI(username, encryptedText, timestamp, type) {
    const container = pmMessagesContainer;
    const msgEl = document.createElement('div');
    msgEl.className = `pm-message pm-${type}`;
    
    const time = new Date(timestamp).toLocaleTimeString();
    const label = type === 'sent' ? 'You' : username;
    
    msgEl.innerHTML = `
        <div class="pm-msg-label">${label}</div>
        <div class="pm-msg-text pm-encrypted-bubble" data-encrypted-text="${escapeHTML(encryptedText)}" onclick="decryptPMMessage(this)">
            <span class="lock-icon">🔐</span>
            <span class="encrypted-text">Encrypted message - Click to decrypt</span>
            <div class="pm-msg-time">${time}</div>
        </div>
    `;
    
    container.appendChild(msgEl);
    container.scrollTop = container.scrollHeight;
}

function addPMImageToUI(username, imageUrl, caption, timestamp, type) {
    const container = pmMessagesContainer;
    const msgEl = document.createElement('div');
    msgEl.className = `pm-message pm-${type}`;
    
    const time = new Date(timestamp).toLocaleTimeString();
    const label = type === 'sent' ? 'You' : username;
    
    msgEl.innerHTML = `
        <div class="pm-msg-label">${label}</div>
        <div class="pm-message-image-container">
            <img src="${escapeHTML(imageUrl)}" alt="Shared image" class="pm-message-image" onclick="openImageModal(this.src)">
            ${caption ? `<div class="pm-image-caption">${escapeHTML(caption)}</div>` : ''}
            <div class="pm-msg-time">${time}</div>
        </div>
    `;
    
    container.appendChild(msgEl);
    container.scrollTop = container.scrollHeight;
}

function sendPrivateMessage() {
    const message = pmInput.value.trim();
    if (!message && !pmImageUrl) return;
    if (!currentPMUser) return;
    
    const timestamp = new Date().toISOString();
    
    // Send to server (let server handle encryption like in group chat)
    socket.emit('pm:send', {
        to: currentPMUser,
        message: message,
        encrypted: isPMEncryptionEnabled,
        encryptionPassword: isPMEncryptionEnabled ? pmEncryptionPassword : undefined,
        imageUrl: pmImageUrl
    });
    
    // Store locally
    if (!pmMessagesMap.has(currentPMUser)) {
        pmMessagesMap.set(currentPMUser, []);
    }
    pmMessagesMap.get(currentPMUser).push({ 
        message: message,
        imageUrl: pmImageUrl, 
        timestamp, 
        encrypted: isPMEncryptionEnabled,
        type: 'sent' 
    });
    
    // Add to UI - show plain text for now, will display encrypted on server if needed
    if (pmImageUrl) {
        addPMImageToUI(currentPMUser, pmImageUrl, message, timestamp, 'sent');
    } else if (isPMEncryptionEnabled) {
        // Show as encrypted (we'll let server encrypt it)
        addEncryptedPMToUI(currentPMUser, message, timestamp, 'sent');
    } else {
        addPMToUI(currentPMUser, message, timestamp, 'sent');
    }
    
    pmInput.value = '';
    pmImageUrl = null;
    
    showToast('✅ PM sent!', 3000);
}

function showNotification(message) {
    // Simple notification - you can enhance this
    if (Notification.permission === 'granted') {
        new Notification('NetChat', { body: message });
    }
}

// In-page toast notification
function showToast(text, duration = 6000) {
    // Allow longer messages for encryption help, but still cap
    const MAX = text.includes('ENCRYPTION') ? 500 : 200;
    let truncated = text;
    if (text.length > MAX) truncated = text.slice(0, MAX - 1) + '…';

    const toast = document.createElement('div');
    toast.className = 'netchat-toast';
    // Support multi-line with <br>
    const formattedText = truncated.replace(/\\n/g, '<br>');
    toast.innerHTML = `
        <span class="toast-text">${formattedText}</span>
        <button class="toast-dismiss" aria-label="Dismiss">✕</button>
    `;

    document.body.appendChild(toast);
    // Trigger show (allow CSS transition)
    setTimeout(() => toast.classList.add('show'), 10);

    const removeToast = () => {
        toast.classList.remove('show');
        setTimeout(() => { try { toast.remove(); } catch(e){} }, 300);
    };

    // Dismiss button
    const dbtn = toast.querySelector('.toast-dismiss');
    if (dbtn) dbtn.addEventListener('click', (e) => { e.stopPropagation(); removeToast(); });

    // Auto remove after specified duration
    const timer = setTimeout(removeToast, duration);

    // Pause auto-hide on hover
    toast.addEventListener('mouseenter', () => clearTimeout(timer));
}

// PM Modal Event Listeners
imageBtn.onclick = () => imageInput.click();

imageInput.addEventListener('change', async (e) => {
    const file = e.target.files[0];
    if (!file) return;

    // Validate file size (5MB max)
    if (file.size > 5 * 1024 * 1024) {
        showToast('❌ Image too large. Maximum size is 5MB.');
        imageInput.value = '';
        return;
    }

    // Validate file type
    const allowedTypes = ['image/jpeg', 'image/jpg', 'image/png', 'image/gif', 'image/webp'];
    if (!allowedTypes.includes(file.type)) {
        showToast('❌ Invalid file type. Only JPEG, PNG, GIF, and WebP are allowed.');
        imageInput.value = '';
        return;
    }

    // Show uploading indicator
    imageBtn.disabled = true;
    imageBtn.textContent = '⏳';

    // Upload image
    const imageUrl = await uploadImage(file);
    
    // Reset button
    imageBtn.disabled = false;
    imageBtn.textContent = '📷';
    imageInput.value = '';

    if (imageUrl && currentRoom) {
        // Send image message
        const message = messageInput.value.trim(); // Optional caption
        socket.emit('message:send', {
            message: message || '',
            room: currentRoom,
            imageUrl: imageUrl
        });
        messageInput.value = '';
        showToast('✅ Image sent!');
    }
});

// PM Image Upload
pmImageBtn.onclick = () => pmImageInput.click();

pmImageInput.addEventListener('change', async (e) => {
    const file = e.target.files[0];
    if (!file) return;

    // Validate file size (5MB max)
    if (file.size > 5 * 1024 * 1024) {
        showToast('❌ Image too large. Maximum size is 5MB.');
        pmImageInput.value = '';
        return;
    }

    // Validate file type
    const allowedTypes = ['image/jpeg', 'image/jpg', 'image/png', 'image/gif', 'image/webp'];
    if (!allowedTypes.includes(file.type)) {
        showToast('❌ Invalid file type. Only JPEG, PNG, GIF, and WebP are allowed.');
        pmImageInput.value = '';
        return;
    }

    // Show uploading indicator
    pmImageBtn.disabled = true;
    pmImageBtn.textContent = '⏳';

    // Upload image
    const imageUrl = await uploadImage(file);
    
    // Reset button
    pmImageBtn.disabled = false;
    pmImageBtn.textContent = '📷';
    pmImageInput.value = '';

    if (imageUrl && currentPMUser) {
        // Store image for sending with optional caption
        pmImageUrl = imageUrl;
        showToast('✅ Image ready to send. Add a caption (optional) and click Send!', 4000);
        pmInput.focus();
    }
});

pmCloseBtn.onclick = closePMChat;
pmSendBtn.onclick = sendPrivateMessage;
pmInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
        e.preventDefault();
        sendPrivateMessage();
    }
});

// Close modal when clicking outside
pmModal.addEventListener('click', (e) => {
    if (e.target === pmModal) {
        closePMChat();
    }
});

// PM Encryption toggle button
pmEncryptBtn.addEventListener('click', () => {
    if (!isPMEncryptionEnabled) {
        // Enabling PM encryption - ask for password
        const password = prompt('🔐 Enter encryption password for private messages:\n\nThis password will be used to encrypt your PM.\nShare this password with the recipient to read encrypted messages.\n\nMinimum 4 characters.');
        
        if (!password) {
            showToast('❌ PM Encryption cancelled - no password provided', 3000);
            return;
        }
        
        if (password.length < 4) {
            showToast('❌ Password too short - minimum 4 characters', 3000);
            return;
        }
        
        pmEncryptionPassword = password;
        isPMEncryptionEnabled = true;
        pmEncryptBtn.textContent = '🔐';
        pmEncryptBtn.title = 'PM Encryption ON - Messages are encrypted with AES-256. Click to disable.';
        pmEncryptBtn.classList.add('active');
        showToast('🔐 PM Encryption ENABLED\n\nYour private messages will be encrypted.\nShare password with recipient to decrypt.', 5000);
    } else {
        // Disabling PM encryption
        isPMEncryptionEnabled = false;
        pmEncryptionPassword = null;
        pmEncryptBtn.textContent = '🔓';
        pmEncryptBtn.title = 'PM Encryption OFF - Messages sent as plain text. Click to enable encryption.';
        pmEncryptBtn.classList.remove('active');
        showToast('🔓 PM Encryption DISABLED\n\nMessages will be sent as plain text.', 3000);
    }
});

// Encryption toggle button
encryptBtn.addEventListener('click', () => {
    if (!isEncryptionEnabled) {
        // Enabling encryption - ask for password
        const password = prompt('🔐 Enter encryption password:\n\nThis password will be used to encrypt your messages.\nShare this password with people who should be able to read your encrypted messages.');
        
        if (!password) {
            showToast('❌ Encryption cancelled - no password provided', 3000);
            return;
        }
        
        if (password.length < 4) {
            showToast('❌ Password too short - minimum 4 characters', 3000);
            return;
        }
        
        encryptionPassword = password;
        isEncryptionEnabled = true;
        encryptBtn.textContent = '🔐';
        encryptBtn.title = 'Encryption ON - Messages are encrypted with AES-256. Click to disable.';
        encryptBtn.classList.add('active');
        showToast('🔐 Encryption ENABLED\n\nYour messages will be encrypted with your password.\nOthers need the same password to decrypt.', 5000);
    } else {
        // Disabling encryption
        isEncryptionEnabled = false;
        encryptionPassword = null;
        encryptBtn.textContent = '🔓';
        encryptBtn.title = 'Encryption OFF - Messages sent as plain text. Click to enable encryption.';
        encryptBtn.classList.remove('active');
        showToast('🔓 Encryption DISABLED\n\nMessages will be sent as plain text.', 3000);
    }
});

// Decrypt PM message function
window.decryptPMMessage = async function(bubbleElement) {
    const encryptedText = bubbleElement.getAttribute('data-encrypted-text');
    
    if (!encryptedText) {
        showToast('❌ No encrypted message found', 2000);
        return;
    }
    
    // Check if already decrypted
    if (bubbleElement.classList.contains('decrypted')) {
        return;
    }
    
    const password = prompt('🔓 Enter decryption password:\n\nEnter the password used to encrypt this message.');
    
    if (!password) {
        showToast('❌ Decryption cancelled', 2000);
        return;
    }
    
    try {
        const token = localStorage.getItem('token');
        const response = await fetch('/api/decrypt', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${token}`
            },
            body: JSON.stringify({
                encryptedMessage: encryptedText,
                password: password
            })
        });
        
        const data = await response.json();
        
        if (data.success) {
            // Update the bubble with decrypted content
            bubbleElement.innerHTML = `
                <span class="lock-icon">🔓</span>
                <span class="decrypted-text">${escapeHTML(data.decryptedMessage)}</span>
                <div class="pm-msg-time">${bubbleElement.querySelector('.pm-msg-time').textContent}</div>
            `;
            bubbleElement.classList.add('decrypted');
            bubbleElement.classList.remove('pm-encrypted-bubble');
            bubbleElement.onclick = null; // Remove click handler
            showToast('✅ Message decrypted successfully', 2000);
        } else {
            showToast('❌ ' + (data.message || 'Decryption failed - wrong password?'), 3000);
        }
    } catch (error) {
        console.error('Decryption error:', error);
        showToast('❌ Decryption failed - please try again', 3000);
    }
};

window.decryptMessage = async function(bubbleElement) {
    const encryptedText = bubbleElement.getAttribute('data-encrypted-text');
    
    if (!encryptedText) {
        showToast('❌ No encrypted message found', 2000);
        return;
    }
    
    // Check if already decrypted
    if (bubbleElement.classList.contains('decrypted')) {
        return;
    }
    
    const password = prompt('🔓 Enter decryption password:\n\nEnter the password used to encrypt this message.');
    
    if (!password) {
        showToast('❌ Decryption cancelled', 2000);
        return;
    }
    
    try {
        const token = localStorage.getItem('token');
        const response = await fetch('/api/decrypt', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${token}`
            },
            body: JSON.stringify({
                encryptedMessage: encryptedText,
                password: password
            })
        });
        
        const data = await response.json();
        
        if (data.success) {
            // Update the bubble with decrypted content
            bubbleElement.innerHTML = `
                <span class="lock-icon">🔓</span>
                <span class="decrypted-text">${escapeHTML(data.decryptedMessage)}</span>
                <div class="message-time">${bubbleElement.querySelector('.message-time').textContent}</div>
            `;
            bubbleElement.classList.add('decrypted');
            bubbleElement.classList.remove('encrypted-message');
            bubbleElement.onclick = null; // Remove click handler
            showToast('✅ Message decrypted successfully', 2000);
        } else {
            showToast('❌ ' + (data.message || 'Decryption failed - wrong password?'), 3000);
        }
    } catch (error) {
        console.error('Decryption error:', error);
        showToast('❌ Decryption failed - please try again', 3000);
    }
};

// Add double-click for encryption help
encryptBtn.addEventListener('dblclick', (e) => {
    e.preventDefault();
    const helpText = `🔐 PASSWORD-BASED ENCRYPTION\n\n` +
        `HOW IT WORKS:\n` +
        `1. Click 🔓 and enter a password\n` +
        `2. Your messages will be encrypted with that password\n` +
        `3. Share the password with who should read them\n` +
        `4. Recipients click encrypted messages to decrypt\n\n` +
        `ENCRYPTION DETAILS:\n` +
        `• Algorithm: AES-256-CBC\n` +
        `• Key Derivation: Scrypt\n` +
        `• Password-based symmetric encryption\n` +
        `• Only users with the password can decrypt\n\n` +
        `USE CASE:\n` +
        `Send sensitive info in group chats.\nEveryone with the password can read it.\nOthers see only encrypted text.`;
    
    alert(helpText);
});

// Request notification permission
if (Notification.permission === 'default') {
    Notification.requestPermission();
}

// Periodically refresh rooms list
setInterval(() => {
    socket.emit('rooms:get');
}, 5000);

console.log('💬 Chat client initialized');

// ===== PM Encryption Handlers =====
if (pmEncryptBtn) {
    // PM Encryption toggle
    pmEncryptBtn.addEventListener('click', () => {
        isPMEncryptionEnabled = !isPMEncryptionEnabled;
        
        if (isPMEncryptionEnabled) {
            pmEncryptBtn.textContent = '????';
            pmEncryptBtn.title = 'PM Encryption ON - Messages are encrypted. Click to disable.';
            pmEncryptBtn.classList.add('active');
            showToast('???? PM Encryption ENABLED\n\nYour private messages will be encrypted.', 4000);
        } else {
            pmEncryptBtn.textContent = '????';
            pmEncryptBtn.title = 'PM Encryption OFF - Messages sent as plain text. Click to enable.';
            pmEncryptBtn.classList.remove('active');
            showToast('???? PM Encryption DISABLED', 2000);
        }
    });

    // PM Encryption double-click help
    pmEncryptBtn.addEventListener('dblclick', (e) => {
        e.preventDefault();
        const helpText = `???? PRIVATE MESSAGE ENCRYPTION\n\n` +
            `This encrypts YOUR private messages to a specific user.\n` +
            `Uses AES-256-CBC encryption algorithm.\n\n` +
            `???? = Encryption ON\n` +
            `???? = Encryption OFF`;
        alert(helpText);
    });
}
