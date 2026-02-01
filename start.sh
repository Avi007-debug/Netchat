#!/bin/bash

echo "╔════════════════════════════════════════╗"
echo "║     NetChat - Dual Mode Chat App      ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Choose mode:"
echo "  1) C Socket Server (port 8080)"
echo "  2) Web Server (port 3000)"
echo ""
read -p "Enter choice (1 or 2): " choice

case $choice in
    1)
        echo ""
        echo "🔨 Building C server and client..."
        make all
        echo ""
        echo "🚀 Starting C server..."
        echo "   Open new terminals and run: make run-client"
        make run-server
        ;;
    2)
        echo ""
        echo "📦 Installing Node.js dependencies..."
        npm install
        echo ""
        echo "🌐 Starting web server..."
        echo "   Open browser: http://localhost:3000"
        npm start
        ;;
    *)
        echo "Invalid choice. Exiting."
        exit 1
        ;;
esac
