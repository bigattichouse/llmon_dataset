# Anthropic Batch API Client

A C program that reads filenames from standard input, loads their contents, and sends them as batch requests to the Anthropic API.

## Overview

This program:
- Takes your Anthropic API key as a command-line parameter
- Reads filenames from standard input (one per line)
- Loads the content of each file
- Uses the filename as the `custom_id` for each request
- Constructs a properly formatted JSON payload
- Sends the payload to the Anthropic batch API endpoint

## Prerequisites

You need the libcurl development package installed on your system:

### On Debian/Ubuntu
```bash
sudo apt-get install libcurl4-openssl-dev
```

### On macOS with Homebrew
```bash
brew install curl
```

## Compilation

Compile the program with:
```bash
gcc -o anthropic_batch anthropic_batch.c -lcurl
```

## Usage

### Using a file list
```bash
./anthropic_batch YOUR_API_KEY < file_list.txt
```

### Interactive mode
```bash
./anthropic_batch YOUR_API_KEY
prompt1.txt
prompt2.txt
prompt3.txt
[empty line to finish]
```

## Notes

- The program processes up to 100 files
- Each file's content becomes the "content" field in a user message
- The filename is used as the "custom_id" for each request
- All special characters in filenames and content are properly escaped
- By default, responses use the claude-3-7-sonnet-20250219 model
- The maximum token count for responses is set to 100 by default
