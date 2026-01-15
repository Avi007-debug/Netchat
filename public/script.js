// API Configuration
const API_BASE_URL = 'http://localhost:3000/api';

// Toggle between login and register forms
function toggleForms() {
    const loginForm = document.getElementById('loginForm');
    const registerForm = document.getElementById('registerForm');
    
    loginForm.classList.toggle('hidden');
    registerForm.classList.toggle('hidden');
    
    // Clear all messages and errors
    clearAllMessages();
}

// Clear all error and success messages
function clearAllMessages() {
    document.querySelectorAll('.error-message').forEach(el => el.textContent = '');
    document.querySelectorAll('.message').forEach(el => {
        el.textContent = '';
        el.className = 'message';
    });
}

// Toggle password visibility
function togglePassword(inputId) {
    const input = document.getElementById(inputId);
    if (input.type === 'password') {
        input.type = 'text';
    } else {
        input.type = 'password';
    }
}

// Check password strength
function checkPasswordStrength(password) {
    let strength = 0;
    
    if (password.length >= 8) strength++;
    if (password.match(/[a-z]/) && password.match(/[A-Z]/)) strength++;
    if (password.match(/\d/)) strength++;
    if (password.match(/[^a-zA-Z\d]/)) strength++;
    
    const strengthEl = document.getElementById('passwordStrength');
    if (strengthEl) {
        strengthEl.className = 'password-strength';
        if (strength === 1) strengthEl.classList.add('weak');
        else if (strength === 2) strengthEl.classList.add('medium');
        else if (strength >= 3) strengthEl.classList.add('strong');
    }
}

// Register form handler
document.getElementById('registerFormElement')?.addEventListener('submit', async (e) => {
    e.preventDefault();
    clearAllMessages();
    
    const username = document.getElementById('registerUsername').value.trim();
    const email = document.getElementById('registerEmail').value.trim();
    const password = document.getElementById('registerPassword').value;
    const confirmPassword = document.getElementById('registerConfirmPassword').value;
    
    // Client-side validation
    if (username.length < 3) {
        document.getElementById('registerUsernameError').textContent = 'Username must be at least 3 characters';
        return;
    }
    
    if (password.length < 6) {
        document.getElementById('registerPasswordError').textContent = 'Password must be at least 6 characters';
        return;
    }
    
    if (password !== confirmPassword) {
        document.getElementById('registerConfirmPasswordError').textContent = 'Passwords do not match';
        return;
    }
    
    // Show loading state
    const submitBtn = document.getElementById('registerSubmitBtn');
    submitBtn.classList.add('loading');
    
    try {
        const response = await fetch(`${API_BASE_URL}/auth/register`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                username,
                email,
                password,
                confirmPassword
            })
        });
        
        const data = await response.json();
        
        if (data.success) {
            // Store token
            localStorage.setItem('token', data.token);
            localStorage.setItem('user', JSON.stringify(data.user));
            
            // Show success modal
            showSuccessModal(
                'Account Created!',
                `Welcome ${data.user.username}! Your account has been created successfully.`
            );
            
            // Reset form
            document.getElementById('registerFormElement').reset();
        } else {
            // Show error message
            const messageEl = document.getElementById('registerMessage');
            messageEl.className = 'message error';
            
            if (data.errors && Array.isArray(data.errors)) {
                const errorText = data.errors.map(e => e.msg).join(', ');
                messageEl.textContent = errorText;
            } else {
                messageEl.textContent = data.message || 'Registration failed. Please try again.';
            }
        }
    } catch (error) {
        console.error('Registration error:', error);
        const messageEl = document.getElementById('registerMessage');
        messageEl.className = 'message error';
        messageEl.textContent = 'Connection error. Please check your internet and try again.';
    } finally {
        submitBtn.classList.remove('loading');
    }
});

// Login form handler
document.getElementById('loginFormElement')?.addEventListener('submit', async (e) => {
    e.preventDefault();
    clearAllMessages();
    
    const email = document.getElementById('loginEmail').value.trim();
    const password = document.getElementById('loginPassword').value;
    
    // Show loading state
    const submitBtn = document.getElementById('loginSubmitBtn');
    submitBtn.classList.add('loading');
    
    try {
        const response = await fetch(`${API_BASE_URL}/auth/login`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                email,
                password
            })
        });
        
        const data = await response.json();
        
        if (data.success) {
            // Store token and user info
            localStorage.setItem('token', data.token);
            localStorage.setItem('user', JSON.stringify(data.user));
            
            // Show success modal
            showSuccessModal(
                'Welcome Back!',
                `Login successful. Redirecting to chat...`
            );
            
            // Reset form
            document.getElementById('loginFormElement').reset();
        } else {
            // Show error message
            const messageEl = document.getElementById('loginMessage');
            messageEl.className = 'message error';
            messageEl.textContent = data.message || 'Login failed. Please try again.';
        }
    } catch (error) {
        console.error('Login error:', error);
        const messageEl = document.getElementById('loginMessage');
        messageEl.className = 'message error';
        messageEl.textContent = 'Connection error. Please check your internet and try again.';
    } finally {
        submitBtn.classList.remove('loading');
    }
});

// Password strength checker for register form
document.getElementById('registerPassword')?.addEventListener('input', (e) => {
    checkPasswordStrength(e.target.value);
});

// Show success modal
function showSuccessModal(title, message) {
    document.getElementById('successTitle').textContent = title;
    document.getElementById('successMessage').textContent = message;
    document.getElementById('successModal').classList.remove('hidden');
}

// Close modal
function closeModal() {
    document.getElementById('successModal').classList.add('hidden');
    
    // Redirect to chat (you can replace this with your actual chat page)
    const user = JSON.parse(localStorage.getItem('user'));
    if (user) {
        // Redirect to chat page after 1 second
        setTimeout(() => {
            window.location.href = '/chat.html';
        }, 1000);
    }
}

// Check if user is already logged in
window.addEventListener('load', () => {
    const token = localStorage.getItem('token');
    const user = localStorage.getItem('user');
    
    if (token && user) {
        // User is already logged in
        console.log('User already logged in:', JSON.parse(user));
        // You can redirect to chat page or do something else
    }
});

// Add form validation on input
const inputs = document.querySelectorAll('input[type="email"], input[type="text"], input[type="password"]');
inputs.forEach(input => {
    input.addEventListener('blur', () => {
        input.classList.remove('valid');
        
        if (input.value.trim() && input.checkValidity()) {
            input.classList.add('valid');
        }
    });
});

// Prevent form submission on enter for password fields
document.querySelectorAll('.password-wrapper input').forEach(input => {
    input.addEventListener('keypress', (e) => {
        if (e.key === 'Enter' && input.type === 'password') {
            e.preventDefault();
        }
    });
});
