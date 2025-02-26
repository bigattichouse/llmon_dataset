// llm_service.c - Implementation of the LLM service interface
#define _GNU_SOURCE  // For GNU/Linux systems

#include "llm_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

// Structure to store response data
typedef struct {
    char *data;
    size_t size;
} response_data;

// Memory callback for curl
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    response_data *mem = (response_data *)userp;
    
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Error: Out of memory\n");
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

// Read file into string
static char* read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *buffer = malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(file);
        return NULL;
    }
    
    size_t read_length = fread(buffer, 1, length, file);
    buffer[read_length] = '\0';
    
    fclose(file);
    return buffer;
}

// Write string to file
static bool write_file(const char *filename, const char *data) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return false;
    }
    
    fputs(data, file);
    fclose(file);
    return true;
}

// Initialize service configuration with defaults
llm_service_config* llm_service_init(llm_service_type type) {
    llm_service_config *config = malloc(sizeof(llm_service_config));
    if (!config) {
        fprintf(stderr, "Error: Out of memory\n");
        return NULL;
    }
    
    // Set defaults
    config->type = type;
    config->context_size = 32500;  // Default for most models
    
    switch (type) {
        case SERVICE_OLLAMA:
            config->port = 11434;
            config->model_name = strdup("hermes3:8b-llama3.1-fp16");
            config->ollama.api_endpoint = strdup("http://localhost:11434/api/generate");
            break;
            
        case SERVICE_LLAMACPP:
            config->port = 8080;
            config->model_name = strdup("default");
            config->llamacpp.server_path = strdup("llama-server");
            config->llamacpp.model_path = strdup("models/model.gguf");
            config->llamacpp.max_retries = 30;
            config->llamacpp.retry_delay = 60;
            break;
            
        case SERVICE_REMOTE:
            config->port = 443;  // HTTPS
            config->model_name = strdup("default");
            config->remote.api_key = NULL;
            config->remote.base_url = NULL;
            break;
    }
    
    return config;
}

// Free service configuration
void llm_service_free(llm_service_config *config) {
    if (!config) return;
    
    free(config->model_name);
    
    switch (config->type) {
        case SERVICE_OLLAMA:
            free(config->ollama.api_endpoint);
            break;
            
        case SERVICE_LLAMACPP:
            free(config->llamacpp.server_path);
            free(config->llamacpp.model_path);
            break;
            
        case SERVICE_REMOTE:
            free(config->remote.api_key);
            free(config->remote.base_url);
            break;
    }
    
    free(config);
}

