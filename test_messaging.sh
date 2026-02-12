#!/bin/bash

echo "=== Testing Enhanced Server Multi-Client Chat ==="
echo ""

# Client 1 - Alice
(
    sleep 1
    echo "alice"
    echo "testpass"
    sleep 2
    echo "Hello from Alice!"
    sleep 4
    echo "/quit"
) | nc localhost 5555 > /tmp/alice_test.log 2>&1 &
ALICE_PID=$!

# Client 2 - Bob  
(
    sleep 2
    echo "bob"
    echo "pass123"
    sleep 2
    echo "Hi Alice, Bob here!"
    sleep 3
    echo "/quit"
) | nc localhost 5555 > /tmp/bob_test.log 2>&1 &
BOB_PID=$!

# Wait for both clients to finish
wait $ALICE_PID
wait $BOB_PID

echo "=== ALICE'S OUTPUT ==="
cat /tmp/alice_test.log | tail -20
echo ""
echo ""
echo "=== BOB'S OUTPUT ==="
cat /tmp/bob_test.log | tail -20
echo ""
echo ""

# Check results
echo "=== TEST RESULTS ==="
if grep -q "bob: Hi Alice, Bob here" /tmp/alice_test.log; then
    echo "✅ SUCCESS: Alice received Bob's message!"
else
    echo "❌ FAIL: Alice did NOT receive Bob's message"
fi

if grep -q "alice: Hello from Alice" /tmp/bob_test.log; then
    echo "✅ SUCCESS: Bob received Alice's message!"
else
    echo "❌ FAIL: Bob did NOT receive Alice's message"
fi

# Cleanup
rm -f /tmp/alice_test.log /tmp/bob_test.log
