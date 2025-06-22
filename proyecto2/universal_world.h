#ifndef UNIVERSAL_WORLD_H
#define UNIVERSAL_WORLD_H

typedef struct {
    double x,y;
    int xf,yf;
} Vertex;

typedef struct {
    Vertex *original_vertex_array;
    Vertex *next_vertex_array;
    double minX, minY, maxX, maxY;
    int original_vertex_count;
    int next_vertex_count;
    bool needs_clipping;
    bool is_out_of_window;
    unsigned char r,g,b;
    char *texture_filepath;
} Poligon;

typedef struct {
    Poligon *poligon_array;
    int poligon_count;
    int total_vertex_count;
} Universal_world;

Universal_world *create_universal_world_from_file(char *filepath_str);

void free_universal_world(Universal_world *universal_world);

void clipping_universal_world(Universal_world *universal_world, double minX, double minY, double maxX, double maxY);

void rotate(Universal_world *universal_world, double xC, double yC, int angle_degree);

#endif

#ifndef M_PI
#define  M_PI 3.14159265358979323846
#endif