CC ?= gcc
CFLAGS ?= -Wall -Wextra
LDFLAGS ?=
TARGET = openmotion-host
SRC = main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
