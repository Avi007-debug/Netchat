# 🔐 Password-Based Encryption Guide

## Overview

NetChat now supports **password-based symmetric encryption** for secure messaging. Only users with the correct password can decrypt and read encrypted messages.

---

## How It Works

### **For Senders:**

1. **Enable Encryption**
   - Click the 🔓 button next to the message input
   - Enter an encryption password (minimum 4 characters)
   - Button changes to 🔐 indicating encryption is ON

2. **Send Encrypted Message**
   - Type your message normally
   - Click Send
   - Message is encrypted with your password before broadcasting
   - You see your own message as normal (purple bubble)
   - Others see encrypted black message with 🔐 icon

3. **Disable Encryption**
   - Click 🔐 button again
   - Returns to 🔓 (OFF state)
   - Future messages sent as plain text

4. **Share Password**
   - Share your encryption password with intended recipients
   - Use a secure channel (not in the chat!)
   - Examples: WhatsApp, phone call, in-person

---

### **For Recipients:**

1. **See Encrypted Message**
   - Encrypted messages appear as black bubbles
   - Show 🔐 icon and text "Encrypted message - Click to decrypt"
   - Hover shows tooltip: "🔒 Click to decrypt with password"

2. **Decrypt Message**
   - Click on the encrypted message bubble
   - Enter the password (shared by sender)
   - If correct: Message decrypts and shows content
   - If wrong: Error message "Wrong password or corrupted message"

3. **After Decryption**
   - Bubble turns green with 🔓 icon
   - Decrypted text is displayed
   - Content persists until page refresh

---

## Technical Details

### **Encryption Algorithm**
- **Algorithm:** AES-256-CBC
- **Key Derivation:** Scrypt with fixed salt
- **IV:** Random 16-byte initialization vector per message
- **Format:** `{iv_hex}:{encrypted_hex}`

### **Security Features**
- ✅ Password-based symmetric encryption
- ✅ Unique IV for each message
- ✅ Server-side encryption/decryption
- ✅ Password never stored or logged
- ✅ Failed decryption doesn't crash app

### **Limitations**
- ❌ NOT end-to-end encrypted (server can decrypt with password)
- ❌ Password shared manually (not via key exchange)
- ❌ Same password for all recipients (symmetric)
- ❌ No password strength requirements beyond 4 chars
- ❌ Decrypted messages lost on page refresh

---

## Use Cases

### ✅ **Good Use Cases**
1. **Private group discussions** - Share password with specific group
2. **Sensitive information** - API keys, credentials, addresses
3. **Confidential planning** - Surprise parties, gifts, events
4. **Temporary secrets** - One-time access codes
5. **Team collaboration** - Project-specific encrypted channels

### ❌ **Not Recommended For**
1. **Highly classified data** - Use proper E2EE systems
2. **Long-term secrets** - Password may be compromised over time
3. **Legal communications** - Needs audit trail and compliance
4. **Large files** - Only text messages supported
5. **Public channels** - Everyone can see there's a secret

---

## Example Workflow

```
Alice in #dev-team:
1. Clicks 🔓
2. Enters password: "project-alpha-2026"
3. Sends: "The staging API key is abc123xyz"
4. Bob, Carol, Dave see black encrypted bubble

Bob in #dev-team:
1. Clicks the encrypted message
2. Enters: "project-alpha-2026"
3. Sees: "The staging API key is abc123xyz"
4. Bubble turns green ✅

Eve (unauthorized):
1. Clicks the encrypted message
2. Enters wrong password
3. Sees: "Wrong password or corrupted message"
4. Cannot read the message ❌
```

---

## Security Warnings

⚠️ **Important Limitations:**

1. **Server Access**: The server CAN decrypt messages if it has the password
2. **Password Sharing**: Must share password via secure out-of-band channel
3. **No Key Rotation**: Same password used until manually changed
4. **No Forward Secrecy**: If password leaked, all past messages compromised
5. **Client Trust**: Assumes no malicious client modifications

