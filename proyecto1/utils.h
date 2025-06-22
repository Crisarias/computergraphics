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
    unsigned char r,g,b;
} Pixel;

void validate_number_argument(char *option);

void validate_number_value(int value, char *option);

#endif