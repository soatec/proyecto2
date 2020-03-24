SOURCE_DIR  := ./src
INCLUDE_DIR := ./include
BUILD_DIR   := ./build

CC        := gcc
CFLAGS    := -Wall
LDFLAGS   := -lm -lpthread -lrt
INC_FLAGS := -I$(INCLUDE_DIR)

.PHONY: all
all: $(BUILD_DIR) $(BUILD_DIR)/secuencial $(BUILD_DIR)/forked $(BUILD_DIR)/threaded $(BUILD_DIR)/preforked $(BUILD_DIR)/prethreaded $(BUILD_DIR)/cliente

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INC_FLAGS)

$(BUILD_DIR)/secuencial: $(BUILD_DIR)/secuencial.o $(BUILD_DIR)/secuencial_main.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/forked: $(BUILD_DIR)/forked.o $(BUILD_DIR)/forked_main.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/threaded: $(BUILD_DIR)/threaded.o $(BUILD_DIR)/threaded_main.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/preforked: $(BUILD_DIR)/preforked.o $(BUILD_DIR)/preforked_main.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/prethreaded: $(BUILD_DIR)/prethreaded.o $(BUILD_DIR)/prethreaded_main.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/cliente: $(BUILD_DIR)/cliente.o $(BUILD_DIR)/cliente_main.o $(BUILD_DIR)/utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $@


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
