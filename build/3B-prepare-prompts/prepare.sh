#!/bin/bash
#merge strings and constants into prompts and create prompt files
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
PROJECT=$1
SOURCEFILES="../working/$1"
#extract strings from the project 
#find/grep to get all the .c and .h files
#extract them all to correspding files in the working dircetory

# Check if SOURCEFILES directory exists
if [ ! -d "$SOURCEFILES" ]; then
    echo "Error: Directory $SOURCEFILES does not exist"
    exit 1
fi

# Find all .c and .h files and process them
find "$SOURCEFILES" -type f \( -name "*.c" -o -name "*.h" \) | while read -r file; do
    echo "    Creating Prompt for $file..."
    cat prompts/extract.prompt > "${file}.prompt"
    
    #add extracted strings to the file, so LLM knows what to keep an eye out for
    cat prompts/strings.prompt.part >> "${file}.prompt"
    echo "\`\`\`" >> "${file}.prompt"
    cat "${file}.strings" >> "${file}.prompt"
    echo "\`\`\`" >> "${file}.prompt"
     
    #now the actual file (obviously this will fail for really long files, 
    #TODO: amend to handle in 32k lengths for local models
    cat prompts/file.prompt.part >> "${file}.prompt"
    echo "\`\`\`" >> "${file}.prompt"
    cat ${file} >> "${file}.prompt"
    echo "\`\`\`" >> "${file}.prompt"
    
PROMPT="Please summarize the purpose and role of the module \`${file}\` from the \`$1\` project from the following source code. Only output the summary text, only use 3 to 4 sentences and be succinct and accurate." 
    echo $PROMPT >> "${file}.summary"
    echo "\`\`\`" >> "${file}.summary"
    cat ${file} >> "${file}.summary"
    echo "\`\`\`" >> "${file}.summary"
     
done

echo "String extraction complete"