// Updated process_ollama function to extract clean response text
static bool process_ollama(llm_service_config *config, const char *prompt, char **response) {
    CURL *curl;
    CURLcode res;
    response_data resp_data = {0};
    resp_data.data = malloc(1);
    resp_data.data[0] = '\0';
    resp_data.size = 0;
    
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize CURL\n");
        free(resp_data.data);
        return false;
    }
    
    // JSON escaping code remains the same
    size_t prompt_len = strlen(prompt);
    char *escaped_prompt = malloc(prompt_len * 2 + 1);
    if (!escaped_prompt) {
        fprintf(stderr, "Error: Out of memory\n");
        curl_easy_cleanup(curl);
        free(resp_data.data);
        return false;
    }
    
    // Perform JSON escaping manually (keep this part unchanged)
    size_t j = 0;
    for (size_t i = 0; i < prompt_len; i++) {
        char c = prompt[i];
        
        // Escape special JSON characters
        if (c == '\"') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = '\"';
        } else if (c == '\\') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = '\\';
        } else if (c == '\b') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 'b';
        } else if (c == '\f') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 'f';
        } else if (c == '\n') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 'n';
        } else if (c == '\r') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 'r';
        } else if (c == '\t') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 't';
        } else {
            escaped_prompt[j++] = c;
        }
    }
    escaped_prompt[j] = '\0';
    
    // Create JSON payload (same as before)
    char *json_template = "{"
        "\"prompt\":\"%s\","
        "\"context_size\":%d,"
        "\"context_length\":%d,"
        "\"num_ctx\":%d,"
        "\"model\":\"%s\","
        "\"stream\":true"  // Ensure streaming is enabled
    "}";
    
    // Calculate required buffer size and allocate
    size_t json_size = strlen(json_template) + strlen(escaped_prompt) + 
                       strlen(config->model_name) + 64;  // Extra space for numbers and safety
    
    char *json_payload = malloc(json_size);
    if (!json_payload) {
        fprintf(stderr, "Error: Out of memory\n");
        free(escaped_prompt);
        curl_easy_cleanup(curl);
        free(resp_data.data);
        return false;
    }
    
    // Format the JSON payload
    snprintf(json_payload, json_size, json_template, 
             escaped_prompt, config->context_size, config->context_size, config->context_size, config->model_name);
    
    free(escaped_prompt);
    
    // Set curl options (same as before)
    curl_easy_setopt(curl, CURLOPT_URL, config->ollama.api_endpoint);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resp_data);
    
    // Add headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Perform request
    res = curl_easy_perform(curl);
    
    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(json_payload);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: CURL request failed: %s\n", curl_easy_strerror(res));
        free(resp_data.data);
        return false;
    }
    
    // Now, parse the response to extract just the text content
    // The response contains multiple JSON lines, each with a "response" field
    
    // Allocate buffer for extracted text (worst case: same size as raw response)
    char *extracted_text = malloc(resp_data.size + 1);
    if (!extracted_text) {
        fprintf(stderr, "Error: Out of memory\n");
        free(resp_data.data);
        return false;
    }
    extracted_text[0] = '\0';
    size_t extracted_len = 0;
    
    // Parse each line
    char *line = strtok(resp_data.data, "\n");
    while (line != NULL) {
        // Look for "response":" in the JSON
        char *response_start = strstr(line, "\"response\":\"");
        if (response_start) {
            // Move pointer to the start of the response value
            response_start += 12;  // Length of "response":"
            
            // Find the end of the response value (the next quote not preceded by a backslash)
            char *response_end = response_start;
            bool escaped = false;
            
            while (*response_end) {
                if (*response_end == '\\') {
                    escaped = !escaped;
                } else if (*response_end == '"' && !escaped) {
                    break;
                } else {
                    escaped = false;
                }
                response_end++;
            }
            
            // Temporarily terminate the string at the end quote for easier handling
            if (*response_end == '"') {
                *response_end = '\0';
                
                // Unescape the JSON string
                char *unesc = malloc(strlen(response_start) + 1);
                if (unesc) {
                    size_t u = 0;
                    for (size_t i = 0; response_start[i]; i++) {
                        if (response_start[i] == '\\' && response_start[i+1]) {
                            i++;
                            switch (response_start[i]) {
                                case 'n': unesc[u++] = '\n'; break;
                                case 'r': unesc[u++] = '\r'; break;
                                case 't': unesc[u++] = '\t'; break;
                                case 'b': unesc[u++] = '\b'; break;
                                case 'f': unesc[u++] = '\f'; break;
                                case '\\': unesc[u++] = '\\'; break;
                                case '\"': unesc[u++] = '\"'; break;
                                default: unesc[u++] = response_start[i]; break;
                            }
                        } else {
                            unesc[u++] = response_start[i];
                        }
                    }
                    unesc[u] = '\0';
                    
                    // Append to the extracted text
                    strcpy(extracted_text + extracted_len, unesc);
                    extracted_len += u;
                    
                    free(unesc);
                }
                
                // Restore the end quote
                *response_end = '"';
            }
        }
        
        line = strtok(NULL, "\n");
    }
    
    // Make sure the extracted text is properly null-terminated
    extracted_text[extracted_len] = '\0';
    
    // Free the original raw response
    free(resp_data.data);
    
    // Return the extracted text
    *response = extracted_text;
    return true;
}

