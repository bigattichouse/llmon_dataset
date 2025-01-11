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

# Function to start the server
start_server() {
    echo "Starting llama-server..."
    
    if [[ ! -x "$LLAMA_SERVER_PATH" ]]; then
        echo "Error: llama-server executable not found or not executable at '$LLAMA_SERVER_PATH'."
        exit 1
    fi

    nohup "$LLAMA_SERVER_PATH" -m $LLAMA_MODEL_PATH --port $LLAMA_PORT -c $LLAMA_CONTEXT_SIZE > "$SERVER_LOG" 2>&1 &
    
    # Wait for server to start (adjust sleep time as needed)
    sleep 5
    
    if check_server; then
        echo "Server started successfully"
    else
        echo "Failed to start server"
        exit 1
    fi
}

# Function to stop the server
stop_server() {
    echo "Stopping llama-server..."
    pkill -f "llama-server"
    sleep 2
}

# Function to send prompt to server and save response
process_prompt() {
    local input_file=$1
    local output_file="$OUTPUT_DIR/$(basename "$input_file" .txt)_response.txt"
    
    # Create output directory if it doesn't exist
    mkdir -p "$OUTPUT_DIR"
    
    # Read prompt from file
    local prompt=$(cat "$input_file")
    
    # Send prompt to server and save response
    curl -X POST "http://localhost:$LLAMA_PORT/completion" \
        -H "Content-Type: application/json" \
        -d "{\\"prompt\\": \\"$prompt\\"}" \
        > "$output_file"
    
    echo "Processed $input_file -> $output_file"
}

# Main script
main() {
    # Check if server is running, start if not
    if ! check_server; then
        start_server
    fi
    
    # Process all input files
    for file in "$@"; do
        process_prompt "$file"
    done
    
    # Stop server after processing all files
    stop_server
}

# Run main function with all command line arguments
main "$@"

