# Compiler
CC = gcc

# Directories
SRCDIR = src
BUILDDIR = build
BINDIR = bin

# Source files
SERVER_SRC = $(SRCDIR)/server.c
CLIENT_SRC = $(SRCDIR)/client.c

# Output binaries
SERVER_BIN = $(BINDIR)/server
CLIENT_BIN = $(BINDIR)/client

# Compiler flags
CFLAGS = -Wall -g

# Targets
all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_SRC)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_SRC)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BINDIR) $(BUILDDIR)

.PHONY: all clean