---

## Best Practices

### **For Maximum Security:**

✅ **DO:**
- Use strong, unique passwords (12+ characters)
- Share passwords via secure channels (Signal, WhatsApp, in-person)
- Rotate passwords periodically
- Use different passwords for different topics
- Verify recipients before sharing password

❌ **DON'T:**
- Share passwords in the same chat room
- Use simple passwords like "1234"
- Reuse passwords across multiple chats
- Assume complete privacy (server can log)
- Share passwords with untrusted users

---

## Comparison with E2EE

| Feature | NetChat Encryption | True E2EE (Signal, WhatsApp) |
|---------|-------------------|------------------------------|
| **Server Access** | Can decrypt with password | Cannot decrypt (no keys) |
| **Key Exchange** | Manual password sharing | Automatic (Diffie-Hellman) |
| **Forward Secrecy** | No | Yes (new keys per session) |
| **Ease of Use** | Very simple | Transparent to user |
| **Use Case** | Group secrets | All conversations |
| **Security Level** | Medium | High |

---

## Troubleshooting

### **Cannot Enable Encryption**
- ✅ Make sure you're in a room
- ✅ Check that message input is enabled
- ✅ Enter password with 4+ characters

### **Decryption Fails**
- ✅ Verify password with sender
- ✅ Check for typos (case-sensitive)
- ✅ Ensure message wasn't corrupted
- ✅ Try refreshing and re-decrypting

### **Password Forgotten**
- ❌ No password recovery possible
- ❌ Encrypted messages permanently unreadable
- ✅ Contact sender for password reminder

---

## Future Improvements

Planned enhancements for encryption:

1. **Client-Side Encryption** - True E2EE with no server access
2. **Public Key Infrastructure** - Automatic key exchange
3. **Per-User Encryption** - Different keys per recipient
4. **Forward Secrecy** - Ephemeral keys per session
5. **Password Strength Meter** - Visual feedback on password quality
6. **Encrypted File Sharing** - Extend to images and documents
7. **Audit Logging** - Track encryption/decryption attempts

---

## Developer Notes

### **Server Implementation** (server.js)
```javascript
// Encrypt with password-derived key
function encryptMessage(text, password) {
  const key = crypto.scryptSync(password, 'netchat-salt-v1', 32);
  const iv = crypto.randomBytes(16);
  const cipher = crypto.createCipheriv('aes-256-cbc', key, iv);
  let encrypted = cipher.update(text, 'utf8', 'hex');
  encrypted += cipher.final('hex');
  return iv.toString('hex') + ':' + encrypted;
}

// Decrypt with same password
function decryptMessage(text, password) {
  const parts = text.split(':');
  const iv = Buffer.from(parts[0], 'hex');
  const encryptedText = Buffer.from(parts[1], 'hex');
  const key = crypto.scryptSync(password, 'netchat-salt-v1', 32);
  const decipher = crypto.createDecipheriv('aes-256-cbc', key, iv);
  let decrypted = decipher.update(encryptedText, 'hex', 'utf8');
  decrypted += decipher.final('utf8');
  return decrypted;
}
```

### **Client Implementation** (chat.js)
```javascript
// Click handler for encrypted messages
window.decryptMessage = async function(bubbleElement) {
  const encryptedText = bubbleElement.getAttribute('data-encrypted-text');
  const password = prompt('Enter decryption password:');
  
  const response = await fetch('/api/decrypt', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${token}`
    },
    body: JSON.stringify({ encryptedMessage: encryptedText, password })
  });
  
  const data = await response.json();
  if (data.success) {
    // Update UI with decrypted text
  }
}
```

---

## Conclusion

This password-based encryption provides a **practical balance** between security and usability for group chat scenarios. While not as secure as true E2EE, it's simple to use and effective for protecting sensitive information from unauthorized viewers.

**Remember:** This is a demonstration feature for educational purposes. For production systems handling truly sensitive data, implement proper end-to-end encryption with public key infrastructure.
