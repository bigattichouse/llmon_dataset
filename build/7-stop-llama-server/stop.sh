#!/bin/bash

LLAMA_PORT=8080  # Adjust port as needed
SERVER_LOG="llama_server.log"
OUTPUT_DIR="responses"
LLAMA_SERVER_PATH=${LLAMA_SERVER_PATH:-"llama-server"}  # Default path if not provided
LLAMA_MODEL_PATH=${LLAMA_MODEL_PATH:-"models/model.gguf"}  # Default path if not provided
LLAMA_CONTEXT_SIZE=${LLAMA_CONTEXT_SIZE:-"32500"} #default for my graphics card

# Function to check if llama-server is running
check_server() {
    if pgrep -f "llama-server" > /dev/null; then
        return 0
    else
        return 1
    fi
}

# Function to stop the server
stop_server() {
    echo "Stopping llama-server..."
    pkill -f "llama-server"
    sleep 2
}

# Main script
main() {
    # Check if server is running, start if not
    if ! check_server; then
        exit;
    fi
    # Stop server after processing all files
    stop_server
}

# Run main function with all command line arguments
main "$@"

