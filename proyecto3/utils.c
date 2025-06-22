#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "utils.h"

Vector vector_init(double x, double y, double z) {
    Vector v = {x, y, z};
    return v;
}

Vector vector_subtract(Vector a, Vector b) {
    return vector_init(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector vector_multiply(Vector a, double scalar) {
    return vector_init(a.x * scalar, a.y * scalar, a.z * scalar);
}

Vector vector_product(Vector a, Vector b) {
    return vector_init(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

double vector_dot(Vector a, Vector b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double vector_length(Vector v) {
    return sqrt(vector_dot(v, v));
}

Vector vector_normalize(Vector v) {
    double length = vector_length(v);
    return vector_init(v.x / length, v.y / length, v.z / length);
}

// This function expect ppm formated with GIMP version 3.0.0 PNM plug-in
Texture *create_texture_from_ppm(char *filepath_str)
{
    FILE *file = fopen(filepath_str, "r");
    if (!file)
    {
        fprintf(stderr, "Error: Unable to open file %s\n", filepath_str);
        exit(1);
    }

    Texture *texture = (Texture *)malloc(sizeof(Texture));
    if (!texture)
    {
        fprintf(stderr, "Error: Unable to allocate memory for texture %s\n", filepath_str);
        fclose(file);
        exit(1);
    }
    texture->resHeight = 0;
    texture->resWidth = 0;
    texture->texel_array = NULL;
    texture->filepath = (char *)malloc((strlen(filepath_str) + 1) * sizeof(char));
    sscanf(filepath_str, "%s", texture->filepath);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    bool read_color = false;
    int *r, *g, *b, *width, *height;
    r = (int *)malloc(sizeof(int));
    g = (int *)malloc(sizeof(int));
    b = (int *)malloc(sizeof(int));
    width = (int *)malloc(sizeof(int));
    height = (int *)malloc(sizeof(int));

    int numLine = 0;
    int nextX = 0;
    int nextY = 0;
    int readMode = 0;

    while ((read = getline(&line, &len, file)) != -1)
    {
        if (line[0] == '#')
        {
            numLine++;
            continue; // Skip comment lines
        }
        if (numLine == 0 && (line[0] != 'P' || line[1] != '3'))
        {
            printf("Error: Unsupported file %s format (must be P3)\n", filepath_str);
            exit(1);
        }
        if (numLine == 2)
        {
            sscanf(line, "%d %d", width, height);
            texture->resWidth = *width;
            texture->resHeight = *height;
            texture->texel_array = (Texel **)malloc(*height * sizeof(Texel *));
            if (texture->texel_array == NULL)
            {
                fprintf(stderr, "\nError: Cannot reserve memory for texture array %s\n", filepath_str);
                exit(1);
            }
            for (int i = 0; i < *height; i++)
            {
                texture->texel_array[i] = (Texel *)malloc(*width * sizeof(Texel));
                if (texture->texel_array[i] == NULL)
                {
                    fprintf(stderr, "\nError: Cannot reserve memory for texel in %s\n", filepath_str);
                    for (int j = 0; j < i; j++)
                    {
                        free(texture->texel_array[j]);
                    }
                    free(texture->texel_array);
                    exit(1);
                }
            }
        }
        if (numLine < 4)
        {
            numLine++;
            continue;
        }
        switch (readMode)
        {
        case 0:
            sscanf(line, "%d", r);
            readMode = 1;
            continue;
        case 1:
            sscanf(line, "%d", g);
            readMode = 2;
            continue;
        case 2:
            sscanf(line, "%d", b);
            readMode = 0;
            texture->texel_array[nextY][nextX].r = *r;
            texture->texel_array[nextY][nextX].g = *g;
            texture->texel_array[nextY][nextX].b = *b;
            nextX++;
            if (nextX == *width)
            {
                nextX = 0;
                nextY++;
            }
            continue;
        default:
            break;
        }
        numLine++;
    }
    free(line);
    free(r);
    free(g);
    free(b);
    free(width);
    free(height);
    fclose(file);
    return texture;
}

void freeTextureMemory(Texture *texture)
{
    for (int i = 0; i < texture->resHeight; i++)
    {
        free(texture->texel_array[i]);
    }
    free(texture->texel_array);
    free(texture->filepath);
    free(texture);
}