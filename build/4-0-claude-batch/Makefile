# Makefile for Anthropic Batch API Client

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -O2
LDFLAGS = -lcurl -lssl -lcrypto

# Main target
all: anthropic_batch

# Compile the program
anthropic_batch: anthropic_batch.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f anthropic_batch

# Install to system (optional)
install: anthropic_batch
	mkdir -p $(DESTDIR)/usr/local/bin
	cp anthropic_batch $(DESTDIR)/usr/local/bin/
	chmod 755 $(DESTDIR)/usr/local/bin/anthropic_batch

# Uninstall from system
uninstall:
	rm -f $(DESTDIR)/usr/local/bin/anthropic_batch

# Check if curl is available
check-deps:
	@echo "Checking for required dependencies..."
	@which curl >/dev/null 2>&1 || { echo "Error: curl not found. Please install curl."; exit 1; }
	@echo "Checking if curl-config is available..."
	@which curl-config >/dev/null 2>&1 || { echo "Warning: curl-config not found. You may need to install libcurl development package."; }
	@if which curl-config >/dev/null 2>&1; then \
		echo "Found libcurl version: $$(curl-config --version)"; \
	fi

# Help target
help:
	@echo "Anthropic Batch API Client Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all         : Build the program (default target)"
	@echo "  clean       : Remove build artifacts"
	@echo "  install     : Install the program to /usr/local/bin"
	@echo "  uninstall   : Remove the program from /usr/local/bin"
	@echo "  check-deps  : Check for required dependencies"
	@echo "  help        : Display this help message"

.PHONY: all clean install uninstall check-deps help
