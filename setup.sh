#!/bin/bash

# NetChat Quick Setup Script for Linux

echo "🔧 NetChat Setup"
echo "================="
echo ""

# Make scripts executable
echo "Making scripts executable..."
chmod +x start.sh

# Check if Node.js is installed
if command -v node &> /dev/null; then
    echo "✅ Node.js found: $(node --version)"
else
    echo "❌ Node.js not found"
    echo "   Install from: https://nodejs.org/"
fi

# Check if gcc is installed
if command -v gcc &> /dev/null; then
    echo "✅ GCC found: $(gcc --version | head -n1)"
else
    echo "❌ GCC not found"
    echo "   Install with: sudo apt-get install build-essential"
fi

echo ""
echo "📦 To install Node.js dependencies:"
echo "   npm install"
echo ""
echo "🚀 To start:"
echo "   ./start.sh (interactive)"
echo "   make run-server (C server)"
echo "   npm start (web server)"
echo ""
