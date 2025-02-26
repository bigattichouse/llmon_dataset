# LLM Query - Usage Guide

`llm_query` is a C application for processing text prompts using different LLM (Large Language Model) services like Ollama and llama.cpp. It provides a unified interface for interacting with these services.

## Installation

### Prerequisites

- C compiler (GCC or Clang)
- libcurl development files
- Make

For Debian/Ubuntu systems:
```bash
sudo apt-get install build-essential libcurl4-openssl-dev
```

For macOS (using Homebrew):
```bash
brew install curl
```

### Building the Application

Clone the repository and build using make:

```bash
git clone <repository-url>
cd <repository-directory>
make
```

The compiler will generate an executable named `llm_query`.

## Basic Usage

```bash
./llm_query [options] <directory>
```

The application will:
1. Scan the specified directory for files with a `.prompt` extension
2. Process each prompt file using the selected LLM service
3. Save the responses as `.prompt.response` files in the same directory

## Command Line Options

### Common Options

| Option | Description | Default |
|--------|-------------|---------|
| `-s, --service <type>` | Service type: `ollama` or `llamacpp` | `ollama` |
| `-m, --model <name>` | Model name to use | Service dependent |
| `-c, --context <size>` | Context size | 32500 |
| `-p, --port <port>` | Port to use | Service dependent |
| `-h, --help` | Display help message | - |

### llama.cpp Specific Options

| Option | Description | Default |
|--------|-------------|---------|
| `--server-path <path>` | Path to llama-server executable | `llama-server` |
| `--model-path <path>` | Path to model file (.gguf) | `models/model.gguf` |
| `--max-retries <num>` | Maximum number of retries | 30 |
| `--retry-delay <seconds>` | Delay between retries | 60 |

## Examples

### Process files using Ollama

```bash
./llm_query --service ollama --model hermes3:8b-llama3.1-fp16 ../working/prompts
```

### Process files using llama.cpp

```bash
./llm_query --service llamacpp --model-path /path/to/models/model.gguf ../working/prompts
```

### Use a different port

```bash
./llm_query --service ollama --port 8000 ../working/prompts
```

### Use a custom context size

```bash
./llm_query --service llamacpp --context 16000 ../working/prompts
```

## Service Details

### Ollama

Ollama is accessed through its HTTP API. The default endpoint is:
```
http://localhost:11434/api/generate
```

### llama.cpp

For the llama.cpp service, the application will:
1. Check if a llama-server is already running
2. Start the server if needed using the provided server and model paths
3. Submit prompt requests to the server
4. Retry with a delay if the server returns a 503 (model loading) response

## File Format

- Input files must have a `.prompt` extension
- The entire file content is sent as the prompt to the LLM
- Response files are saved with an additional `.response` extension

## Troubleshooting

If you encounter issues:

1. Verify the LLM service is installed and configured correctly
2. Check network connectivity to the local server
3. Ensure you have proper permissions to read/write files
4. Look for error messages in the application output
5. For llama.cpp, check the server log file (`llama_server.log`)
