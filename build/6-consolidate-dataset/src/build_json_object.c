#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void encode_json_string(const char *input, FILE *output) {
    fputc('"', output);
    while (*input) {
        switch (*input) {
            case '"': fputs("\\\"", output); break;
            case '\\': fputs("\\\\", output); break;
            case '\b': fputs("\\b", output); break;
            case '\f': fputs("\\f", output); break;
            case '\n': fputs("\\n", output); break;
            case '\r': fputs("\\r", output); break;
            case '\t': fputs("\\t", output); break;
            default:
                if ((unsigned char)*input < 0x20 || (unsigned char)*input > 0x7E) {
                    fprintf(output, "\\u%04x", (unsigned char)*input);
                } else {
                    fputc(*input, output);
                }
        }
        input++;
    }
    fputc('"', output);
}

void read_file(const char *filename, char **content, size_t *size) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *content = malloc(*size + 1);
    if (!*content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fread(*content, 1, *size, file);
    (*content)[*size] = '\0';
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s --summary [file1] --logs [file2] --embeddings [file3]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *summary_file = NULL;
    char *logs_file = NULL;
    char *embeddings_file = NULL;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "--summary") == 0) {
            summary_file = argv[i + 1];
        } else if (strcmp(argv[i], "--logs") == 0) {
            logs_file = argv[i + 1];
        } else if (strcmp(argv[i], "--embeddings") == 0) {
            embeddings_file = argv[i + 1];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    if (!summary_file || !logs_file || !embeddings_file) {
        fprintf(stderr, "Error: Missing required files\n");
        return EXIT_FAILURE;
    }

    char *summary_content;
    size_t summary_size;
    read_file(summary_file, &summary_content, &summary_size);

    char *logs_content;
    size_t logs_size;
    read_file(logs_file, &logs_content, &logs_size);

    char *embeddings_content;
    size_t embeddings_size;
    read_file(embeddings_file, &embeddings_content, &embeddings_size);

    printf("{\n");
    printf("  \"summary\": ");
    encode_json_string(summary_content, stdout);
    printf(",\n");

    printf("  \"logs\": %s,\n", logs_content);

    printf("  \"embeddings\": %s\n", embeddings_content);
    printf("}\n");

    free(summary_content);
    free(logs_content);
    free(embeddings_content);

    return EXIT_SUCCESS;
}

