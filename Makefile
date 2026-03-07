CC = gcc
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0` -lsqlite3
TARGET = vibe-journal

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean