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
    local output_file="$input_file.summarized"
    echo "Processing $input_file...\n" 
    
    # Read prompt from file
    local prompt=$(cat "$input_file")
    
    # This will properly escape all special characters including quotes
    json_data=$(jq -n --arg p "$prompt" '{"prompt": $p}' | tr -d '\n')

    curl -X POST "http://localhost:$LLAMA_PORT/completion" \
        -H "Content-Type: application/json" \
        -d "$json_data" \
        > "$output_file"
    
    echo "Processed $input_file -> $output_file"
}

# Main script
main() {
    # Check if server is running, start if not
    if check_server; then
        # Process all input files
        #find "$SOURCEFILES" -type f \( -name "*.prompt"\) | while read -r file; do
        find "$SOURCEFILES" -type f -name "*.summary" -print0 | while IFS= read -r -d '' file; do
            process_prompt "$file"
        done
    fi
}

# Run main function with all command line arguments
main "$@"

