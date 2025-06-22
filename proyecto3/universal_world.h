
#include <stdbool.h>
#include "utils.h"

#ifndef UNIVERSAL_WORLD_H
#define UNIVERSAL_WORLD_H

typedef struct {
    Vector center;
    double radius;
    double kd; // Diffuse coefficient
    double ks; // Specular coefficient
    double ka; // Ambient coefficient
    int kn; // Shininess coefficient
    unsigned char r,g,b;
} Sphere;

typedef struct {
    Vector point;
    double lp; // Light power
    double c1, c2, c3; // Attenuation coefficients
    unsigned char r,g,b;
} LightSource;

typedef struct {
    Sphere *sphere;
    Vector intersection_point;
    double t;
} Intersection;

typedef struct {
    Sphere *sphere_array;
    LightSource *light_source_array;
    Vector eye;
    bool output_to_screen;
    double projection_minX, projection_minY, projection_maxX, projection_maxY;
    double ia; // Ambient light intensity;
    int spheres_count;
    int lights_count;
} Universal_world;

Universal_world *create_universal_world_from_file(char *filepath_str);

void free_universal_world(Universal_world *universal_world);

#endif

#ifndef M_PI
#define  M_PI 3.14159265358979323846
#endif