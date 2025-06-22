#ifndef UTILS_H
#define UTILS_H

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
    double x,y,z;
} Vector;

typedef struct {
    Vector origin;
    Vector direction;
} Ray;

Vector vector_init(double x, double y, double z);

Vector vector_add(Vector a, Vector b);

Vector vector_subtract(Vector a, Vector b);

Vector vector_multiply(Vector a, double scalar);

Vector vector_product(Vector a, Vector b);

double vector_dot(Vector a, Vector b);

double vector_length(Vector v);

Vector vector_normalize(Vector v);

Texture* create_texture_from_ppm(char *filepath_str);

void freeTextureMemory(Texture* texture);


#endif

#ifndef EPSILON
#define  EPSILON 0.0005
#endif