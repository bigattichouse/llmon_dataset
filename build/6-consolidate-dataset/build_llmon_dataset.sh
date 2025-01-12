#!/bin/bash

# Check if the required parameters are provided
if [[ $# -lt 3 ]]; then
    echo "Usage: $0 <source_dir> <destination_dir> <file_extensions...>"
    echo "Example: $0 working/apache/httpd processed/apache/httpd .something .txt"
    exit 1
fi

# Assign parameters to variables
SOURCE_DIR="$1"
DEST_DIR="$2"
shift 2 # Shift positional parameters to skip source and destination
EXTENSIONS=("$@") # Remaining parameters are the file extensions

# Create the destination directory if it doesn't exist
mkdir -p "$DEST_DIR"

# Loop through the extensions and copy matching files
for ext in "${EXTENSIONS[@]}"; do
    find "$SOURCE_DIR" -type f -name "*$ext" | while read -r file; do
        # Compute the relative path of the file from the source directory
        relative_path="${file#$SOURCE_DIR/}"
        # Compute the destination path
        dest_path="$DEST_DIR/$relative_path"
        # Create the destination directory structure
        mkdir -p "$(dirname "$dest_path")"
        # Copy the file
        cp "$file" "$dest_path"
    done
done

echo "Files matching extensions (${EXTENSIONS[*]}) have been copied to $DEST_DIR"

