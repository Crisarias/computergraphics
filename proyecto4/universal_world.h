
#include <stdbool.h>
#include "utils.h"

#ifndef UNIVERSAL_WORLD_H
#define UNIVERSAL_WORLD_H

typedef struct {
    Vector center;
    Vector p1, p2, p3; // Points to calculate the cut plane
    double A, B, C, D;  // Plane equation coefficients
    bool has_cut_plane; // Indicates if the sphere has a cut plane
    bool cutTop;
    double radius;
    double kd; // Diffuse coefficient
    double ks; // Specular coefficient
    double ka; // Ambient coefficient
    double o1; // object color
    double o2; // reflective color
    double o3; // transparent color
    int kn; // Shininess coefficient
    unsigned char r,g,b;
} Sphere;

typedef struct {
    Vector center;
    Vector p1, p2; // Points to calculate the plane
    double A, B, C, D;  // Plane equation coefficients
    double radius;
    double kd; // Diffuse coefficient
    double ks; // Specular coefficient
    double ka; // Ambient coefficient
    double o1; // object color
    double o2; // reflective color
    double o3; // transparent color
    int kn; // Shininess coefficient
    unsigned char r,g,b;
} Disc;

typedef struct {
    Vector d1; // First point of the cylinder
    Vector d2; // Second point of the cylinder
    Vector Q; // Axis vector of the cylinder
    double radius;
    double kd; // Diffuse coefficient
    double ks; // Specular coefficient
    double ka; // Ambient coefficient
    double o1; // object color
    double o2; // reflective color
    double o3; // transparent color
    int kn; // Shininess coefficient
    unsigned char r,g,b;
} Cylinder;

typedef struct {
    Vector d1; // First point of the cylinder
    Vector d2; // Second point of the cylinder
    double k1;
    double k2;
    Vector Q; // Axis vector of the cylinder
    double radius;
    double kd; // Diffuse coefficient
    double ks; // Specular coefficient
    double ka; // Ambient coefficient
    double o1; // object color
    double o2; // reflective color
    double o3; // transparent color
    int kn; // Shininess coefficient
    unsigned char r,g,b;
} Cone;

typedef struct {
    Vector *vertor_array;
    Vector normal; // Normal vector of the polygon
    double A, B, C, D;  // Plane equation coefficients
    int vector_count;
    char *texture_path;
    double texture_scale;
    int texture_width;
    int texture_height;
    char discard_coord; // 'x' for x coordinate, 'y' for y coordinate, 'z' for z coordinate
    double kd; // Diffuse coefficient
    double ks; // Specular coefficient
    double ka; // Ambient coefficien
    double o1; // object color
    double o2; // reflective color
    double o3; // transparent color    t
    int kn; // Shininess coefficient
    unsigned char r,g,b;
} Polygon;

typedef struct {
    Vector point;
    double lp; // Light power
    double c1, c2, c3; // Attenuation coefficients
    unsigned char r,g,b;
} LightSource;

typedef struct {
    Sphere *sphere;
    Polygon *polygon;
    Cylinder *cylinder;
    Disc *disc;
    Cone *cone;
    Vector intersection_point;
    double t;
} Intersection;

typedef struct {
    Ray ray;
    Pixel pixel;
    bool valid;
} IntersectionCacheEntry;

typedef struct {
    Sphere *sphere_array;
    Cylinder *cylinder_array;
    Polygon *polygon_array;
    Disc *disc_array;
    Cone *cone_array;
    LightSource *light_source_array;
    Vector eye;
    bool output_to_screen;
    double projection_minX, projection_minY, projection_maxX, projection_maxY;
    double ia; // Ambient light intensity;
    int spheres_count;
    int cylinders_count;
    int polygons_count;
    int discs_count;
    int cones_count;
    int lights_count;
} Universal_world;

Universal_world *create_universal_world_from_file(char *filepath_str);

void free_universal_world(Universal_world *universal_world);

unsigned int ray_hash(Ray ray);

bool rays_equal(Ray a, Ray b);

Intersection* deep_copy_intersection(Intersection* original);

#endif

#ifndef M_PI
#define  M_PI 3.14159265358979323846
#endif

#ifndef CACHE_SIZE
#define CACHE_SIZE 65536
#endif

#ifndef CACHE_MASK
#define CACHE_MASK (CACHE_SIZE - 1)
#endif