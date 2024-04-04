CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Wno-deprecated-declarations
LIBS = -lyaml -lssl -lcrypto -lpthread

TARGET = fg
SOURCE = main.c yamlConfigParser.c directoryAnalyzer.c hashCalculator.c multiThreading.c config.c logging.c notifications.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS) 

debug: $(TARGET)
	gdb ./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean

