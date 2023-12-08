CC = gcc
OBJ = obj
SRC = src
BIN = bin
DIRS = $(OBJ) $(BIN)
INCS = -Iinclude
CFLAGS = -g  $(INCS) -std=c99

SOURCES = shell.c

# Modify this line to include the source files for mytimeout
TIMEOUT_SOURCES = mytimeout.c

# Define the object files for both programs
OBJECTS = $(SOURCES:.c=.o)
TIMEOUT_OBJECTS = $(TIMEOUT_SOURCES:.c=.o)

# Define the target executable names
TARGET = bin/shell
TIMEOUT_TARGET = bin/mytimeout

# Default target
all: $(TARGET) $(TIMEOUT_TARGET)



# Compile the myshell executable
$(TARGET): $(OBJ)/$(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)/$(OBJECTS)

$(OBJ)/$(OBJECTS): $(SRC)/$(SOURCES)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile the mytimeout executable
$(TIMEOUT_TARGET): $(OBJ)/$(TIMEOUT_OBJECTS)
	$(CC) $(CFLAGS) -o $(TIMEOUT_TARGET) $(OBJ)/$(TIMEOUT_OBJECTS)

$(OBJ)/$(TIMEOUT_OBJECTS): $(SRC)/$(TIMEOUT_SOURCES)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile the object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJ)/$(OBJECTS) $(TARGET) $(OBJ)/$(TIMEOUT_OBJECTS) $(TIMEOUT_TARGET)

run:
	./$(BIN)/shell

$(shell mkdir -p $(DIRS))

.PHONY: run clean all
	rm $(OBJ)/*.o $(EXEC)

$(shell mkdir -p $(DIRS))

.PHONY: run clean all
