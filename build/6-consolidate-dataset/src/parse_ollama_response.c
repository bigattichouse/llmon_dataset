#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096
#define RESPONSE_KEY "\"response\":"

// Function to extract and concatenate all response values from the file
void parse_json_responses(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return;
    }

    char buffer[BUFFER_SIZE];
    char response_buffer[BUFFER_SIZE * 10] = ""; // Accumulator for concatenated responses

    while (fgets(buffer, sizeof(buffer), file)) {
        char *response_start = strstr(buffer, RESPONSE_KEY);
        if (response_start) {
            response_start += strlen(RESPONSE_KEY); // Move past "response":

            // Skip leading spaces
            while (*response_start == ' ') {
                response_start++;
            }

            // Check if response starts with a quote
            if (*response_start == '"') {
                response_start++;
                // Find the end of the response value
                char *response_end = response_start;
                while (*response_end && *response_end != '"') {
                    if (*response_end == '\\' && *(response_end + 1) == 'n') {
                        // Replace escaped newline with an actual newline
                        strncat(response_buffer, response_start, response_end - response_start);
                        strcat(response_buffer, "\n");
                        response_end += 2; // Skip past \n
                        response_start = response_end;
                    } else {
                        response_end++;
                    }
                }

                // Copy remaining response value
                strncat(response_buffer, response_start, response_end - response_start);
            }
        }
    }

    fclose(file);

    // Output the concatenated responses
    printf("%s", response_buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    parse_json_responses(argv[1]);
    return 0;
}

