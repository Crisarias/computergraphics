#include <stdbool.h>

#ifndef UTILS_H
#define UTILS_H

typedef struct
{
    int x0;
    int y0;
    int x1;
    int y1;
} Line;

typedef struct {
    int x0, y0, x1, y1;
    bool active;
} Border;

typedef struct {
    unsigned char r,g,b;
} Pixel;

typedef struct {
    unsigned char r,g,b;
} Texel;

typedef struct {
    Texel **texel_array;
    char *filepath;
    int resWidth, resHeight;
} Texture;

typedef struct {
    int x;
    bool is_y_low;
    bool is_y_high;
} IntersectionPoint;

int compareBorders(const void *a, const void *b);

int compareIntersactions(const void *a, const void *b);

Texture* create_texture_from_ppm(char *filepath_str);

void freeTextureMemory(Texture* texture);

#endif