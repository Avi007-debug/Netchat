#!/bin/bash

# NetChat Web Authentication System Installation & Setup

echo "🚀 NetChat Web Authentication System"
echo "====================================="
echo ""

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo "❌ Node.js is not installed. Please install Node.js first."
    exit 1
fi

echo "✅ Node.js is installed: $(node --version)"
echo "✅ npm is installed: $(npm --version)"
echo ""

# Navigate to project directory
cd "$(dirname "$0")" || exit

echo "📦 Installing dependencies..."
npm install

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Installation completed successfully!"
    echo ""
    echo "🎯 Next steps:"
    echo "1. Start the server: npm start"
    echo "2. Open http://localhost:3000 in your browser"
    echo "3. Create an account or login"
    echo ""
    echo "📚 Documentation:"
    echo "   - AUTH_SETUP.md     - Detailed API documentation"
    echo "   - QUICKSTART.md     - Quick start guide"
    echo ""
else
    echo "❌ Installation failed. Please check the error messages above."
    exit 1
fi
