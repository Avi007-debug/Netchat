#!/bin/bash

# Test script to verify message broadcasting between clients

echo "Testing Enhanced Server Message Broadcasting"
echo "============================================="
echo ""

# Start Client 1 in background
(
  sleep 1
  echo "alice"
  echo "pass123"
  sleep 1
  echo "Hello from Alice!"
  sleep 2
  echo "/quit"
) | nc localhost 5555 > /tmp/client1.log 2>&1 &
CLIENT1_PID=$!

# Start Client 2 in background
(
  sleep 2
  echo "bob"
  echo "pass123"
  sleep 1
  echo "Hello from Bob!"
  sleep 2
  echo "/quit"
) | nc localhost 5555 > /tmp/client2.log 2>&1 &
CLIENT2_PID=$!

# Wait for clients to finish
sleep 6

echo "Client 1 output:"
echo "==============="
cat /tmp/client1.log
echo ""
echo ""

echo "Client 2 output:"
echo "==============="
cat /tmp/client2.log
echo ""

# Check if Alice's message appeared in Bob's output
if grep -q "alice: Hello from Alice" /tmp/client2.log; then
  echo "✅ SUCCESS: Bob received Alice's message!"
else
  echo "❌ FAIL: Bob did NOT receive Alice's message"
fi

# Check if Bob's message appeared in Alice's output
if grep -q "bob: Hello from Bob" /tmp/client1.log; then
  echo "✅ SUCCESS: Alice received Bob's message!"
else
  echo "❌ FAIL: Alice did NOT receive Bob's message"
fi

# Cleanup
rm -f /tmp/client1.log /tmp/client2.log
