CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2
TARGET_SERVER = server/server
TARGET_CLIENT = client/client
SRC_SERVER = server/server.c
SRC_CLIENT = client/client.c

.PHONY: all server client clean run-server run-client web reset help

all: server client
	@echo "✅ Build complete!"
	@echo "Run 'make run-server' for C server or 'make web' for Node.js web server"

server:
	@echo "🔨 Compiling C server..."
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SRC_SERVER)
	@echo "✅ Server compiled successfully!"

client:
	@echo "🔨 Compiling C client..."
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(SRC_CLIENT)
	@echo "✅ Client compiled successfully!"

run-server: server
	@echo "🚀 Starting C server on port 8080..."
	@cd server && ./server

run-client: client
	@echo "🚀 Starting C client..."
	@cd client && ./client

web:
	@echo "🌐 Starting web server on port 3000..."
	@echo "Install dependencies with: npm install"
	@echo "Then run: npm start"
	@npm start

clean:
	@echo "🧹 Cleaning up..."
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT) chat.log users.txt
	@echo "✅ Cleanup complete!"

reset: clean all
	@echo "✅ Fresh build ready!"

help:
	@echo "NetChat - Makefile Commands"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo "C SERVER (Socket-based):"
	@echo "  make all         - Compile C server and client"
	@echo "  make server      - Compile only C server"
	@echo "  make client      - Compile only C client"
	@echo "  make run-server  - Run C server (port 8080)"
	@echo "  make run-client  - Run C client"
	@echo ""
	@echo "WEB SERVER (Node.js + WebSocket):"
	@echo "  make web         - Run web server (port 3000)"
	@echo ""
	@echo "UTILITIES:"
	@echo "  make clean       - Remove binaries and logs"
	@echo "  make reset       - Clean and rebuild"
	@echo "  make help        - Show this help"
