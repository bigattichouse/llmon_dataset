#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
SOURCEFILES="../working/$1"

# Check if SOURCEFILES directory exists
if [ ! -d "$SOURCEFILES" ]; then
    echo "Error: Directory $SOURCEFILES does not exist"
    exit 1
fi

LLAMA_PORT=8080  # Adjust port as needed
SERVER_LOG="llama_server.log" 
LLAMA_SERVER_PATH=${LLAMA_SERVER_PATH:-"llama-server"}  # Default path if not provided
LLAMA_MODEL_PATH=${LLAMA_MODEL_PATH:-"models/model.gguf"}  # Default path if not provided
LLAMA_CONTEXT_SIZE=${LLAMA_CONTEXT_SIZE:-"32500"} #default for my graphics card
MAX_RETRIES=30  # Maximum number of retries
RETRY_DELAY=60  # Delay between retries in seconds

# Function to check if llama-server is running
check_server() {
    if pgrep -f "llama-server" > /dev/null; then
        return 0
    else
        return 1
    fi
}

# Function to send prompt to server and save response
process_prompt() {
    local input_file=$1
    local output_file="$input_file.response"
    echo "Processing $input_file...\n" 
    
    # Read prompt from file
    local prompt=$(cat "$input_file")
    
    # This will properly escape all special characters including quotes
    json_data=$(jq -n --arg p "$prompt" '{"prompt": $p}' | tr -d '\n')


    local retries=0
    while [ $retries -lt $MAX_RETRIES ]; do
        # Make the request and capture both stdout and stderr
        response=$(curl -s -w "\n%{http_code}" -X POST "http://localhost:$LLAMA_PORT/completion" \
            -H "Content-Type: application/json" \
            -d "$json_data")
        
        # Extract the HTTP status code
        http_code=$(echo "$response" | tail -n1)
        # Extract the response body (everything except the last line)
        body=$(echo "$response" | sed \$d)
        
        if [ "$http_code" = "503" ]; then
            retries=$((retries + 1))
            echo "Server returned 503 (Loading model). Retry $retries of $MAX_RETRIES..."
            if [ $retries -lt $MAX_RETRIES ]; then
                sleep $RETRY_DELAY
                continue
            else
                echo "Error: Maximum retries reached. Server still not ready."
                return 1
            fi
        elif [ "$http_code" = "200" ]; then
            echo "$body" > "$output_file"
            echo "Successfully processed $input_file -> $output_file"
            return 0
        else
            echo "Error: Unexpected response code: $http_code"
            echo "Response body: $body"
            return 1
        fi
    done
}

# Main script
main() {
    # Check if server is running, start if not
    if check_server; then
        # Process all input files
        #find "$SOURCEFILES" -type f \( -name "*.prompt"\) | while read -r file; do
        find "$SOURCEFILES" -type f -name "*.prompt" -print0 | while IFS= read -r -d '' file; do
            process_prompt "$file"
        done
    fi
}

# Run main function with all command line arguments
main "$@"

