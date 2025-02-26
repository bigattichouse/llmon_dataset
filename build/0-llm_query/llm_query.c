// llm_query.c - Main application for processing prompt files with LLM services
#define _GNU_SOURCE  // For GNU/Linux systems

#include "llm_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

void print_usage(const char *program_name) {
    printf("Usage: %s [options] <directory>\n", program_name);
    printf("\nOptions:\n");
    printf("  -s, --service <type>    Service type: ollama, llamacpp (default: ollama)\n");
    printf("  -m, --model <name>      Model name to use\n");
    printf("  -c, --context <size>    Context size (default: 32500)\n");
    printf("  -p, --port <port>       Port to use (default: depends on service)\n");
    printf("  -h, --help              Display this help message\n");
    printf("\nLlama.cpp specific options:\n");
    printf("  --server-path <path>    Path to llama-server executable\n");
    printf("  --model-path <path>     Path to model file (.gguf)\n");
    printf("  --max-retries <num>     Maximum number of retries (default: 30)\n");
    printf("  --retry-delay <seconds> Delay between retries (default: 60)\n");
}

int main(int argc, char *argv[]) {
    // Default options
    llm_service_type service_type = SERVICE_OLLAMA;
    char *model_name = NULL;
    int context_size = 0;  // 0 means use default
    int port = 0;          // 0 means use default
    char *server_path = NULL;
    char *model_path = NULL;
    int max_retries = 0;    // 0 means use default
    int retry_delay = 0;    // 0 means use default
    char *directory = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--service") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: Missing service type\n");
                print_usage(argv[0]);
                return 1;
            }
            
            if (strcmp(argv[i], "ollama") == 0) {
                service_type = SERVICE_OLLAMA;
            } else if (strcmp(argv[i], "llamacpp") == 0) {
                service_type = SERVICE_LLAMACPP;
            } else {
                fprintf(stderr, "Error: Unknown service type: %s\n", argv[i]);
                print_usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--model") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: Missing model name\n");
                print_usage(argv[0]);
                return 1;
            }
            model_name = argv[i];
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--context") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: Missing context size\n");
                print_usage(argv[0]);
                return 1;
            }
            context_size = atoi(argv[i]);
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: Missing port number\n");
                print_usage(argv[0]);
                return 1;
            }
            port = atoi(argv[i]);
        } else if (strcmp(argv[i], "--server-path") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: Missing server path\n");
                print_usage(argv[0]);
                return 1;
            }
            server_path = argv[i];
        } else if (strcmp(argv[i], "--model-path") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: Missing model path\n");
                print_usage(argv[0]);
                return 1;
            }
            model_path = argv[i];
        } else if (strcmp(argv[i], "--max-retries") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: Missing max retries\n");
                print_usage(argv[0]);
                return 1;
            }
            max_retries = atoi(argv[i]);
        } else if (strcmp(argv[i], "--retry-delay") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: Missing retry delay\n");
                print_usage(argv[0]);
                return 1;
            }
            retry_delay = atoi(argv[i]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (directory == NULL) {
            directory = argv[i];
        } else {
            fprintf(stderr, "Error: Unexpected argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (directory == NULL) {
        fprintf(stderr, "Error: No directory specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Initialize service configuration
    llm_service_config *config = llm_service_init(service_type);
    if (!config) {
        return 1;
    }
    
    // Apply command line overrides
    if (model_name) {
        free(config->model_name);
        config->model_name = strdup(model_name);
    }
    
    if (context_size > 0) {
        config->context_size = context_size;
    }
    
    if (port > 0) {
        config->port = port;
    }
    
    // Service-specific overrides
    if (service_type == SERVICE_LLAMACPP) {
        if (server_path) {
            free(config->llamacpp.server_path);
            config->llamacpp.server_path = strdup(server_path);
        }
        
        if (model_path) {
            free(config->llamacpp.model_path);
            config->llamacpp.model_path = strdup(model_path);
        }
        
        if (max_retries > 0) {
            config->llamacpp.max_retries = max_retries;
        }
        
        if (retry_delay > 0) {
            config->llamacpp.retry_delay = retry_delay;
        }
    }
    
    // Process prompt files
    int result = llm_process_directory(config, directory);
    
    // Clean up
    llm_service_free(config);
    
    if (result < 0) {
        fprintf(stderr, "Error: Failed to process directory\n");
        return 1;
    }
    
    printf("Successfully processed %d prompt files\n", result);
    return 0;
}
