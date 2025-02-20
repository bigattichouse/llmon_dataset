#!/bin/bash

# Summary: This script processes text files using the Ollama API.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
SOURCEFILES="../working/$1"

# Check if SOURCEFILES directory exists
if [ ! -d "$SOURCEFILES" ]; then
    echo "Error: Directory $SOURCEFILES does not exist"
    exit 1
fi

OLLAMA_PORT=11434  # Adjust port as needed
OLLAMA_API_URL="http://localhost:$OLLAMA_PORT/api/generate"  # Ollama API endpoint
CONTEXT_SIZE=${CONTEXT_SIZE:-32500}  # Default context size, can be overridden
MODEL=${MODEL:-"hermes3:8b-llama3.1-fp16"}  # Default model name, can be overridden

# Function to send prompt to server and save response
process_prompt() {
    local input_file=$1
    local output_file="$input_file.summarized"
    echo "Processing $input_file..." 
    
    # Read prompt from file
    local prompt=$(cat "$input_file")
    
    # This will properly escape all special characters including quotes
    json_data=$(jq -n --arg p "$prompt" --argjson c $CONTEXT_SIZE --arg m "$MODEL" '{"prompt": $p, "context_size": $c, "num_ctx": $c, "model": $m}' | tr -d '\n')

    curl -s -X POST "$OLLAMA_API_URL" \
        -H "Content-Type: application/json" \
        -d "$json_data" \
        > "$output_file"
    
    echo "Processed $input_file -> $output_file"
}

# Main script
main() {
    # Process all input files
    find "$SOURCEFILES" -type f -name "*.summary" -print0 | while IFS= read -r -d '' file; do
        process_prompt "$file"
    done
}

# Run main function
main "$@"