// Process a prompt using llama.cpp server
static bool process_llamacpp(llm_service_config *config, const char *prompt, char **response) {
    CURL *curl;
    CURLcode res;
    response_data resp_data = {0};
    resp_data.data = malloc(1);
    resp_data.data[0] = '\0';
    resp_data.size = 0;
    
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize CURL\n");
        free(resp_data.data);
        return false;
    }
    
    // We'll manually construct a valid JSON object by properly escaping the prompt
    // First, allocate a buffer large enough for the escaped prompt (worst case: each char needs escaping)
    size_t prompt_len = strlen(prompt);
    char *escaped_prompt = malloc(prompt_len * 2 + 1);
    if (!escaped_prompt) {
        fprintf(stderr, "Error: Out of memory\n");
        curl_easy_cleanup(curl);
        free(resp_data.data);
        return false;
    }
    
    // Perform JSON escaping manually
    size_t j = 0;
    for (size_t i = 0; i < prompt_len; i++) {
        char c = prompt[i];
        
        // Escape special JSON characters
        if (c == '\"') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = '\"';
        } else if (c == '\\') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = '\\';
        } else if (c == '\b') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 'b';
        } else if (c == '\f') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 'f';
        } else if (c == '\n') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 'n';
        } else if (c == '\r') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 'r';
        } else if (c == '\t') {
            escaped_prompt[j++] = '\\';
            escaped_prompt[j++] = 't';
        } else {
            escaped_prompt[j++] = c;
        }
    }
    escaped_prompt[j] = '\0';
    
    // Create JSON payload
    char *json_template = "{\"prompt\":\"%s\"}";
    
    // Calculate required buffer size and allocate
    size_t json_size = strlen(json_template) + strlen(escaped_prompt) + 16;  // Extra space for safety
    
    char *json_payload = malloc(json_size);
    if (!json_payload) {
        fprintf(stderr, "Error: Out of memory\n");
        free(escaped_prompt);
        curl_easy_cleanup(curl);
        free(resp_data.data);
        return false;
    }
    
    // Format the JSON payload
    snprintf(json_payload, json_size, json_template, escaped_prompt);
    
    free(escaped_prompt);
    
    // Create URL
    char url[256];
    snprintf(url, sizeof(url), "http://localhost:%d/completion", config->port);
    
    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resp_data);
    
    // Add headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Retry mechanism
    bool success = false;
    long http_code = 0;
    
    for (int retry = 0; retry < config->llamacpp.max_retries; retry++) {
        // Perform request
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            fprintf(stderr, "Error: CURL request failed: %s\n", curl_easy_strerror(res));
            continue;
        }
        
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        if (http_code == 200) {
            success = true;
            break;
        } else if (http_code == 503) {
            printf("Server returned 503 (Loading model). Retry %d of %d...\n", 
                   retry + 1, config->llamacpp.max_retries);
            
            // Reset response data for next attempt
            free(resp_data.data);
            resp_data.data = malloc(1);
            resp_data.data[0] = '\0';
            resp_data.size = 0;
            
            sleep(config->llamacpp.retry_delay);
        } else {
            fprintf(stderr, "Error: Unexpected response code: %ld\n", http_code);
            break;
        }
    }
    
    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(json_payload);
    
    if (!success) {
        free(resp_data.data);
        return false;
    }
    
    *response = resp_data.data;
    return true;
}
// Check if a server is running
bool llm_check_server(llm_service_config *config) {
    if (config->type != SERVICE_LLAMACPP) {
        // Always return true for services that don't need server checking
        return true;
    }
    
    // Simple check using curl to see if server is responding
    CURL *curl;
    CURLcode res;
    long http_code = 0;
    
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize CURL\n");
        return false;
    }
    
    char url[256];
    snprintf(url, sizeof(url), "http://localhost:%d/health", config->port);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
    
    res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }
    
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && http_code == 200);
}

