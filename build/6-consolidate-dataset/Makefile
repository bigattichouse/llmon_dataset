CC = gcc
PKGCONFIG = pkg-config

CFLAGS = $(shell $(PKGCONFIG) ) -Wall -Wextra -O2
LDFLAGS = $(shell $(PKGCONFIG))

PREFIX = /usr/local
LOCALBIN_DIR = bin
BINDIR = $(PREFIX)/bin

TARGET = parse_ollama_response
SRC_DIR = src
BUILD_DIR = build
SRC = $(SRC_DIR)/parse_ollama_response.c
OBJ = $(BUILD_DIR)/parse_ollama_response.o

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ)
	mkdir -p $(BUILD_DIR)
	mkdir -p $(LOCALBIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)
	mv $(BUILD_DIR)/$(TARGET) ./$(LOCALBIN_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

install: $(BUILD_DIR)/$(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(BUILD_DIR)/$(TARGET) $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

check-deps:
	@which $(PKGCONFIG) > /dev/null || (echo "pkg-config not found"; exit 1)
	@$(PKGCONFIG) --exists libgit2 || (echo "libgit2 development files not found"; exit 1)
	@echo "All dependencies are satisfied"

help:
	@echo "Available targets:"
	@echo "  all        - Build the program (default)"
	@echo "  install    - Install the program to $(BINDIR)"
	@echo "  uninstall  - Remove the installed program"
	@echo "  clean      - Remove build files"
	@echo "  check-deps - Check for required dependencies"
	@echo "  help       - Show this help message"

.PHONY: all install uninstall clean check-deps help

