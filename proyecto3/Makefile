CC = gcc
CFLAGS = $(shell sdl2-config --cflags --libs) -lm
TARGET = proyecto3
SOURCES = proyecto3.c utils.c universal_world.c
IMAGES = scene.ppm

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS)

clean:
	rm -f $(TARGET) $(IMAGES)