// Start a server if needed (for llama.cpp)
bool llm_start_server(llm_service_config *config) {
    if (config->type != SERVICE_LLAMACPP) {
        // Only llama.cpp requires starting a server
        return true;
    }
    
    // Check if server is already running
    if (llm_check_server(config)) {
        printf("llama.cpp server is already running\n");
        return true;
    }
    
    // Start the server
    pid_t pid = fork();
    
    if (pid < 0) {
        fprintf(stderr, "Error: Failed to fork process\n");
        return false;
    }
    
    if (pid == 0) {
        // Child process - start the server
        char port_str[16];
        char ctx_size_str[16];
        
        snprintf(port_str, sizeof(port_str), "%d", config->port);
        snprintf(ctx_size_str, sizeof(ctx_size_str), "%d", config->context_size);
        
        // Redirect stdout and stderr to a log file
        freopen("llama_server.log", "w", stdout);
        freopen("llama_server.log", "a", stderr);
        
        // Execute the server
        execl(config->llamacpp.server_path, config->llamacpp.server_path, 
              "--port", port_str,
              "--ctx-size", ctx_size_str,
              "--model", config->llamacpp.model_path,
              (char *)NULL);
        
        // If execl returns, an error occurred
        fprintf(stderr, "Error: Failed to start llama-server: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Parent process
    printf("Started llama.cpp server (PID: %d)\n", pid);
    
    // Wait for server to come online
    for (int i = 0; i < config->llamacpp.max_retries; i++) {
        sleep(config->llamacpp.retry_delay);
        if (llm_check_server(config)) {
            printf("llama.cpp server is now online\n");
            return true;
        }
        printf("Waiting for server to come online (%d/%d)...\n", 
               i + 1, config->llamacpp.max_retries);
    }
    
    fprintf(stderr, "Error: Server failed to come online within the timeout period\n");
    return false;
}

// Process a single prompt file
bool llm_process_prompt(llm_service_config *config, const char *input_file, const char *output_file) {
    printf("Processing %s...\n", input_file);
    
    // Read prompt from file
    char *prompt = read_file(input_file);
    if (!prompt) {
        return false;
    }
    
    // Prepare response
    char *response = NULL;
    bool success = false;
    
    // Process based on service type
    switch (config->type) {
        case SERVICE_OLLAMA:
            success = process_ollama(config, prompt, &response);
            break;
            
        case SERVICE_LLAMACPP:
            success = process_llamacpp(config, prompt, &response);
            break;
            
        case SERVICE_REMOTE:
            fprintf(stderr, "Error: Remote service not yet implemented\n");
            success = false;
            break;
    }
    
    free(prompt);
    
    if (!success || !response) {
        return false;
    }
    
    // Write response to output file
    success = write_file(output_file, response);
    free(response);
    
    if (success) {
        printf("Successfully processed %s -> %s\n", input_file, output_file);
    }
    
    return success;
}

// Check if a file has .prompt extension
static bool is_prompt_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return false;
    return strcmp(ext, ".prompt") == 0;
}

// Process all prompt files in a directory
int llm_process_directory(llm_service_config *config, const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;
    
    // Open directory
    dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Error: Cannot open directory %s\n", dir_path);
        return -1;
    }
    
    // Make sure server is running for llama.cpp
    if (config->type == SERVICE_LLAMACPP && !llm_check_server(config)) {
        if (!llm_start_server(config)) {
            closedir(dir);
            return -1;
        }
    }
    
    // Process each .prompt file
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_prompt_file(entry->d_name)) {
            char input_path[PATH_MAX];
            char output_path[PATH_MAX+10];
            
            snprintf(input_path, sizeof(input_path), "%s/%s", dir_path, entry->d_name);
            snprintf(output_path, sizeof(output_path), "%s.response", input_path);
            
            if (llm_process_prompt(config, input_path, output_path)) {
                count++;
            }
        }
    }
    
    closedir(dir);
    return count;
}
