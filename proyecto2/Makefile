CC = gcc
CFLAGS = $(shell sdl2-config --cflags --libs) -lm
TARGET = proyecto2
SOURCES = proyecto2.c utils.c universal_world.c signature.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS)

clean:
	rm -f $(TARGET)