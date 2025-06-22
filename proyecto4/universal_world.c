#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "universal_world.h"

void get_polygon_plane_equation(Polygon *polygon)
{
    Vector v1 = vector_subtract(polygon->vertor_array[1], polygon->vertor_array[0]);
    Vector v2 = vector_subtract(polygon->vertor_array[2], polygon->vertor_array[0]);
    Vector normal = vector_normalize(vector_product(v1, v2));
    polygon->A = normal.x;
    polygon->B = normal.y;
    polygon->C = normal.z;
    polygon->D = - (polygon->A * polygon->vertor_array[0].x + polygon->B * polygon->vertor_array[0].y + polygon->C * polygon->vertor_array[0].z);
    polygon->normal = normal;

    // Choose coordinate to discard based on polygon normal
    int discard_coord;
    
    // Discard coordinate with largest normal component (project onto most stable plane)
    if (fabs(normal.x) >= fabs(normal.y) && fabs(normal.x) >= fabs(normal.z))
         polygon ->discard_coord = 'x'; // Discard x - project onto YZ plane
    else if (fabs(normal.y) >= fabs(normal.x) && fabs(normal.y) >= fabs(normal.z))
        polygon ->discard_coord = 'y'; // Discard y - project onto XZ plane
    else
        polygon ->discard_coord = 'z'; // Discard z - project onto XY plane

}

void get_disc_plane_equation(Disc *disc)
{
    Vector v1 = vector_subtract(disc->p1, disc->center);
    Vector v2 = vector_subtract(disc->p2, disc->center);
    Vector normal = vector_normalize(vector_product(v1, v2));
    disc->A = normal.x;
    disc->B = normal.y;
    disc->C = normal.z;
    disc->D = -(disc->A * disc->center.x + disc->B * disc->center.y + disc->C * disc->center.z);
}

void get_cut_plane_equation(double *A, double *B, double *C, double *D, Vector p1, Vector p2, Vector p3)
{
    Vector v1 = vector_subtract(p2, p1);
    Vector v2 = vector_subtract(p3, p1);
    Vector normal = vector_normalize(vector_product(v1, v2));
    *A = normal.x;
    *B = normal.y;
    *C = normal.z;
    *D = -(*A * p1.x + *B * p1.y + *C * p1.z);
}

