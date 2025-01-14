#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define BUFFER_SIZE 4096
#define MAX_RESPONSE_SIZE (BUFFER_SIZE * 10)
#define RESPONSE_KEY "\"response\":"

// Error codes
typedef enum {
    SUCCESS = 0,
    ERROR_FILE_OPEN,
    ERROR_BUFFER_OVERFLOW,
    ERROR_INVALID_INPUT,
    ERROR_MEMORY
} ParseError;

// Structure to hold response data
typedef struct {
    char* buffer;
    size_t size;
    size_t capacity;
} ResponseBuffer;

// Function declarations
static ParseError init_response_buffer(ResponseBuffer* resp, size_t initial_capacity);
static ParseError append_to_response(ResponseBuffer* resp, const char* str, size_t len);
static void free_response_buffer(ResponseBuffer* resp);
static ParseError handle_escape_sequence(ResponseBuffer* resp, char escaped_char);
static const char* get_error_message(ParseError error);

ParseError parse_json_responses(const char* filename, ResponseBuffer* response) {
    if (!filename || !response) {
        return ERROR_INVALID_INPUT;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        return ERROR_FILE_OPEN;
    }

    char buffer[BUFFER_SIZE];
    ParseError error = SUCCESS;
    bool in_escape = false;

    while (fgets(buffer, sizeof(buffer), file)) {
        char* response_start = strstr(buffer, RESPONSE_KEY);
        if (!response_start) {
            continue;
        }

        response_start += strlen(RESPONSE_KEY);
        while (*response_start == ' ') {
            response_start++;
        }

        if (*response_start != '"') {
            continue;
        }

        response_start++; // Skip opening quote
        char* response_end = response_start;

        while (*response_end && !(!in_escape && *response_end == '"')) {
            if (in_escape) {
                error = handle_escape_sequence(response, *response_end);
                if (error != SUCCESS) {
                    fclose(file);
                    return error;
                }
                response_start = response_end + 1;
                in_escape = false;
            } else if (*response_end == '\\') {
                // Append everything up to the escape character
                size_t len = response_end - response_start;
                if (len > 0) {
                    error = append_to_response(response, response_start, len);
                    if (error != SUCCESS) {
                        fclose(file);
                        return error;
                    }
                }
                in_escape = true;
            }
            response_end++;
        }

        // Append remaining characters
        if (response_start < response_end && !in_escape) {
            size_t len = (*response_end == '"') ? 
                        (response_end - response_start) : 
                        (response_end - response_start + 1);
            error = append_to_response(response, response_start, len);
            if (error != SUCCESS) {
                fclose(file);
                return error;
            }
        }
    }

    fclose(file);
    return SUCCESS;
}

static ParseError init_response_buffer(ResponseBuffer* resp, size_t initial_capacity) {
    resp->buffer = (char*)malloc(initial_capacity);
    if (!resp->buffer) {
        return ERROR_MEMORY;
    }
    resp->buffer[0] = '\0';
    resp->size = 0;
    resp->capacity = initial_capacity;
    return SUCCESS;
}

static ParseError append_to_response(ResponseBuffer* resp, const char* str, size_t len) {
    if (resp->size + len + 1 > resp->capacity) {
        size_t new_capacity = resp->capacity * 2;
        char* new_buffer = (char*)realloc(resp->buffer, new_capacity);
        if (!new_buffer) {
            return ERROR_MEMORY;
        }
        resp->buffer = new_buffer;
        resp->capacity = new_capacity;
    }

    memcpy(resp->buffer + resp->size, str, len);
    resp->size += len;
    resp->buffer[resp->size] = '\0';
    return SUCCESS;
}

static ParseError handle_escape_sequence(ResponseBuffer* resp, char escaped_char) {
    char escape_buffer[2] = {0};
    switch (escaped_char) {
        case 'n':
            return append_to_response(resp, "\n", 1);
        case '\\':
        case '"':
            escape_buffer[0] = escaped_char;
            return append_to_response(resp, escape_buffer, 1);
        default:
            escape_buffer[0] = '\\';
            escape_buffer[1] = escaped_char;
            return append_to_response(resp, escape_buffer, 2);
    }
}

static const char* get_error_message(ParseError error) {
    switch (error) {
        case SUCCESS:
            return "Success";
        case ERROR_FILE_OPEN:
            return "Could not open file";
        case ERROR_BUFFER_OVERFLOW:
            return "Buffer overflow";
        case ERROR_INVALID_INPUT:
            return "Invalid input parameters";
        case ERROR_MEMORY:
            return "Memory allocation failed";
        default:
            return "Unknown error";
    }
}

void free_response_buffer(ResponseBuffer* resp) {
    if (resp && resp->buffer) {
        free(resp->buffer);
        resp->buffer = NULL;
        resp->size = 0;
        resp->capacity = 0;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    ResponseBuffer response;
    ParseError error = init_response_buffer(&response, BUFFER_SIZE);
    if (error != SUCCESS) {
        fprintf(stderr, "Error: %s\n", get_error_message(error));
        return 1;
    }

    error = parse_json_responses(argv[1], &response);
    if (error != SUCCESS) {
        fprintf(stderr, "Error: %s\n", get_error_message(error));
        free_response_buffer(&response);
        return 1;
    }

    printf("%s", response.buffer);
    free_response_buffer(&response);
    return 0;
}
