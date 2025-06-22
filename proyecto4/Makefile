CC = gcc
CFLAGS = $(shell sdl2-config --cflags --libs) -lm
TARGET = proyecto4
SOURCES = proyecto4.c utils.c universal_world.c
IMAGES = scene.ppm

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS)

clean:
	rm -f $(TARGET) $(IMAGES)