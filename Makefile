CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2
LDFLAGS = -lpthread -lrt
DEBUG_FLAGS = -g -DDEBUG
TARGET_SERVER = server/server
TARGET_SERVER_ENHANCED = server/server_enhanced
TARGET_CLIENT = client/client
SRC_SERVER = server/server.c
SRC_SERVER_ENHANCED = server/server_enhanced.c
SRC_CLIENT = client/client.c

.PHONY: all server client enhanced debug clean run-server run-client run-enhanced web reset help install

all: server client
	@echo "✅ Build complete!"
	@echo "Run 'make run-server' for C server or 'make web' for Node.js web server"
	@echo "Run 'make enhanced' for OS-enhanced server with shared memory, message queues, etc."

server:
	@echo "🔨 Compiling C server..."
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SRC_SERVER)
	@echo "✅ Server compiled successfully!"

enhanced:
	@echo "🔨 Compiling enhanced C server with OS features..."
	@echo "   Features: Shared Memory, Message Queues, Process Forking, Semaphores"
	$(CC) $(CFLAGS) $(SRC_SERVER_ENHANCED) -o $(TARGET_SERVER_ENHANCED) $(LDFLAGS)
	@echo "✅ Enhanced server compiled successfully!"

debug: $(SRC_SERVER_ENHANCED)
	@echo "🔨 Compiling enhanced C server in DEBUG mode..."
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRC_SERVER_ENHANCED) -o $(TARGET_SERVER_ENHANCED)_debug $(LDFLAGS)
	@echo "✅ Debug build complete! Run with: cd server && ./server_enhanced_debug"

client:
	@echo "🔨 Compiling C client..."
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(SRC_CLIENT)
	@echo "✅ Client compiled successfully!"

run-server: server
	@echo "🚀 Starting C server on port 8080..."
	@cd server && ./server

run-enhanced: enhanced
	@echo "🚀 Starting enhanced C server on port 5555..."
	@echo "   OS Features Active: Shared Memory | Message Queues | Process Forking | Semaphores"
	@echo "🔧 Creating IPC key file..."
	@touch /tmp/netchat_key
	@cd server && ./server_enhanced

run-client: client
	@echo "🚀 Starting C client..."
	@echo "   Connecting to ENHANCED server (port 5555)"
	@echo "   For standard server (port 8080), run: CLIENT_PORT=8080 ./client/client"
	@./client/client

web:
	@echo "🌐 Starting web server on port 3000..."
	@echo "Install dependencies with: npm install"
	@echo "Then run: npm start"
	@npm start

install: enhanced
	@echo "📦 Installing NetChat enhanced server..."
	@sudo cp $(TARGET_SERVER_ENHANCED) /usr/local/bin/netchat-server
	@sudo chmod +x /usr/local/bin/netchat-server
	@echo "✅ Installed to /usr/local/bin/netchat-server"

clean:
	@echo "🧹 Cleaning up..."
	rm -f $(TARGET_SERVER) $(TARGET_SERVER_ENHANCED) $(TARGET_SERVER_ENHANCED)_debug $(TARGET_CLIENT) chat.log users.txt
	@echo "✅ Cleanup complete!"

reset: clean all
	@echo "✅ Fresh build ready!"

help:
	@echo "NetChat - Makefile Commands"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo "BUILD TARGETS:"
	@echo "  make all          - Compile standard C server and client (default)"
	@echo "  make server       - Compile only standard C server"
	@echo "  make client       - Compile only C client"
	@echo "  make enhanced     - Compile enhanced server with OS features"
	@echo "                      (Shared Memory, Message Queues, Forking, Semaphores)"
	@echo "  make debug        - Compile enhanced server with debug symbols"
	@echo ""
	@echo "RUN TARGETS:"
	@echo "  make run-server   - Compile and run standard C server (port 8080)"
	@echo "  make run-enhanced - Compile and run enhanced C server (port 5555)"
	@echo "  make run-client   - Compile and run C client"
	@echo "  make web          - Run Node.js web server (port 3000)"
	@echo ""
	@echo "INSTALLATION:"
	@echo "  make install      - Install enhanced server to /usr/local/bin"
	@echo ""
	@echo "UTILITIES:"
	@echo "  make clean        - Remove all binaries, logs, and temp files"
	@echo "  make reset        - Clean and rebuild all targets"
	@echo "  make help         - Show this help message"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo ""
	@echo "EXAMPLES:"
	@echo "  make && make run-server       # Build and run standard server"
	@echo "  make enhanced && make run-enhanced  # Build and run OS-enhanced server"
	@echo "  make web                      # Start web interface"

