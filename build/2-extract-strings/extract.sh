#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
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
if [ ! -x "bin/string_extractor" ]; then
    echo "Error: extract_strings not found or not executable"
    exit 1
fi

# Find all .c and .h files and process them
find "$SOURCEFILES" -type f \( -name "*.c" -o -name "*.h" \) | while read -r file; do
    echo "Processing $file..."
    ./bin/c_string_extractor "$file" > "${file}.strings"
done

echo "String extraction complete"
