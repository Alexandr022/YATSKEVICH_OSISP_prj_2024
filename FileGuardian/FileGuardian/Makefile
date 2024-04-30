CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -D_DEFAULT_SOURCE -Wno-deprecated-declarations $(shell pkg-config --cflags libnotify)
LIBS = $(shell pkg-config --libs libnotify) -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lssl -lcrypto -lyaml -lpthread

BUILD_DIR = build
SRCS = src/main.c src/monitor.c src/hash.c src/notifications.c src/thread.c src/config.c 
OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))
EXEC = monitor

$(BUILD_DIR)/%.o: src/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

debug: $(EXEC)
	gdb ./$(EXEC)

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

.PHONY: clean
