#!/bin/bash

# Script to run tests for the Manna project

# Define the project root directory
PROJECT_ROOT="$HOME/dev/Manna"

# Define the path to the tests directory
TESTS_DIR="$PROJECT_ROOT/bin/debug-linux-x86_64/tests"

# Define the test executable
TEST_EXEC="$TESTS_DIR/tests"

# Check if the tests directory exists
if [ ! -d "$TESTS_DIR" ]; then
    echo "Error: Tests directory not found at $TESTS_DIR"
    exit 1
fi

# Check if the test executable exists
if [ ! -f "$TEST_EXEC" ]; then
    echo "Error: Test executable not found at $TEST_EXEC"
    exit 1
fi

# Set the LD_LIBRARY_PATH to include the tests directory (for libmanna_engine.so)
export LD_LIBRARY_PATH="$TESTS_DIR:$LD_LIBRARY_PATH"

# Navigate to the tests directory
cd "$TESTS_DIR" || {
    echo "Error: Could not change to directory $TESTS_DIR"
    exit 1
}

# Run the tests
echo "Running tests..."
./tests

# Check the exit status of the test run
if [ $? -eq 0 ]; then
    echo "Tests completed successfully!"
else
    echo "Tests failed with exit code $?"
    exit 1
fi
