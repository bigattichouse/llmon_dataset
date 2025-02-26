// llm_service.h - Generic interface for LLM services

#ifndef LLM_SERVICE_H
#define LLM_SERVICE_H

#include <stdbool.h>

// Service types supported by the interface
typedef enum {
    SERVICE_OLLAMA,
    SERVICE_LLAMACPP,
    SERVICE_REMOTE,  // For future implementation
} llm_service_type;

// Configuration for each service
typedef struct {
    llm_service_type type;
    
    // Common settings
    char *model_name;     // Model to use
    int context_size;     // Context size for the model
    int port;             // Port to connect to
    
    // Service-specific settings
    union {
        struct {
            char *api_endpoint;    // API endpoint URL for Ollama
        } ollama;
        
        struct {
            char *server_path;     // Path to llama-server executable
            char *model_path;      // Path to model file (.gguf)
            int max_retries;       // Max retries for server loading
            int retry_delay;       // Delay between retries in seconds
        } llamacpp;
        
        struct {
            char *api_key;         // API key for remote services
            char *base_url;        // Base URL for remote API
        } remote;
    };
} llm_service_config;

// Initialize service configuration with defaults
llm_service_config* llm_service_init(llm_service_type type);

// Free service configuration
void llm_service_free(llm_service_config *config);

// Process a single prompt file
bool llm_process_prompt(llm_service_config *config, const char *input_file, const char *output_file);

// Process all prompt files in a directory
int llm_process_directory(llm_service_config *config, const char *dir_path);

// Check if a server is running (for services that require local servers)
bool llm_check_server(llm_service_config *config);

// Start a server if needed (for llama.cpp)
bool llm_start_server(llm_service_config *config);

#endif /* LLM_SERVICE_H */