void free_universal_world(Universal_world *universal_world)
{
    free(universal_world->sphere_array);
    free(universal_world->disc_array);
    free(universal_world->cylinder_array);
    free(universal_world->cone_array);
    free(universal_world->light_source_array);
    if(universal_world->polygon_array != NULL)
    {
        for (int i = 0; i < universal_world->polygons_count; i++)
        {
            if (universal_world->polygon_array[i].texture_path != NULL)
            {
                free(universal_world->polygon_array[i].texture_path);
            }
            free(universal_world->polygon_array[i].vertor_array);
        }
    }
    free(universal_world->polygon_array);
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
    world->cylinder_array = NULL;
    world->polygon_array = NULL;
    world->disc_array = NULL;
    world->cone_array = NULL;
    int spheres_count = 0;
    int polygons_count = 0;
    int cylinders_count = 0;
    int discs_count = 0;
    int cones_count = 0;
    int lights_count = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    bool read_sphere = false;
    bool read_disc = false;
    bool read_light = false;
    bool read_light_factors = false;
    bool read_light_color = false;
    bool read_factors = false;
    bool read_color = false;
    bool read_projection_window = false;
    bool read_eye = false;
    bool read_polygon = false;
    bool read_cylinder = false;
    bool read_cone = false;
    bool read_ambient_light = false;
    bool read_output = false;
    bool read_cut_plane = false;
    bool read_texture = false;
    char type = 'S';
    int *r, *g, *b;
    r = (int *)malloc(sizeof(int));
    g = (int *)malloc(sizeof(int));
    b = (int *)malloc(sizeof(int));
    double *temp_factor, *temp_factor2, *temp_factor3, *temp_factor4, *temp_factor5, *temp_factor6, *temp_factor7, *temp_factor8, *temp_factor9;
    int *int_factor;
    temp_factor = (double *)malloc(sizeof(double));
    temp_factor2 = (double *)malloc(sizeof(double));
    temp_factor3 = (double *)malloc(sizeof(double));
    temp_factor4 = (double *)malloc(sizeof(double));
    temp_factor5 = (double *)malloc(sizeof(double));
    temp_factor6 = (double *)malloc(sizeof(double));
    temp_factor7 = (double *)malloc(sizeof(double));
    temp_factor8 = (double *)malloc(sizeof(double));
    temp_factor9 = (double *)malloc(sizeof(double));
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
        if (line[0] == 'W')
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
            type = 'S';
            continue; // Move to next line
        }
        if (line[0] == 'P')
        {
            read_polygon = true;
            type = 'P';
            continue; // Move to next line
        }
        if (line[0] == 'C')
        {
            read_cylinder = true;
            type = 'C';
            continue; // Move to next line
        }
        if (line[0] == 'K')
        {
            read_cone = true;
            type = 'K';
            continue; // Move to next line
        }
        if (line[0] == 'D')
        {
            read_disc = true;
            type = 'D';
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
        if (line[0] == 'X')
        {
            read_cut_plane = true;
            continue; // Move to next line
        }
        if (line[0] == 'T')
        {
            read_texture = true;
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
            sscanf(line, "%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3);
            world->eye.x = *temp_factor;
            world->eye.y = *temp_factor2;
            world->eye.z = *temp_factor3;
            read_eye = false;
            continue; // Move to next line
        }
        if (read_projection_window)
        {
            sscanf(line, "%lf,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, temp_factor4);
            world->projection_minX = *temp_factor;
            world->projection_minY = *temp_factor2;
            world->projection_maxX = *temp_factor3;
            world->projection_maxY = *temp_factor4;
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
            sscanf(line, "%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3);
            world->sphere_array = (Sphere *)realloc(world->sphere_array, sizeof(Sphere) * (spheres_count));
            world->sphere_array[spheres_count-1].center.x = *temp_factor;
            world->sphere_array[spheres_count-1].center.y = *temp_factor2;
            world->sphere_array[spheres_count-1].center.z = *temp_factor3;
            read_sphere = false;
            read_factors = true;
            continue; // Move to next line
        }
        if (read_cut_plane && type == 'S')
        {
            double A, B, C, D;
            sscanf(line, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d", temp_factor, temp_factor2, temp_factor3, temp_factor4, temp_factor5, temp_factor6, temp_factor7, temp_factor8, temp_factor9, int_factor);
            world->sphere_array[spheres_count-1].p1.x = *temp_factor;
            world->sphere_array[spheres_count-1].p1.y = *temp_factor2;
            world->sphere_array[spheres_count-1].p1.z = *temp_factor3;
            world->sphere_array[spheres_count-1].p2.x = *temp_factor4;
            world->sphere_array[spheres_count-1].p2.y = *temp_factor5;
            world->sphere_array[spheres_count-1].p2.z = *temp_factor6;
            world->sphere_array[spheres_count-1].p3.x = *temp_factor7;
            world->sphere_array[spheres_count-1].p3.y = *temp_factor8;
            world->sphere_array[spheres_count-1].p3.z = *temp_factor9;
            if (*int_factor == 1)
            {
                world->sphere_array[spheres_count-1].cutTop = true;
            }
            else
            {
                world->sphere_array[spheres_count-1].cutTop = false;
            }
            get_cut_plane_equation(&A, &B, &C, &D, world->sphere_array[spheres_count-1].p1, 
                                   world->sphere_array[spheres_count-1].p2, 
                                   world->sphere_array[spheres_count-1].p3);
            world->sphere_array[spheres_count-1].A = A;
            world->sphere_array[spheres_count-1].B = B;
            world->sphere_array[spheres_count-1].C = C;
            world->sphere_array[spheres_count-1].D = D;
            world->sphere_array[spheres_count-1].has_cut_plane = true;
            read_cut_plane = false;
            continue; // Move to next line
        }
        if (read_factors && type == 'S')
        {
            sscanf(line, "%lf,%lf,%lf,%lf,%d,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, temp_factor4, int_factor, temp_factor5, temp_factor6, temp_factor7);
            world->sphere_array[spheres_count-1].radius = *temp_factor;
            world->sphere_array[spheres_count-1].kd = *temp_factor2;
            world->sphere_array[spheres_count-1].ka = *temp_factor3;
            world->sphere_array[spheres_count-1].ks = *temp_factor4;
            world->sphere_array[spheres_count-1].kn = *int_factor;
            world->sphere_array[spheres_count-1].o1 = *temp_factor5;
            world->sphere_array[spheres_count-1].o2 = *temp_factor6;
            world->sphere_array[spheres_count-1].o3 = *temp_factor7;
            read_factors = false;
            read_color = true;
            continue; // Move to next line
        }
        if (read_color && type == 'S')
        {
            sscanf(line, "%d,%d,%d", r, g, b);
            world->sphere_array[spheres_count-1].r = *r;
            world->sphere_array[spheres_count-1].g = *g;
            world->sphere_array[spheres_count-1].b = *b;
            read_color = false;
            continue; // Move to next line
        }
        // Discs
        if (read_disc)
        {
            discs_count++;
            sscanf(line, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3,temp_factor4,temp_factor5,temp_factor6,temp_factor7,temp_factor8,temp_factor9);
            world->disc_array = (Disc *)realloc(world->disc_array, sizeof(Disc) * (discs_count));
            world->disc_array[discs_count-1].center.x = *temp_factor;
            world->disc_array[discs_count-1].center.y = *temp_factor2;
            world->disc_array[discs_count-1].center.z = *temp_factor3;
            world->disc_array[discs_count-1].p1.x = *temp_factor4;
            world->disc_array[discs_count-1].p1.y = *temp_factor5;
            world->disc_array[discs_count-1].p1.z = *temp_factor6;
            world->disc_array[discs_count-1].p2.x = *temp_factor7;
            world->disc_array[discs_count-1].p2.y = *temp_factor8;
            world->disc_array[discs_count-1].p2.z = *temp_factor9;
            get_disc_plane_equation(&world->disc_array[discs_count-1]);
            read_disc = false;
            read_factors = true;
            continue; // Move to next line
        }
        if (read_factors && type == 'D')
        {
            sscanf(line, "%lf,%lf,%lf,%lf,%d,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, temp_factor4, int_factor, temp_factor5, temp_factor6, temp_factor7);
            world->disc_array[discs_count-1].radius = *temp_factor;
            world->disc_array[discs_count-1].kd = *temp_factor2;
            world->disc_array[discs_count-1].ka = *temp_factor3;
            world->disc_array[discs_count-1].ks = *temp_factor4;
            world->disc_array[discs_count-1].kn = *int_factor;
            world->disc_array[discs_count-1].o1 = *temp_factor5;
            world->disc_array[discs_count-1].o2 = *temp_factor6;
            world->disc_array[discs_count-1].o3 = *temp_factor7;
            read_factors = false;
            read_color = true;
            continue; // Move to next line
        }
        if (read_color && type == 'D')
        {
            sscanf(line, "%d,%d,%d", r, g, b);
            world->disc_array[discs_count-1].r = *r;
            world->disc_array[discs_count-1].g = *g;
            world->disc_array[discs_count-1].b = *b;
            read_color = false;
            continue; // Move to next line
        }
        // Cylinders
        if (read_cylinder)
        {
            cylinders_count++;
            sscanf(line, "%lf,%lf,%lf,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, temp_factor4, temp_factor5, temp_factor6);
            world->cylinder_array = (Cylinder *)realloc(world->cylinder_array, sizeof(Cylinder) * (cylinders_count));
            world->cylinder_array[cylinders_count-1].d1.x = *temp_factor;
            world->cylinder_array[cylinders_count-1].d1.y = *temp_factor2;
            world->cylinder_array[cylinders_count-1].d1.z = *temp_factor3;
            world->cylinder_array[cylinders_count-1].d2.x = *temp_factor4;
            world->cylinder_array[cylinders_count-1].d2.y = *temp_factor5;
            world->cylinder_array[cylinders_count-1].d2.z = *temp_factor6;
            // Calculate Q vector (axis direction) as normalized vector from d1 to d2
            Vector d1_to_d2 = vector_subtract(world->cylinder_array[cylinders_count-1].d2, 
            world->cylinder_array[cylinders_count-1].d1);
            world->cylinder_array[cylinders_count-1].Q = vector_normalize(d1_to_d2);
            read_cylinder = false;
            read_factors = true;
            continue; // Move to next line
        }
        if (read_factors && type == 'C')
        {
            sscanf(line, "%lf,%lf,%lf,%lf,%d,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, temp_factor4, int_factor, temp_factor5, temp_factor6, temp_factor7);
            world->cylinder_array[cylinders_count-1].radius = *temp_factor;
            world->cylinder_array[cylinders_count-1].kd = *temp_factor2;
            world->cylinder_array[cylinders_count-1].ka = *temp_factor3;
            world->cylinder_array[cylinders_count-1].ks = *temp_factor4;
            world->cylinder_array[cylinders_count-1].kn = *int_factor;
            world->cylinder_array[cylinders_count-1].o1 = *temp_factor5;
            world->cylinder_array[cylinders_count-1].o2 = *temp_factor6;
            world->cylinder_array[cylinders_count-1].o3 = *temp_factor7;            
            read_factors = false;
            read_color = true;
            continue; // Move to next line
        }
        if (read_color && type == 'C')
        {
            sscanf(line, "%d,%d,%d", r, g, b);
            world->cylinder_array[cylinders_count-1].r = *r;
            world->cylinder_array[cylinders_count-1].g = *g;
            world->cylinder_array[cylinders_count-1].b = *b;
            read_color = false;
            continue; // Move to next line
        }
         // Cones
        if (read_cone)
        {
            cones_count++;
            sscanf(line, "%lf,%lf,%lf,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, temp_factor4, temp_factor5, temp_factor6);
            world->cone_array = (Cone *)realloc(world->cone_array, sizeof(Cone) * (cones_count));
            world->cone_array[cones_count-1].d1.x = *temp_factor;
            world->cone_array[cones_count-1].d1.y = *temp_factor2;
            world->cone_array[cones_count-1].d1.z = *temp_factor3;
            world->cone_array[cones_count-1].d2.x = *temp_factor4;
            world->cone_array[cones_count-1].d2.y = *temp_factor5;
            world->cone_array[cones_count-1].d2.z = *temp_factor6;
            // Calculate Q vector (axis direction) as normalized vector from d1 to d2
            Vector d1_to_d2 = vector_subtract(world->cone_array[cones_count-1].d2, 
            world->cone_array[cones_count-1].d1);
            world->cone_array[cones_count-1].Q = vector_normalize(d1_to_d2);
            read_cone = false;
            read_factors = true;
            continue; // Move to next line
        }
        if (read_factors && type == 'K')
        {
            sscanf(line, "%lf,%lf,%lf,%lf,%d,%lf,%lf,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, temp_factor4, int_factor, temp_factor5, temp_factor6, temp_factor7, temp_factor8, temp_factor9);
            world->cone_array[cones_count-1].radius = *temp_factor;
            world->cone_array[cones_count-1].kd = *temp_factor2;
            world->cone_array[cones_count-1].ka = *temp_factor3;
            world->cone_array[cones_count-1].ks = *temp_factor4;
            world->cone_array[cones_count-1].kn = *int_factor;
            world->cone_array[cones_count-1].k1 = *temp_factor5;
            world->cone_array[cones_count-1].k2 = *temp_factor6;
            world->cone_array[cones_count-1].o1 = *temp_factor7;
            world->cone_array[cones_count-1].o2 = *temp_factor8;
            world->cone_array[cones_count-1].o3 = *temp_factor9;
            read_factors = false;
            read_color = true;
            continue; // Move to next line
        }
        if (read_color && type == 'K')
        {
            sscanf(line, "%d,%d,%d", r, g, b);
            world->cone_array[cones_count-1].r = *r;
            world->cone_array[cones_count-1].g = *g;
            world->cone_array[cones_count-1].b = *b;
            read_color = false;
            continue; // Move to next line
        }
        // Polygons
        if (read_polygon)
        {
            polygons_count++;
            world->polygon_array = (Polygon *)realloc(world->polygon_array, sizeof(Polygon) * (polygons_count));
            int vector_count = 1;
            for (const char *p = line; *p; p++)
            {
                if (*p == ' ')
                {
                    vector_count++;
                }
            }
            Polygon* polygon = &world->polygon_array[polygons_count - 1];
            polygon -> vector_count = vector_count;
            polygon -> texture_path = NULL;
            polygon -> vertor_array = (Vector *)malloc(vector_count * sizeof(Vector));
            // Parse the line and fill the vertor_array
            const char *p = line;
            for (int i = 0; i < vector_count; i++)
            {
                sscanf(p, "%lf,%lf,%lf", &polygon->vertor_array[i].x, &polygon->vertor_array[i].y, &polygon->vertor_array[i].z);
                while (*p && *p != ' ')
                    p++;
                if (*p)
                    p++;
            }
            get_polygon_plane_equation(polygon);
            read_polygon = false;
            read_factors = true;
            continue; // Move to next line
        }
        if (read_texture && type == 'P')
        {
            // Read texture path
            if (world->polygon_array[polygons_count-1].texture_path == NULL)
            {
                world->polygon_array[polygons_count-1].texture_path = (char *)realloc(world->polygon_array[polygons_count-1].texture_path, (strlen(line) + 1) * sizeof(char));
                sscanf(line, "%s", world->polygon_array[polygons_count-1].texture_path);
                continue; // Move to next line
            } else {
                read_texture = false;
                if (strcmp(world->polygon_array[polygons_count-1].texture_path, "checkboard") == 0) {
                    sscanf(line, "%lf", temp_factor);
                    world->polygon_array[polygons_count-1].texture_scale = *temp_factor;
                    continue; // Move to next line
                }                
            }         
        }
        if (read_factors && type == 'P')
        {
            sscanf(line, "%lf,%lf,%lf,%d,%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3, int_factor, temp_factor4, temp_factor5, temp_factor6);
            world->polygon_array[polygons_count-1].kd = *temp_factor;
            world->polygon_array[polygons_count-1].ka = *temp_factor2;
            world->polygon_array[polygons_count-1].ks = *temp_factor3;
            world->polygon_array[polygons_count-1].kn = *int_factor;
            world->polygon_array[polygons_count-1].o1 = *temp_factor4;
            world->polygon_array[polygons_count-1].o2 = *temp_factor5;
            world->polygon_array[polygons_count-1].o3 = *temp_factor6;
            read_factors = false;
            read_color = true;
            continue; // Move to next line
        }
        if (read_color && type == 'P')
        {
            sscanf(line, "%d,%d,%d", r, g, b);
            world->polygon_array[polygons_count-1].r = *r;
            world->polygon_array[polygons_count-1].g = *g;
            world->polygon_array[polygons_count-1].b = *b;
            read_color = false;
            continue; // Move to next line
        }
        // Light sources
        if (read_light)
        {
            lights_count++;
            sscanf(line, "%lf,%lf,%lf", temp_factor, temp_factor2, temp_factor3);
            world->light_source_array = (LightSource *)realloc(world->light_source_array, sizeof(LightSource) * (lights_count));
            world->light_source_array[lights_count-1].point.x = *temp_factor;
            world->light_source_array[lights_count-1].point.y = *temp_factor2;
            world->light_source_array[lights_count-1].point.z = *temp_factor3;
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
    world->polygons_count = polygons_count;
    world->cylinders_count = cylinders_count;
    world->discs_count = discs_count;
    world->lights_count = lights_count;
    world->cones_count = cones_count;
    free(line);
    free(r);
    free(g);
    free(b);
    free(temp_factor);
    free(temp_factor2);
    free(temp_factor3);
    free(temp_factor4);
    free(temp_factor5);
    free(temp_factor6);
    free(temp_factor7);
    free(temp_factor8);
    free(temp_factor9);
    free(int_factor);
    fclose(file);
    
    return world;
}

unsigned int ray_hash(Ray ray) {
    // Simple but effective hash function for rays
    unsigned int hash = 0;
    
    // Hash the origin components
    unsigned int* px = (unsigned int*)&ray.origin.x;
    unsigned int* py = (unsigned int*)&ray.origin.y;
    unsigned int* pz = (unsigned int*)&ray.origin.z;
    
    // Hash the direction components
    unsigned int* dx = (unsigned int*)&ray.direction.x;
    unsigned int* dy = (unsigned int*)&ray.direction.y;
    unsigned int* dz = (unsigned int*)&ray.direction.z;
    
    // Combine all components into a single hash
    hash = (hash * 397) ^ *px;
    hash = (hash * 397) ^ *py;
    hash = (hash * 397) ^ *pz;
    hash = (hash * 397) ^ *dx;
    hash = (hash * 397) ^ *dy;
    hash = (hash * 397) ^ *dz;
    
    return hash & CACHE_MASK; // Keep it within cache bounds
}

bool rays_equal(Ray a, Ray b) {  
    // Check if origin points are close enough
    if (fabs(a.origin.x - b.origin.x) > EPSILON) return false;
    if (fabs(a.origin.y - b.origin.y) > EPSILON) return false;
    if (fabs(a.origin.z - b.origin.z) > EPSILON) return false;
    
    // Check if direction vectors are close enough
    if (fabs(a.direction.x - b.direction.x) > EPSILON) return false;
    if (fabs(a.direction.y - b.direction.y) > EPSILON) return false;
    if (fabs(a.direction.z - b.direction.z) > EPSILON) return false;
    
    return true;
}

Intersection* deep_copy_intersection(Intersection* original) {
    if (original == NULL) return NULL;
    
    Intersection* copy = (Intersection*)malloc(sizeof(Intersection));
    *copy = *original; // Copy the basic structure
    
    // Don't need to deep copy the pointers to objects
    // since they point to the universal world arrays
    
    return copy;
}
