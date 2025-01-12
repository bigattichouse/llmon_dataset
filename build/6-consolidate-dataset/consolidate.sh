#!/bin/bash
#consolidate the dataset
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
make clean; make
SOURCEFILES="../working/$1"
#extract strings from the project 
#find/grep to get all the .c and .h files
#extract them all to correspding files in the working dircetory

# Check if SOURCEFILES directory exists
if [ ! -d "$SOURCEFILES" ]; then
    echo "Error: Directory $SOURCEFILES does not exist"
    exit 1
fi

# Check if extract_strings executable exists and is executable
if [ ! -x "bin/parse_ollama_response" ]; then
    echo "Error: parse_ollama_response not found or not executable"
    exit 1
fi

# Find all summaries
find "$SOURCEFILES" -type f -name "*.summary.summarized" -print0 | while IFS= read -r -d '' file; do
    echo "Parsing ollama response $file..."
    ./bin/parse_ollama_response "$file" > "${file}.summary.text"
done

# Find all log messages
find "$SOURCEFILES" -type f -name "*.prompt.response" -print0 | while IFS= read -r -d '' file; do
    echo "Parsing ollama response $file..."
    ./bin/parse_ollama_response "$file" > "${file}.prompt.json"
done

#build the final JSON output
find "$SOURCEFILES" -type f \( -name "*.c" -o -name "*.h" \) | while read -r file; do
    echo "Processing $file..."
    ./bin/build_json_object --summary "${file}.summary.text" --logs "${file}.prompt.json" --embeddings "${file}.prompt.json.embedding"  > "${file}.llmon.json"
done

echo "String extraction complete"
