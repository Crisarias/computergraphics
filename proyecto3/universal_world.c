#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "universal_world.h"

void free_universal_world(Universal_world *universal_world)
{
    free(universal_world->sphere_array);
    free(universal_world->light_source_array);
    free(universal_world);
    universal_world = NULL;
}

Universal_world *create_universal_world_from_file(char *filepath_str)
{
    FILE *file = fopen(filepath_str, "r");
    if (!file)
    {
        perror("Failed to open file");
        exit(1);
    } 

    Universal_world *world = (Universal_world *)malloc(sizeof(Universal_world));
    world->sphere_array = NULL;
    world->spheres_count = 0;
    int spheres_count = 0;
    int lights_count = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    bool read_sphere = false;
    bool read_light = false;
    bool read_light_factors = false;
    bool read_light_color = false;
    bool read_sphere_factors = false;
    bool read_color = false;
    bool read_projection_window = false;
    bool read_eye = false;
    bool read_ambient_light = false;
    bool read_output = false;
    int *r, *g, *b;
    r = (int *)malloc(sizeof(int));
    g = (int *)malloc(sizeof(int));
    b = (int *)malloc(sizeof(int));
    double *minX, *minY, *maxX, *maxY;
    minX = (double *)malloc(sizeof(double));
    minY = (double *)malloc(sizeof(double));
    maxX = (double *)malloc(sizeof(double));
    maxY = (double *)malloc(sizeof(double));
    double *x, *y, *z, *temp_factor, *temp_factor2, *temp_factor3, *temp_factor4;
    int *int_factor;
    x = (double *)malloc(sizeof(double));
    y = (double *)malloc(sizeof(double));
    z = (double *)malloc(sizeof(double));
    temp_factor = (double *)malloc(sizeof(double));
    temp_factor2 = (double *)malloc(sizeof(double));
    temp_factor3 = (double *)malloc(sizeof(double));
    temp_factor4 = (double *)malloc(sizeof(double));
    int_factor = (int *)malloc(sizeof(int));
    while ((read = getline(&line, &len, file)) != -1)
    {
        if (line[0] == '#')
        {
            continue; // Skip comment lines
        }
        if (line[0] == 'O')
        {
            read_output = true;
            continue; // Move to next line
        }
        if (line[0] == 'P')
        {
            read_projection_window = true;
            continue; // Move to next line
        }
        if (line[0] == 'E')
        {
            read_eye = true;
            continue; // Move to next line
        }
        if (line[0] == 'S')
        {
            read_sphere = true;
            continue; // Move to next line
        }
        if (line[0] == 'L')
        {
            read_light = true;
            continue; // Move to next line
        }
        if (line[0] == 'A')
        {
            read_ambient_light = true;
            continue; // Move to next line
        }
        // world
        if(read_output)
        {
            if (line[0] == '1')
            {
                world->output_to_screen = true;
            }
            else
            {
                world->output_to_screen = false;
            }
            read_output = false;
            continue; // Move to next line
        }
        if(read_eye)
        {
            sscanf(line, "%lf,%lf,%lf", x, y, z);
            world->eye.x = *x;
            world->eye.y = *y;
            world->eye.z = *z;
            read_eye = false;
            continue; // Move to next line
        }
        if (read_projection_window)
        {
            sscanf(line, "%lf,%lf,%lf,%lf", minX, minY, maxX, maxY);
            world->projection_minX = *minX;
            world->projection_minY = *minY;
            world->projection_maxX = *maxX;
            world->projection_maxY = *maxY;
            read_projection_window = false;
            continue; // Move to next line
        }
        if (read_ambient_light)
        {
            sscanf(line, "%lf", temp_factor);
            world-> ia = *temp_factor;
            read_ambient_light = false;
            continue; // Move to next line
        }
        // Spheres
        if (read_sphere)
        {
            spheres_count++;
            sscanf(line, "%lf,%lf,%lf", x, y, z);
            world->sphere_array = (Sphere *)realloc(world->sphere_array, sizeof(Sphere) * (spheres_count));
            world->sphere_array[spheres_count-1].center.x = *x;
            world->sphere_array[spheres_count-1].center.y = *y;
            world->sphere_array[spheres_count-1].center.z = *z;
            read_sphere = false;
            read_sphere_factors = true;
            continue; // Move to next line
        }
        if (read_sphere_factors)
        {
            sscanf(line, "%lf,%lf,%lf,%lf,%d", temp_factor, temp_factor2, temp_factor3, temp_factor4, int_factor);
            world->sphere_array[spheres_count-1].radius = *temp_factor;
            world->sphere_array[spheres_count-1].kd = *temp_factor2;
            world->sphere_array[spheres_count-1].ka = *temp_factor3;
            world->sphere_array[spheres_count-1].ks = *temp_factor4;
            world->sphere_array[spheres_count-1].kn = *int_factor;
            read_sphere_factors = false;
            read_color = true;
            continue; // Move to next line
        }
        if (read_color)
        {
            sscanf(line, "%d,%d,%d", r, g, b);
            world->sphere_array[spheres_count-1].r = *r;
            world->sphere_array[spheres_count-1].g = *g;
            world->sphere_array[spheres_count-1].b = *b;
            read_color = false;
            continue; // Move to next line
        }
        // Light sources
        if (read_light)
        {
            lights_count++;
            sscanf(line, "%lf,%lf,%lf", x, y, z);
            world->light_source_array = (LightSource *)realloc(world->light_source_array, sizeof(LightSource) * (lights_count));
            world->light_source_array[lights_count-1].point.x = *x;
            world->light_source_array[lights_count-1].point.y = *y;
            world->light_source_array[lights_count-1].point.z = *z;
            read_light = false;
            read_light_factors = true;
            continue; // Move to next line
        }
        if (read_light_factors)
        {
            sscanf(line, "%lf,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, temp_factor4);
            world->light_source_array[lights_count-1].lp = *temp_factor;
            world->light_source_array[lights_count-1].c1 = *temp_factor2;
            world->light_source_array[lights_count-1].c2 = *temp_factor3;
            world->light_source_array[lights_count-1].c3 = *temp_factor4;
            read_light_factors = false;
            read_light_color = true;
            continue; // Move to next line
        }
        if (read_light_color)
        {
            sscanf(line, "%d,%d,%d", r, g, b);
            world->light_source_array[lights_count-1].r = *r;
            world->light_source_array[lights_count-1].g = *g;
            world->light_source_array[lights_count-1].b = *b;
            read_light_color = false;
            continue; // Move to next line
        }
    }
    world->spheres_count = spheres_count;
    world->lights_count = lights_count;
    free(line);
    free(r);
    free(g);
    free(b);
    free(minX);
    free(minY);
    free(maxX);
    free(maxY);
    free(x);
    free(y);
    free(z);
    free(temp_factor);
    free(temp_factor2);
    free(temp_factor3);
    free(temp_factor4);
    free(int_factor);
    fclose(file);
    
    return world;
}