#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024

//just grep for "String Line" to get the possible log messages

enum TokenType {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_COMMENT,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_UNKNOWN
};

const char *keywords[] = {
    "auto", "break", "case", "char", "const", "continue",
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "int", "long", "register",
    "return", "short", "signed", "sizeof", "static", "struct",
    "switch", "typedef", "union", "unsigned", "void", "volatile",
    "while", NULL
};

int is_keyword(const char *str) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(keywords[i], str) == 0)
            return 1;
    }
    return 0;
}

int tokenize_and_color(const char *line,int linenum,int incomment) {
    const char *p = line;
    int returnInComment=incomment;
    while (*p) {
        // Skip whitespace

        if (isspace(*p)) {
            putchar(*p);
            p++;
            continue;
        }


        // Handle comments

        if (*p == '/' && p[1] == '/') {
            printf("\nComment Line %d:\033[36m",linenum); // Cyan for single-line comments
            while (*p && *p != '\n')
                putchar(*p++);
            printf("\033[0m");
            continue;
        }


        if (*p == '/' && p[1] == '*') {
            returnInComment = 1;
            printf("\nComment Line %d:\033[36m",linenum); // Cyan for multi-line comments
            p += 2;
            while (*p && !(*p == '*' && p[1] == '/'))
                putchar(*p++);
            if (*p)
                p += 2; // Skip past '* /'
            printf("\033[0m");
            continue;
        }
        
        if (*p == '*' && p[1] == '/') {
            returnInComment =0; 
        }

        
        // Handle strings 
        if (*p == '"') {
            printf("\nString Line %d:\033[32m",linenum); // Green for strings
            putchar(*p);
            p++;
            while (*p && *p != '"') {
                if (*p == '\\') {
                    putchar(*p);
                    p++;
                }
                putchar(*p);
                p++;
            }
            if (*p == '"')
                putchar(*p);
            printf("\033[0m\n");
            p++;
            continue;
        }
        // Handle identifiers and keywords
        if (isalnum(*p) || *p == '_') {
            char token[MAX_LINE_LENGTH];
            int i = 0;
            while (isalnum(*p) || *p == '_')
                token[i++] = *p++;
            token[i] = '\0';
            if (is_keyword(token)) {
                printf("\033[35m"); // Magenta for keywords
                printf("%s", token);
                printf("\033[0m");
            } else {
                printf("\033[33m"); // Yellow for identifiers
                printf("%s", token);
                printf("\033[0m");
            }
            continue;
        }
        // Handle numbers
        if (isdigit(*p) || (*p == '.' && isdigit(p[1]))) {
            printf("\033[34m"); // Blue for numbers
            while (isdigit(*p) || *p == '.' || (*p == 'e' && (isalnum(p[1]) || p[1] == '+' || p[1] == '-')))
                putchar(*p++);
            printf("\033[0m");
            continue;
        }
        // Handle operators and punctuation
        printf("\033[31m"); // Red for operators and punctuation
        putchar(*p);
        printf("\033[0m");
        p++;
    }
    return returnInComment;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }
    char line[MAX_LINE_LENGTH];
    int linenum = 1; int incomment=0;
    while (fgets(line, sizeof(line), file)) {
        incomment = tokenize_and_color(line,linenum,incomment);
        linenum++;
    }
    putchar('\n');
    fclose(file);
    return 0;
}
