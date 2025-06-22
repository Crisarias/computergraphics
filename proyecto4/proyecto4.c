#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "utils.h"
#include "universal_world.h"

Pixel color;
Pixel **frameBuffer;
SDL_Window *window = NULL;
Universal_world *universal_world = NULL;
SDL_Renderer *renderer = NULL;
int numTextures = 0;
Texture **texturesCache;

int RESOLUTION_WIDTH = 1920;
int RESOLUTION_HEIGHT = 1080;

int recursion_depth_max = 6;

IntersectionCacheEntry intersectionCache[CACHE_SIZE];

void setColor(int r, int g, int b)
{
    color.r = r;
    color.g = g;
    color.b = b;
}

void plot_pixel(int x, int y)
{
    frameBuffer[x][y].r = color.r;
    frameBuffer[x][y].g = color.g;
    frameBuffer[x][y].b = color.b;
}

void save_to_image()
{
    FILE *file = fopen("scene.ppm", "w");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Cannot open file for writing.\n");
        exit(1);
    }
    fprintf(file, "P3\n%d %d\n255\n", RESOLUTION_WIDTH, RESOLUTION_HEIGHT);
    for (int j = 0; j < RESOLUTION_HEIGHT; j++)
    {
        for (int i = 0; i < RESOLUTION_WIDTH; i++)
        {
            fprintf(file, "%d %d %d ", frameBuffer[i][j].r, frameBuffer[i][j].g, frameBuffer[i][j].b);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

void initWindow()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(
        "Proyecto 4 - Cristian Arias - Rendering in progress",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        RESOLUTION_WIDTH,
        RESOLUTION_HEIGHT,
        SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
    setColor(255, 255, 255);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void render_frame_buffer()
{
    for (int i = 0; i < RESOLUTION_WIDTH; i++)
    {
        for (int j = 0; j < RESOLUTION_HEIGHT; j++)
        {
            SDL_SetRenderDrawColor(renderer, frameBuffer[i][j].r, frameBuffer[i][j].g, frameBuffer[i][j].b, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(renderer, i, j);
        }
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
}

void render_frame_buffer_column(int i)
{
    for (int j = 0; j < RESOLUTION_HEIGHT; j++)
    {
        SDL_SetRenderDrawColor(renderer, frameBuffer[i][j].r, frameBuffer[i][j].g, frameBuffer[i][j].b, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawPoint(renderer, i, j);
    }
    SDL_RenderPresent(renderer);
}

void render_point(int i, int j)
{
    SDL_SetRenderDrawColor(renderer, frameBuffer[i][j].r, frameBuffer[i][j].g, frameBuffer[i][j].b, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoint(renderer, i, j);
    SDL_RenderPresent(renderer);
}

void clearFrameBuffer()
{
    for (int i = 0; i < RESOLUTION_WIDTH; i++)
    {
        for (int j = 0; j < RESOLUTION_HEIGHT; j++)
        {
            frameBuffer[i][j].r = 255;
            frameBuffer[i][j].g = 255;
            frameBuffer[i][j].b = 255;
        }
    }
}

Texture *getTextureFromCache(char *texture_filepath)
{
    for (int i = 0; i < numTextures; i++)
    {
        if (strcmp(texturesCache[i]->filepath, texture_filepath) == 0)
        {
            return texturesCache[i];
        }
    }
    return NULL;
}

void init_intersection_cache() {
    for (int i = 0; i < CACHE_SIZE; i++) {
        intersectionCache[i].valid = false;
    }
}

bool is_point_inside_polygon(Vector point, Polygon* polygon) {    
    // Count ray intersections with polygon edges
    int intersections = 0;
    
    for (int i = 0; i < polygon->vector_count; i++) {
        int j = (i + 1) % polygon->vector_count;
        
        // Step 2: Convert to (u,v) 2D coordinates by discarding appropriate coordinate
        double u1, v1, u2, v2, u_point, v_point;
        
        if (polygon->discard_coord == 'x') {
            u1 = polygon->vertor_array[i].y;
            v1 = polygon->vertor_array[i].z;
            u2 = polygon->vertor_array[j].y;
            v2 = polygon->vertor_array[j].z;
            u_point = point.y;
            v_point = point.z;
        } else if (polygon->discard_coord == 'y') {
            u1 = polygon->vertor_array[i].x;
            v1 = polygon->vertor_array[i].z;
            u2 = polygon->vertor_array[j].x;
            v2 = polygon->vertor_array[j].z;
            u_point = point.x;
            v_point = point.z;
        } else { // Discard z
            u1 = polygon->vertor_array[i].x;
            v1 = polygon->vertor_array[i].y;
            u2 = polygon->vertor_array[j].x;
            v2 = polygon->vertor_array[j].y;
            u_point = point.x;
            v_point = point.y;
        }
        
        // Step 3: Translate to make the test point the origin
        u1 -= u_point;
        v1 -= v_point;
        u2 -= u_point;
        v2 -= v_point;
        
        // Step 4: Apply the crossing test logic
        
        // Trivial reject: both endpoints on the same side of the u axis
        if ((v1 < 0 && v2 < 0) || (v1 > 0 && v2 > 0))
            continue; // No intersection
            
        // Trivial reject: both endpoints are behind origin on u axis
        if (u1 < 0 && u2 < 0)
            continue; // No intersection
            
        // Trivial accept: both endpoints are in front of origin on u axis (with edge crossing v axis)
        if (u1 > 0 && u2 > 0 && ((v1 < 0 && v2 > 0) || (v1 > 0 && v2 < 0))) {
            intersections++;
            continue;
        }
        
        // Complicated case: need to calculate intersection with u axis
        if ((v1 < 0 && v2 > 0) || (v1 > 0 && v2 < 0)) {
            // Calculate intersection with v=0 (u axis)
            double u_intersect = u1 + (u2 - u1) * (-v1) / (v2 - v1);
            
            // If intersection point is positive u, count the intersection
            if (u_intersect > 0)
                intersections++;
        }
    }
    
    // If the number of intersections is odd, the point is inside
    return (intersections % 2) != 0;
}

Intersection *get_intersection_cylinder_ray(Ray ray, Cylinder *cylinder)
{
    Intersection *intersection = (Intersection *)malloc(sizeof(Intersection));
    
    // Extract cylinder parameters
    Vector Q = cylinder->Q;      // Cylinder axis direction (already normalized)
    Vector base = cylinder->d1;  // Cylinder base point (x0,y0,z0)
    double radius = cylinder->radius;
    
    // Ray parameters
    Vector origin = ray.origin;  // Ray origin (Xe,Ye,Ze)
    Vector direction = ray.direction; // Ray direction (XD,YD,ZD)
    
    // Calculate required dot products for quadratic equation

    // Vector from ray origin to cylinder base
    Vector delta = vector_subtract(origin, base);
    
    double Q_dot_D = vector_dot(direction, Q); // Dot product of ray direction with axis    

    double Dx_dot_Q = vector_dot(delta, Q); // Dot product of delta with axis

    double Dx_dot_D = vector_dot(delta, direction); // Dot product of delta with direction    
    
    // Quadratic equation coefficients based on the mathematical derivation

    // a = |D|² - (D·Q)²
    double a = vector_dot(direction, direction) - pow(Q_dot_D, 2);
    
    // b = 2(D·delta - (D·Q)(delta·Q))
    double b = 2.0 * (Dx_dot_D- Q_dot_D * Dx_dot_Q);
    
    // c = |delta|² - (delta·Q)² - radius²
    double c = vector_dot(delta, delta) - pow(Dx_dot_Q, 2) - pow(radius, 2);
    
    // Solve the quadratic equation
    double discriminant = b*b - 4*a*c;
    if (discriminant < 0) {
        free(intersection);
        return NULL; // No intersection
    }
    
    // Calculate both intersection points
    double t1 = (-b - sqrt(discriminant)) / (2*a);
    double t2 = (-b + sqrt(discriminant)) / (2*a);
    
    // Sort intersections by distance
    if (t1 > t2) {
        double temp = t1;
        t1 = t2;
        t2 = temp;
    }
    
    // Check if both intersections are behind the ray
    if (t2 < EPSILON) {
        free(intersection);
        return NULL;
    }
    
    // Choose the closest valid intersection
    double t_body = (t1 > EPSILON) ? t1 : t2;
    
    // Calculate cylinder height
    double height = vector_dot(vector_subtract(cylinder->d2, base), Q);
    
    // Calculate intersection point
    Vector hit_point = vector_add(origin, vector_multiply(direction, t_body));
    
    // Check if the intersection is within the cylinder's height
    Vector hit_to_base = vector_subtract(hit_point, base);
    double hit_height = vector_dot(hit_to_base, Q);
    
    // If the closest intersection is outside height bounds, try the other one
    if (hit_height < 0 || hit_height > height) {
        if (t2 < EPSILON || t1 > t2) {  // No valid second intersection
            free(intersection);
            return NULL;
        }
        
        t_body = t2;  // Try the farther intersection
        hit_point = vector_add(origin, vector_multiply(direction, t_body));
        hit_to_base = vector_subtract(hit_point, base);
        hit_height = vector_dot(hit_to_base, Q);
        
        // If both intersections are outside the cylinder height, no valid intersection
        if (hit_height < 0 || hit_height > height) {
            free(intersection);
            return NULL;
        }
    }
    
    // Valid intersection found
    intersection->t = t_body;
    intersection->intersection_point = hit_point;
    intersection->cylinder = cylinder;
    intersection->sphere = NULL;
    intersection->polygon = NULL;
    
    return intersection;
}

Intersection *get_intersection_cone_ray(Ray ray, Cone *cone)
{
    Intersection *intersection = (Intersection *)malloc(sizeof(Intersection));
    
    // Extract cone parameters
    Vector Q = cone->Q;      // Cone axis direction (already normalized)
    Vector base = cone->d1;  // Cone base point (x0,y0,z0)
    double radius = cone->radius;  // Base radius of cone
    double k1 = cone->k1;
    double k2 = cone->k2;
    
    // Ray parameters
    Vector origin = ray.origin;  // Ray origin (Xe,Ye,Ze)
    Vector direction = ray.direction; // Ray direction (XD,YD,ZD)
    
    // Calculate required dot products for quadratic equation
    
    // Vector from ray origin to cone base
    Vector delta = vector_subtract(origin, base);
    
    double Q_dot_D = vector_dot(direction, Q); // Dot product of ray direction with axis    
    double Dx_dot_Q = vector_dot(delta, Q);     // Dot product of delta with axis
    double Dx_dot_D = vector_dot(delta, direction); // Dot product of delta with direction
    
    // The squared ratio k2/k1 for cone shape
    double k_ratio_squared = pow(k2/k1, 2);
    
    // Quadratic equation coefficients for cone
    // a = |D|² - (1 + k_ratio_squared)(D·Q)²
    double a = vector_dot(direction, direction) - (1 + k_ratio_squared) * pow(Q_dot_D, 2);
    
    // b = 2[D·delta - (1 + k_ratio_squared)(D·Q)(delta·Q)]
    double b = 2.0 * (Dx_dot_D - (1 + k_ratio_squared) * Q_dot_D * Dx_dot_Q);
    
    // c = |delta|² - (1 + k_ratio_squared)(delta·Q)² - radius²
    double c = vector_dot(delta, delta) - (1 + k_ratio_squared) * pow(Dx_dot_Q, 2) - pow(radius, 2);
    
    // Solve the quadratic equation
    double discriminant = b*b - 4*a*c;
    if (discriminant < 0) {
        free(intersection);
        return NULL; // No intersection
    }
    
    // Calculate both intersection points
    double t1 = (-b - sqrt(discriminant)) / (2*a);
    double t2 = (-b + sqrt(discriminant)) / (2*a);
    
    // Sort intersections by distance
    if (t1 > t2) {
        double temp = t1;
        t1 = t2;
        t2 = temp;
    }
    
    // Check if both intersections are behind the ray
    if (t2 < EPSILON) {
        free(intersection);
        return NULL;
    }
    
    // Choose the closest valid intersection
    double t_body = (t1 > EPSILON) ? t1 : t2;
    
    // Calculate cone height
    double height = vector_dot(vector_subtract(cone->d2, base), Q);
    
    // Calculate intersection point
    Vector hit_point = vector_add(origin, vector_multiply(direction, t_body));
    
    // Check if the intersection is within the cone's height
    Vector hit_to_base = vector_subtract(hit_point, base);
    double hit_height = vector_dot(hit_to_base, Q);
    
    // If the closest intersection is outside height bounds, try the other one
    if (hit_height < 0 || hit_height > height) {
        if (t2 < EPSILON || t1 > t2) {  // No valid second intersection
            free(intersection);
            return NULL;
        }
        
        t_body = t2;  // Try the farther intersection
        hit_point = vector_add(origin, vector_multiply(direction, t_body));
        hit_to_base = vector_subtract(hit_point, base);
        hit_height = vector_dot(hit_to_base, Q);
        
        // If both intersections are outside the cone height, no valid intersection
        if (hit_height < 0 || hit_height > height) {
            free(intersection);
            return NULL;
        }
    }
    
    // Valid intersection found
    intersection->t = t_body;
    intersection->intersection_point = hit_point;
    intersection->cone = cone;
    
    return intersection;
}

Intersection *get_intersection_sphere_ray(Ray ray, Sphere *sphere)
{
    Intersection *intersection = (Intersection *)malloc(sizeof(Intersection));
    Vector origin_center = vector_subtract(ray.origin, sphere->center);
    double alpha = vector_dot(ray.direction, ray.direction);
    double beta = 2.0 * vector_dot(origin_center, ray.direction);
    double gamma = vector_dot(origin_center, origin_center) - pow(sphere->radius, 2);
    double discriminant = pow(beta, 2) - 4 * alpha * gamma;
    if (discriminant < 0)
    {
        free(intersection);
        return NULL;
    }
    double t1 = (-beta - sqrt(discriminant)) / (2.0 * alpha);
    double t2 = (-beta + sqrt(discriminant)) / (2.0 * alpha);
    if (t1 > EPSILON && t1 < t2)
    {
        intersection->t = t1;
    }
    else if (t2 > EPSILON && t2 < t1)
    {
        intersection->t = t2;
    }
    else
    {
        free(intersection);
        return NULL;
    }
    intersection->intersection_point.x = ray.origin.x + ray.direction.x * intersection->t;
    intersection->intersection_point.y = ray.origin.y + ray.direction.y * intersection->t;
    intersection->intersection_point.z = ray.origin.z + ray.direction.z * intersection->t;
    intersection->sphere = sphere;
    // Check if the intersection point is above or below the cut plane
    if (sphere->has_cut_plane)
    {
        double plane_equation_value = sphere->A * intersection->intersection_point.x +
                                    sphere->B * intersection->intersection_point.y +
                                    sphere->C * intersection->intersection_point.z + sphere->D;
        if (plane_equation_value > EPSILON && !sphere->cutTop)
        {
            // Intersection point is above the cut plane
            free(intersection);
            return NULL;
        }
        
        if(plane_equation_value < -EPSILON && sphere->cutTop)
        {
            // Intersection point is below the cut plane
            free(intersection);
            return NULL;
        }
    }
    return intersection;
}

Intersection *get_intersection_polygon_ray(Ray ray, Polygon *polygon)
{
    Intersection *intersection = (Intersection *)malloc(sizeof(Intersection));
    double D = polygon->D;
    double denominator = vector_dot(polygon->normal, ray.direction);
    if (fabs(denominator) < EPSILON)
    {        
        free(intersection);
        return NULL; // Ray is parallel to the plane
    }
    intersection->t = -(vector_dot(polygon->normal, ray.origin) + D) / denominator;
    if (intersection->t < EPSILON)
    {
        free(intersection);
        return NULL; // Intersection is behind the ray origin
    }
    intersection->intersection_point.x = ray.origin.x + ray.direction.x * intersection->t;
    intersection->intersection_point.y = ray.origin.y + ray.direction.y * intersection->t;
    intersection->intersection_point.z = ray.origin.z + ray.direction.z * intersection->t;

    // Check if point is inside the polygon
    if (!is_point_inside_polygon(intersection->intersection_point, polygon)) {
        free(intersection);
        return NULL;
    }
    intersection->polygon = polygon;
    return intersection;
}

Intersection *get_intersection_disc_ray(Ray ray, Disc *disc)
{
    Intersection *intersection = (Intersection *)malloc(sizeof(Intersection));    
    Vector N = vector_init(disc->A, disc->B, disc->C);
    double D = disc->D;
    double denominator = vector_dot(N, ray.direction);
    if (fabs(denominator) < EPSILON)
    {        
        free(intersection);
        return NULL; // Ray is parallel to the plane
    }
    intersection->t = -(vector_dot(N, ray.origin) + D) / denominator;
    if (intersection->t < EPSILON)
    {
        free(intersection);
        return NULL; // Intersection is behind the ray origin
    }
    intersection->intersection_point.x = ray.origin.x + ray.direction.x * intersection->t;
    intersection->intersection_point.y = ray.origin.y + ray.direction.y * intersection->t;
    intersection->intersection_point.z = ray.origin.z + ray.direction.z * intersection->t;

    // Check if point is inside the disc
    Vector center_to_hit = vector_subtract(intersection->intersection_point, disc->center);
    
    // Calculate the distance squared (avoiding square root for efficiency)
    double distance_squared = vector_dot(center_to_hit, center_to_hit);
    
    // Check if the intersection point is within the disc's radius
    if (distance_squared > disc->radius * disc->radius) {
        // Intersection point is outside the disc
        free(intersection);
        return NULL;
    }    
    intersection->disc = disc;
    return intersection;
}

Intersection *get_first_intersection(Ray ray)
{
    Intersection *intersection = NULL;
    double tmin = 999999999999999;
    // Check intersections with spheres
    for (int i = 0; i < universal_world->spheres_count; i++)
    {
        Intersection *intersection_object_ray = get_intersection_sphere_ray(ray, &universal_world->sphere_array[i]);
        if (intersection_object_ray != NULL && intersection_object_ray->t < tmin)
        {
            if (intersection != NULL) {
                free(intersection);  // Free previous intersection
            }
            tmin = intersection_object_ray->t;
            intersection = intersection_object_ray;
            intersection->polygon = NULL;
            intersection->cylinder = NULL;
            intersection->disc = NULL;
            intersection->cone = NULL;
        }
        else
        {
            free(intersection_object_ray);
        }
    }
    // Check intersections with polygons
    for (int i = 0; i < universal_world->polygons_count; i++)
    {
        Intersection *intersection_object_ray = get_intersection_polygon_ray(ray, &universal_world->polygon_array[i]);
        if (intersection_object_ray != NULL && intersection_object_ray->t < tmin)
        {
            if (intersection != NULL) {
                free(intersection);  // Free previous intersection
            }
            tmin = intersection_object_ray->t;
            intersection = intersection_object_ray;
            intersection->sphere = NULL;
            intersection->cylinder = NULL;
            intersection->disc = NULL;
            intersection->cone = NULL;
        }
        else
        {
            free(intersection_object_ray);
        }
    }
    // Check intersections with cylinders
    for (int i = 0; i < universal_world->cylinders_count; i++)
    {
        Intersection *intersection_object_ray = get_intersection_cylinder_ray(ray, &universal_world->cylinder_array[i]);
        if (intersection_object_ray != NULL && intersection_object_ray->t < tmin)
        {
            if (intersection != NULL) {
                free(intersection);  // Free previous intersection
            }
            tmin = intersection_object_ray->t;
            intersection = intersection_object_ray;
            intersection->polygon = NULL;
            intersection->sphere = NULL;
            intersection->disc = NULL;
            intersection->cone = NULL;
        }
        else
        {
            free(intersection_object_ray);
        }
    }
    // Check intersections with discs
    for (int i = 0; i < universal_world->discs_count; i++)
    {
        Intersection *intersection_object_ray = get_intersection_disc_ray(ray, &universal_world->disc_array[i]);
        if (intersection_object_ray != NULL && intersection_object_ray->t < tmin)
        {
            if (intersection != NULL) {
                free(intersection);  // Free previous intersection
            }
            tmin = intersection_object_ray->t;
            intersection = intersection_object_ray;       
            intersection->polygon = NULL;
            intersection->sphere = NULL;
            intersection->cylinder = NULL;
            intersection->cone = NULL;
        }
        else
        {
            free(intersection_object_ray);
        }
    }
    // Check intersections with cones
    for (int i = 0; i < universal_world->cones_count; i++)
    {
        Intersection *intersection_object_ray = get_intersection_cone_ray(ray, &universal_world->cone_array[i]);
        if (intersection_object_ray != NULL && intersection_object_ray->t < tmin)
        {
            if (intersection != NULL) {
                free(intersection);  // Free previous intersection
            }
            tmin = intersection_object_ray->t;
            intersection = intersection_object_ray;       
            intersection->polygon = NULL;
            intersection->sphere = NULL;
            intersection->cylinder = NULL;
            intersection->disc = NULL;
        }
        else
        {
            free(intersection_object_ray);
        }
    }

    return intersection;
}

void get_poligon_texture(Polygon *polygon, Pixel *pixel,Vector *intersection_point)
{
    pixel->r = polygon->r;
    pixel->g = polygon->g;
    pixel->b = polygon->b;

    if (polygon->texture_path == NULL) {
        return;
    }
        // Apply texture if available           
    if(strcmp(polygon->texture_path, "checkboard") == 0) {
        int coord1,coord2;
        double scale = polygon->texture_scale;
        
        // Discard coordinate with largest normal component (project onto most stable plane)
        if (polygon -> discard_coord == 'x') {
            coord1 = (int)(intersection_point->y * scale) % 2;
            coord2 = (int)(intersection_point->z * scale) % 2;
        } 
        else if (polygon -> discard_coord == 'y') 
        {
            coord1 = (int)(intersection_point->x * scale) % 2;
            coord2 = (int)(intersection_point->z * scale) % 2;
        }
        else
        {
            coord1 = (int)(intersection_point->y * scale) % 2;
            coord2 = (int)(intersection_point->z * scale) % 2;
        }            
        // Apply checkboard pattern        
        if ((coord1 + coord2) % 2 == 0) {
            pixel->r = 255;
            pixel->g = 255;
            pixel->b = 255;
        }
    } else {
        // Load and apply texture from ppm file
        Texture *texture = getTextureFromCache(polygon->texture_path);
        if (texture == NULL) {
            return;
        }

        int yt, xt;

        if (polygon -> discard_coord == 'x') {
             yt = (int)intersection_point->y % texture->resHeight;
             xt = (int)intersection_point->z % texture->resWidth;
        } 
        else if (polygon -> discard_coord == 'y') 
        {
            yt = (int)intersection_point->z % texture->resHeight;
            xt = (int)intersection_point->x % texture->resWidth;
        }
        else
        {
            yt = (int)intersection_point->y % texture->resHeight;
            xt = (int)intersection_point->x % texture->resWidth;
        }

        pixel->r = texture->texel_array[yt][xt].r;
        pixel->g = texture->texel_array[yt][xt].g;
        pixel->b = texture->texel_array[yt][xt].b;
    }
}

Pixel get_pixel_color(Ray ray, int recursion_depth)
{
    unsigned int index = ray_hash(ray);

    if (intersectionCache[index].valid && rays_equal(intersectionCache[index].ray, ray)) {
        // Cache hit! Return a copy of the cached pixel
        return intersectionCache[index].pixel;
    }

    Pixel pixel;    
    Intersection *intersection = get_first_intersection(ray);
    if (intersection != NULL)
    {
        Vector N;
        double kd, ks, ka, o1, o2, o3;
        int kn;
        if (intersection->sphere != NULL)
        {            
            // Sphere intersection
            pixel.r = intersection->sphere->r;
            pixel.g = intersection->sphere->g;
            pixel.b = intersection->sphere->b;
            Vector center_intersection = vector_subtract(intersection->intersection_point, intersection->sphere->center);
            N = vector_normalize(center_intersection);
            kd = intersection->sphere->kd;
            ks = intersection->sphere->ks;
            ka = intersection->sphere->ka;
            kn = intersection->sphere->kn;
            o1 = intersection->sphere->o1;
            o2 = intersection->sphere->o2;
            o3 = intersection->sphere->o3;
        }
        else if (intersection->polygon != NULL)
        {
            // Polygon intersection
            get_poligon_texture(intersection->polygon, &pixel, &intersection->intersection_point);
            N = vector_init(intersection->polygon->A, intersection->polygon->B, intersection->polygon->C);
            // Check if normal points toward the viewer and flip if needed
            Vector view_dir = vector_multiply(ray.direction, -1);
            if (vector_dot(N, view_dir) < 0) {
                N = vector_multiply(N, -1);
            }
            kd = intersection->polygon->kd;
            ks = intersection->polygon->ks;
            ka = intersection->polygon->ka;
            kn = intersection->polygon->kn;
            o1 = intersection->polygon->o1;
            o2 = intersection->polygon->o2;
            o3 = intersection->polygon->o3;
        } 
        else if (intersection->cylinder != NULL) {        
            // Cylinder intersection
            pixel.r = intersection->cylinder->r;
            pixel.g = intersection->cylinder->g;
            pixel.b = intersection->cylinder->b;
            
            // Calculate normal - for cylinder body only
            double d = vector_dot(vector_subtract(intersection->intersection_point, intersection->cylinder->d1), intersection->cylinder->Q);
            Vector along_axis = vector_multiply(intersection->cylinder->Q, d);
            Vector center_at_height = vector_add(intersection->cylinder->d1, along_axis);
            Vector to_hit = vector_subtract(intersection->intersection_point, center_at_height);
            
            // Use radial vector as normal - always points outward
            N = vector_normalize(to_hit);
            
            // Check if we're inside the cylinder and need to flip the normal
            if (vector_dot(N, vector_multiply(ray.direction, -1)) < 0) {
                N = vector_multiply(N, -1);  // Flip normal if viewing from inside
            }
            
            kd = intersection->cylinder->kd;
            ks = intersection->cylinder->ks;
            ka = intersection->cylinder->ka;
            kn = intersection->cylinder->kn;
            o1 = intersection->cylinder->o1;
            o2 = intersection->cylinder->o2;
            o3 = intersection->cylinder->o3;
        } 
        else if (intersection->disc != NULL)
        {
            // Disc intersection
            pixel.r = intersection->disc->r;
            pixel.g = intersection->disc->g;
            pixel.b = intersection->disc->b;
            N = vector_init(intersection->disc->A, intersection->disc->B, intersection->disc->C);
            // Check if normal points toward the viewer and flip if needed
            Vector view_dir = vector_multiply(ray.direction, -1);
            if (vector_dot(N, view_dir) < 0) {
                N = vector_multiply(N, -1);
            }
            kd = intersection->disc->kd;
            ks = intersection->disc->ks;
            ka = intersection->disc->ka;
            kn = intersection->disc->kn;
            o1 = intersection->disc->o1;
            o2 = intersection->disc->o2;
            o3 = intersection->disc->o3;
        }
        else if (intersection->cone != NULL) {
            // Cone intersection
            pixel.r = intersection->cone->r;
            pixel.g = intersection->cone->g;
            pixel.b = intersection->cone->b;
            
            // Calculate normal - for cone surface
            double d = vector_dot(vector_subtract(intersection->intersection_point, intersection->cone->d1), 
                                intersection->cone->Q);
            Vector along_axis = vector_multiply(intersection->cone->Q, d);
            Vector center_at_height = vector_add(intersection->cone->d1, along_axis);
            Vector to_hit = vector_subtract(intersection->intersection_point, center_at_height);
            
            // Adjust normal for cone shape
            double height = vector_dot(vector_subtract(intersection->cone->d2, 
                                                    intersection->cone->d1), 
                                    intersection->cone->Q);
            double k_ratio = intersection->cone->k2 / intersection->cone->k1;
            Vector axis_component = vector_multiply(intersection->cone->Q, k_ratio);
            N = vector_normalize(vector_add(to_hit, axis_component));
            
            // Check if normal points toward the viewer
            if (vector_dot(N, vector_multiply(ray.direction, -1)) < 0) {
                N = vector_multiply(N, -1);  // Flip normal if viewing from inside
            }
            
            kd = intersection->cone->kd;
            ks = intersection->cone->ks;
            ka = intersection->cone->ka;
            kn = intersection->cone->kn;
            o1 = intersection->cone->o1;
            o2 = intersection->cone->o2;
            o3 = intersection->cone->o3;
        }
        else
        {
            // No valid intersection
            free(intersection);
            pixel.r = 192;
            pixel.g = 192;
            pixel.b = 192;
            return pixel; // Background color
        }
        // Calculate lighting
        double I = 0.0;
        double E = 0.0;
        for (int i = 0; i < universal_world->lights_count; i++)
        {
            LightSource light = universal_world->light_source_array[i];
            Vector light_intersection = vector_subtract(light.point, intersection->intersection_point);
            Vector L = vector_normalize(light_intersection);
            double distance_to_light = vector_length(light_intersection);
            double cos_theta = vector_dot(L, N);
            double fatt = 1.0 / (light.c1 + light.c2 * distance_to_light + light.c3 * distance_to_light * distance_to_light);
            if (cos_theta > 0)
            {
                Vector V = vector_multiply(ray.direction, -1);
                Vector R = vector_subtract(vector_multiply(N, 2 * vector_dot(N, L)), L);

                // Calculate shadows
                Ray shadow_ray;
                shadow_ray.origin = intersection->intersection_point;
                shadow_ray.direction = L;
                Intersection *shadow_intersection = get_first_intersection(shadow_ray);
                if (shadow_intersection == NULL)
                {
                    // No intersection - light isn't blocked
                    I = I + (cos_theta * kd * fatt * light.lp);
                    double cos_angle = vector_dot(R, V);
                    if (cos_angle > 0)
                    {
                        E = E + (pow(cos_angle, kn) * ks * fatt * light.lp);
                    }
                }
                else
                {
                    // Compare the distance to the intersection with the distance to the light
                    double distance_to_obstacle = shadow_intersection->t;

                    if (distance_to_obstacle > distance_to_light)
                    {
                        // Obstacle is beyond the light source, so light isn't blocked
                        I = I + (cos_theta * kd * fatt * light.lp);
                        double cos_angle = vector_dot(R, V);
                        if (cos_angle > 0)
                        {
                            E = E + (pow(cos_angle, kn) * ks * fatt * light.lp);
                        }
                    } // else light is blocked

                    // Free the shadow intersection memory
                    free(shadow_intersection);
                }
            }
        }
        I = I + (universal_world->ia * ka);
        I = fmin(1, I);
        E = fmin(1, E);
        pixel.r = (unsigned char)(pixel.r * I);
        pixel.g = (unsigned char)(pixel.g * I);
        pixel.b = (unsigned char)(pixel.b * I);
        pixel.r = (unsigned char)(pixel.r + E * (255 - pixel.r));
        pixel.g = (unsigned char)(pixel.g + E * (255 - pixel.g));
        pixel.b = (unsigned char)(pixel.b + E * (255 - pixel.b));
        if ((o2 == 0 && o3 == 0) || recursion_depth >= recursion_depth_max)
        {
            // No reflection or transparency
            pixel.r = (unsigned char)(pixel.r * o1);
            pixel.g = (unsigned char)(pixel.g * o1);
            pixel.b = (unsigned char)(pixel.b * o1);
        }
        else if (o2 != 0 && o3 == 0)
        {
            // Reflection only
            Ray reflection_ray;
            reflection_ray.origin = intersection->intersection_point;
            Vector reflection_direction = vector_subtract(ray.direction, vector_multiply(N, 2 * vector_dot(N, ray.direction)));
            reflection_ray.direction = vector_normalize(reflection_direction);
            Pixel reflection_color = get_pixel_color(reflection_ray, recursion_depth + 1);
            pixel.r = (unsigned char)(pixel.r * o1 + reflection_color.r * o2);
            pixel.g = (unsigned char)(pixel.g * o1 + reflection_color.g * o2);
            pixel.b = (unsigned char)(pixel.b * o1 + reflection_color.b * o2);
        }
        else if (o2 == 0 && o3 != 0)
        {
            // Transparency only
            Ray refraction_ray;
            refraction_ray.origin = intersection->intersection_point;
            refraction_ray.direction = ray.direction; // Use the original ray direction for refraction
            Pixel refraction_color = get_pixel_color(refraction_ray, recursion_depth + 1);
            pixel.r = (unsigned char)(pixel.r * o1 + refraction_color.r * o3);
            pixel.g = (unsigned char)(pixel.g * o1 + refraction_color.g * o3);
            pixel.b = (unsigned char)(pixel.b * o1 + refraction_color.b * o3);
        }
        else
        {
            // Reflection and transparency
            Ray reflection_ray;
            reflection_ray.origin = intersection->intersection_point;
            Vector reflection_direction = vector_subtract(ray.direction, vector_multiply(N, 2 * vector_dot(N, ray.direction)));
            reflection_ray.direction = vector_normalize(reflection_direction);
            Pixel reflection_color = get_pixel_color(reflection_ray, recursion_depth + 1);

            Ray refraction_ray;
            refraction_ray.origin = intersection->intersection_point;
            refraction_ray.direction = ray.direction; // Use the original ray direction for refraction
            Pixel refraction_color = get_pixel_color(refraction_ray, recursion_depth + 1);

            pixel.r = (unsigned char)(pixel.r * o1 + reflection_color.r * o2 + refraction_color.r * o3);
            pixel.g = (unsigned char)(pixel.g * o1 + reflection_color.g * o2 + refraction_color.g * o3);
            pixel.b = (unsigned char)(pixel.b * o1 + reflection_color.b * o2 + refraction_color.b * o3);
        }        
        free(intersection);
    }
    else
    {
        // No intersection with any sphere, set pixel to background
        pixel.r = 192;
        pixel.g = 192;
        pixel.b = 192;
    }

    // Store the result in the cache before returning
    // if (intersectionCache[index].valid && intersectionCache[index].intersection != NULL) {
    //     free(intersectionCache[index].intersection);
    // }
    
    intersectionCache[index].ray = ray;
    intersectionCache[index].pixel = pixel;
    intersectionCache[index].valid = true;

    return pixel;
}

void mapWorldToFrameBuffer()
{
    for (int i = 0; i < RESOLUTION_WIDTH; i++)
    {
        for (int j = 0; j < RESOLUTION_HEIGHT; j++)
        {
            // Upper Left

            Ray ray;
            ray.origin = universal_world->eye;

            double xw = (double)(i) * (universal_world->projection_maxX - universal_world->projection_minX) / RESOLUTION_WIDTH + universal_world->projection_minX;
            double yw = (double)(j) * (universal_world->projection_maxY - universal_world->projection_minY) / RESOLUTION_HEIGHT + universal_world->projection_minY;
            double zw = 0.00;

            Vector point_ul = vector_init(xw, yw, zw);
            Vector direction_ul = vector_subtract(point_ul, ray.origin);
            ray.direction = vector_normalize(direction_ul);
            Pixel color_ul = get_pixel_color(ray,1);

            // Upper Right

            xw = (double)(i + 1) * (universal_world->projection_maxX - universal_world->projection_minX) / RESOLUTION_WIDTH + universal_world->projection_minX;
            yw = (double)(j) * (universal_world->projection_maxY - universal_world->projection_minY) / RESOLUTION_HEIGHT + universal_world->projection_minY;

            Vector point_ur = vector_init(xw, yw, zw);
            Vector direction_ur = vector_subtract(point_ur, ray.origin);
            ray.direction = vector_normalize(direction_ur);
            Pixel color_ur = get_pixel_color(ray,1);

            // Center

            xw = (double)(i + 0.5) * (universal_world->projection_maxX - universal_world->projection_minX) / RESOLUTION_WIDTH + universal_world->projection_minX;
            yw = (double)(j + 0.5) * (universal_world->projection_maxY - universal_world->projection_minY) / RESOLUTION_HEIGHT + universal_world->projection_minY;

            Vector point_c = vector_init(xw, yw, zw);
            Vector direction_c = vector_subtract(point_c, ray.origin);
            ray.direction = vector_normalize(direction_c);
            Pixel color_c = get_pixel_color(ray,1);

            // Lower Left

            xw = (double)(i) * (universal_world->projection_maxX - universal_world->projection_minX) / RESOLUTION_WIDTH + universal_world->projection_minX;
            yw = (double)(j + 1) * (universal_world->projection_maxY - universal_world->projection_minY) / RESOLUTION_HEIGHT + universal_world->projection_minY;

            Vector point_ll = vector_init(xw, yw, zw);
            Vector direction_ll = vector_subtract(point_ll, ray.origin);
            ray.direction = vector_normalize(direction_ll);
            Pixel color_ll = get_pixel_color(ray,1);

            // Lower Right

            xw = (double)(i + 1) * (universal_world->projection_maxX - universal_world->projection_minX) / RESOLUTION_WIDTH + universal_world->projection_minX;
            yw = (double)(j + 1) * (universal_world->projection_maxY - universal_world->projection_minY) / RESOLUTION_HEIGHT + universal_world->projection_minY;

            Vector point_lr = vector_init(xw, yw, zw);
            Vector direction_lr = vector_subtract(point_lr, ray.origin);
            ray.direction = vector_normalize(direction_lr);
            Pixel color_lr = get_pixel_color(ray,1);

            // Average the colors
            Pixel average_color;
            average_color.r = (color_ul.r + color_ur.r + color_c.r + color_ll.r + color_lr.r) / 5;
            average_color.g = (color_ul.g + color_ur.g + color_c.g + color_ll.g + color_lr.g) / 5;
            average_color.b = (color_ul.b + color_ur.b + color_c.b + color_ll.b + color_lr.b) / 5;
            setColor(average_color.r, average_color.g, average_color.b);
            plot_pixel(i,j);
        }
        if (universal_world->output_to_screen)
        {
            render_frame_buffer_column(i);
        }
    }
}

void redraw_screen()
{
    mapWorldToFrameBuffer();
    render_frame_buffer();
    SDL_SetWindowTitle(window, "Proyecto 4 - Cristian Arias - Rendering complete");
}

void reset_universal_world()
{
    free(universal_world);
    universal_world = create_universal_world_from_file("world.txt");
}

int main(int argc, char *argv[])
{
    printf("Generating scene please wait... \n\n");

    printf("---------------------------------------------------\n\n");

    // Read Universal world from car.txt

    universal_world = create_universal_world_from_file("world.txt");

    // Read Textures

    for (int i = 0; i < universal_world->polygons_count; i++)
    {
        char *texture_filepath = universal_world->polygon_array[i].texture_path;
        if (texture_filepath != NULL && strcmp(texture_filepath, "checkboard") != 0)
        {
            Texture *texture = getTextureFromCache(texture_filepath);
            if (texture == NULL)
            {
                numTextures++;
                texturesCache = (Texture **)realloc(texturesCache, (numTextures) * sizeof(Texture *));
                if (texturesCache == NULL)
                {
                    fprintf(stderr, "\nError: Cannot reserve memory for texture cache.\n");
                    exit(1);
                }
                texturesCache[numTextures - 1] = create_texture_from_ppm(texture_filepath);
            }
        }
    }

    // Init Cache

    init_intersection_cache();

    printf("Universal world created\n");
    printf("Spheres count: %d\n", universal_world->spheres_count);
    printf("Polygons count: %d\n", universal_world->polygons_count);
    printf("Cylinders count: %d\n", universal_world->cylinders_count);
    printf("Discs count: %d\n", universal_world->discs_count);
    printf("Cones count: %d\n", universal_world->cones_count);
    printf("Lights count: %d\n", universal_world->lights_count);
    printf("Eye: (%lf, %lf, %lf)\n", universal_world->eye.x, universal_world->eye.y, universal_world->eye.z);
    printf("Projection window: (%lf, %lf, %lf, %lf)\n", universal_world->projection_minX, universal_world->projection_minY, universal_world->projection_maxX, universal_world->projection_maxY);
    printf("---------------------------------------------------\n\n");

    fflush(stdout);

    // Initialize Frame Buffer

    frameBuffer = (Pixel **)malloc(RESOLUTION_WIDTH * sizeof(Pixel *));
    if (frameBuffer == NULL)
    {
        fprintf(stderr, "\nError: Cannot reserve memory for the frame buffer.\n");
        exit(1);
    }
    for (int i = 0; i <= RESOLUTION_WIDTH; i++)
    {
        frameBuffer[i] = (Pixel *)malloc(RESOLUTION_HEIGHT * sizeof(Pixel *));
        if (frameBuffer[i] == NULL)
        {
            fprintf(stderr, "\nError: Cannot reserve memory for the frame buffer.\n");
            for (int j = 0; j < i; j++)
            {
                free(frameBuffer[j]);
            }
            free(frameBuffer);
            exit(1);
        }
    }

    if (universal_world->output_to_screen)
    {
        initWindow();
    }

    bool continue_program = universal_world->output_to_screen;
    bool processing = false;
    SDL_Event event;

    while (continue_program)
    {

        while (SDL_PollEvent(&event))
        {
            if (processing)
            {
                continue;
            }
            if (event.type == SDL_QUIT || event.type == SDL_APP_TERMINATING)
            {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                continue_program = false;
            }
            processing = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_EXPOSED)
            {
                redraw_screen(universal_world);
            }
            processing = false;
        }
    }

    // Save the image to a file
    if (!universal_world->output_to_screen)
    {
        mapWorldToFrameBuffer();
        printf("Saving image to scene.ppm...\n");
        save_to_image();
        printf("Image saved to scene.ppm\n");
    }

    // Free allocated memory

    for (int i = 0; i <= RESOLUTION_WIDTH; i++)
    {
        free(frameBuffer[i]);
    }
    free(frameBuffer);

    free_universal_world(universal_world);

    exit(0);
}
