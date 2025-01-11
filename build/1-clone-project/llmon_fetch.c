#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <git2.h>
#include <getopt.h>

#define DEFAULT_HOST "https://github.com"
#define BUFFER_SIZE 1024

void print_help() {
    printf("Usage: llmon_checkout [options] organization/repository\n\n");
    printf("Options:\n");
    printf("  --help              Show this help message\n");
    printf("  --repo URL          Specify base repository URL (default: https://github.com)\n\n");
    printf("Examples:\n");
    printf("  llmon_checkout apache/httpd\n");
    printf("  llmon_checkout --repo https://bitbucket.com apache/httpd\n");
}

void checkout_progress(const char *path, size_t completed_steps, size_t total_steps, void *payload) {
    (void)payload;
    printf("\rCloning %s: %.0f%%", path, (double)completed_steps / total_steps * 100);
    fflush(stdout);
}

void check_error(int error, const char* message) {
    if (error < 0) {
        const git_error *e = git_error_last();
        printf("\nError %s: %s\n", message, e ? e->message : "unknown");
        git_libgit2_shutdown();
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    char *repo_base = DEFAULT_HOST;
    char clone_url[BUFFER_SIZE];
    char *repo_path = NULL;
    
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"repo", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "hr:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                print_help();
                return 0;
            case 'r':
                repo_base = optarg;
                break;
            case '?':
                return 1;
        }
    }

    if (optind >= argc) {
        print_help();
        return 1;
    }

    repo_path = argv[optind];

    git_libgit2_init();

    snprintf(clone_url, BUFFER_SIZE, "%s/%s.git", repo_base, repo_path);
    printf("Cloning from: %s\n", clone_url);

    // Initialize checkout options
    git_checkout_options checkout_opts;
    memset(&checkout_opts, 0, sizeof(git_checkout_options));
    checkout_opts.version = GIT_CHECKOUT_OPTIONS_VERSION;
    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.progress_cb = checkout_progress;
    checkout_opts.disable_filters = 0;

    // Initialize clone options
    git_clone_options clone_opts;
    memset(&clone_opts, 0, sizeof(git_clone_options));
    clone_opts.version = GIT_CLONE_OPTIONS_VERSION;
    clone_opts.checkout_opts = checkout_opts;
    git_clone_init_options(&clone_opts, GIT_CLONE_OPTIONS_VERSION);
     
    

    char *last_slash = strrchr(repo_path, '/');
    const char *local_path = last_slash ? last_slash + 1 : repo_path;

    git_repository *repo = NULL;
    int error = git_clone(&repo, clone_url, local_path, &clone_opts);
    
    if (error == 0) {
        printf("\nSuccessfully cloned repository to: %s\n", local_path);
    } else {
        const git_error *e = git_error_last();
        printf("\nClone failed: %s\n", e ? e->message : "unknown error");
    }

    if (repo) git_repository_free(repo);
    git_libgit2_shutdown();

    return error ? 1 : 0;
}
