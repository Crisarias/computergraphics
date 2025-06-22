CC = gcc
CFLAGS = $(shell sdl2-config --cflags --libs) -lm
TARGET = proyecto1
SOURCES = proyecto1.c utils.c
IMAGES = BruteForceOutput.ppm IncrementalOutput.ppm IncrementalV2Output.ppm BresenhamV2Output.ppm

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS)

clean:
	rm -f $(TARGET) $(IMAGES)