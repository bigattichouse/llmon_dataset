#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#define MAX_FILENAME_LEN 1024
#define MAX_FILES 100
#define BUFFER_SIZE 4096

// Structure to hold file info
typedef struct {
    char filename[MAX_FILENAME_LEN];
    char *content;
    size_t content_size;
} FileData;


char *hash_filename(const char *filename) {
    if (!filename) return NULL;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    char *output = malloc(SHA256_DIGEST_LENGTH * 2 + 1); // Hex string + null terminator
    if (!output) return NULL;

    // Use OpenSSL EVP API for SHA-256 hashing
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        free(output);
        return NULL;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(mdctx, filename, strlen(filename)) != 1 ||
        EVP_DigestFinal_ex(mdctx, hash, NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        free(output);
        return NULL;
    }

    EVP_MD_CTX_free(mdctx);

    // Convert binary hash to a readable hex string
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(output + (i * 2), "%02x", hash[i]);

    output[SHA256_DIGEST_LENGTH * 2] = '\0'; // Null-terminate

    return output;
}

// Read file content into memory
char *read_file(const char *filename, size_t *size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Check for negative file size (error condition)
    if (file_size < 0) {
        fclose(file);
        fprintf(stderr, "Error: Could not determine size of file %s\n", filename);
        return NULL;
    }
    
    // Convert to unsigned size_t for safe comparison later
    size_t unsigned_file_size = (size_t)file_size;
    
    // Allocate memory for content
    char *content = (char *)malloc(unsigned_file_size + 1);
    if (!content) {
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed for %s\n", filename);
        return NULL;
    }
    
    // Read file content
    size_t bytes_read = fread(content, 1, unsigned_file_size, file);
    fclose(file);
    
    if (bytes_read != unsigned_file_size) {
        free(content);
        fprintf(stderr, "Error: Failed to read entire file %s\n", filename);
        return NULL;
    }
    
    // Null terminate the content
    content[unsigned_file_size] = '\0';
    *size = unsigned_file_size;
    
    return content;
}

// Function to escape JSON string content
char *escape_json_string(const char *input) {
    if (!input) return NULL;
    
    size_t input_len = strlen(input);
    // Allocate memory for worst case scenario (every character needs escaping)
    char *output = (char *)malloc(input_len * 2 + 1);
    if (!output) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < input_len; i++) {
        switch (input[i]) {
            case '"':
                output[j++] = '\\';
                output[j++] = '"';
                break;
            case '\\':
                output[j++] = '\\';
                output[j++] = '\\';
                break;
            case '\b':
                output[j++] = '\\';
                output[j++] = 'b';
                break;
            case '\f':
                output[j++] = '\\';
                output[j++] = 'f';
                break;
            case '\n':
                output[j++] = '\\';
                output[j++] = 'n';
                break;
            case '\r':
                output[j++] = '\\';
                output[j++] = 'r';
                break;
            case '\t':
                output[j++] = '\\';
                output[j++] = 't';
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\0';
    
    return output;
}

// Build the JSON payload for the API request
char *build_json_payload(FileData *files, int file_count) {
    // Initial buffer size
    size_t buffer_size = BUFFER_SIZE;
    char *buffer = (char *)malloc(buffer_size);
    if (!buffer) return NULL;
    
    // Start the JSON payload
    strcpy(buffer, "{\n  \"requests\": [\n");
    size_t position = strlen(buffer);
    
    for (int i = 0; i < file_count; i++) {
        // Escape the file content for JSON
        char *escaped_content = escape_json_string(files[i].content);
        if (!escaped_content) {
            free(buffer);
            return NULL;
        }
        
        // Escape the filename for JSON
        char *escaped_filename = hash_filename(files[i].filename);
        if (!escaped_filename) {
            free(escaped_content);
            free(buffer);
            return NULL;
        }
         
        
        // Calculate required size for this entry
        size_t required_size = position + strlen(escaped_content) + strlen(escaped_filename) + 200;
        
        // Resize buffer if needed
        if (required_size > buffer_size) {
            buffer_size = required_size * 2;
            char *new_buffer = (char *)realloc(buffer, buffer_size);
            if (!new_buffer) {
                free(escaped_content);
                free(escaped_filename);
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
        
        // Add this file's request to the JSON
        position += sprintf(buffer + position,
            "    {\n"
            "      \"custom_id\": \"%s\",\n"
            "      \"params\": {\n"
            "        \"model\": \"claude-3-7-sonnet-20250219\",\n"
            "        \"max_tokens\": 655350,\n"
            "        \"messages\": [\n"
            "          {\"role\": \"user\", \"content\": \"%s\"}\n"
            "        ]\n"
            "      }\n"
            "    }%s\n",
            escaped_filename,
            escaped_content,
            (i < file_count - 1) ? "," : ""
        );
        
        free(escaped_content);
        free(escaped_filename);
    }
    
    // Finish the JSON payload
    strcat(buffer + position, "  ]\n}");
    
    return buffer;
}

// Function to make the API request
void make_api_request(const char *api_key, const char *payload) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    char auth_header[256];
    
    // Initialize curl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize curl\n");
        return;
    }
    
    // Set up headers
    headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
    headers = curl_slist_append(headers, "content-type: application/json");
    headers = curl_slist_append(headers, "anthropic-beta: message-batches-2024-09-24");
    
    // Add API key header
    snprintf(auth_header, sizeof(auth_header), "x-api-key: %s", api_key);
    headers = curl_slist_append(headers, auth_header);
    
    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.anthropic.com/v1/messages/batches");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    
    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: curl_easy_perform() failed: %s\n", 
                curl_easy_strerror(res));
    }
    
    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

// Safer string copy function to avoid truncation warnings
void safe_strcpy(char *dest, const char *src, size_t dest_size) {
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s API_KEY\n", argv[0]);
        return 1;
    }
    
    const char *api_key = argv[1];
    
    // Initialize curl
    curl_global_init(CURL_GLOBAL_ALL);
    
    FileData files[MAX_FILES];
    int file_count = 0;
    char line[MAX_FILENAME_LEN];
    
    printf("Enter filenames (one per line, empty line to finish):\n");
    
    // Read filenames from stdin
    while (fgets(line, sizeof(line), stdin) && file_count < MAX_FILES) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        
        // Check if line is empty
        if (strlen(line) == 0) {
            break;
        }
        
        // Store filename - using our safe string copy function instead of strncpy
        safe_strcpy(files[file_count].filename, line, MAX_FILENAME_LEN);
        
        // Read file content
        files[file_count].content = read_file(line, &files[file_count].content_size);
        if (files[file_count].content) {
            file_count++;
        }
    }
    
    if (file_count == 0) {
        fprintf(stderr, "No valid files provided.\n");
        curl_global_cleanup();
        return 1;
    }
    
    // Build JSON payload
    char *payload = build_json_payload(files, file_count);
    if (!payload) {
        fprintf(stderr, "Failed to build JSON payload.\n");
        for (int i = 0; i < file_count; i++) {
            free(files[i].content);
        }
        curl_global_cleanup();
        return 1;
    }
    
    // Print the payload for debugging
    printf("\nSending the following payload:\n%s\n\n", payload);
    
    // Make API request
    make_api_request(api_key, payload);
    
    // Clean up
    for (int i = 0; i < file_count; i++) {
        free(files[i].content);
    }
    free(payload);
    curl_global_cleanup();
    
    return 0;
}
