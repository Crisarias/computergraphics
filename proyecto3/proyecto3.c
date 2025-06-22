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

int RESOLUTION_WIDTH = 1008;
int RESOLUTION_HEIGHT = 567;

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
        "Proyecto 3 - Cristian Arias",
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

Intersection *get_intersection_object_ray(Ray ray, Sphere *sphere)
{
    Intersection *intersection = (Intersection *)malloc(sizeof(Intersection));
    Vector origin_center = vector_subtract(ray.origin, sphere->center);
    double alpha =  vector_dot(ray.direction, ray.direction);
    double beta = 2.0 * vector_dot(origin_center, ray.direction);
    double gamma =  vector_dot(origin_center, origin_center) - pow(sphere->radius, 2);
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
    } else if (t2 > EPSILON && t2 < t1)
    {
        intersection->t = t2;
    } else {
        free(intersection);
        return NULL;
    }
    intersection->intersection_point.x = ray.origin.x + ray.direction.x * intersection->t;
    intersection->intersection_point.y = ray.origin.y + ray.direction.y * intersection->t;
    intersection->intersection_point.z = ray.origin.z + ray.direction.z * intersection->t;
    intersection->sphere = sphere;    
    return intersection;
}

Intersection *get_first_intersection(Ray ray)
{
    Intersection *intersection = NULL;
    double tmin = 999999999999999;
    for (int i = 0; i < universal_world->spheres_count; i++)
    {
        Intersection *intersection_object_ray = get_intersection_object_ray(ray, &universal_world->sphere_array[i]);
        Sphere sphere = universal_world->sphere_array[i];
        if (intersection_object_ray != NULL && intersection_object_ray -> t < tmin)
        {
            tmin = intersection_object_ray -> t;
            intersection = intersection_object_ray;
        }
        else
        {
            free(intersection_object_ray);
        }
    }
    return intersection;
}

Pixel get_pixel_color(Ray ray) {
    Pixel pixel;
    Intersection *intersection = get_first_intersection(ray);
    if (intersection != NULL)
    {
        pixel.r = intersection->sphere->r;
        pixel.g = intersection->sphere->g;
        pixel.b = intersection->sphere->b;
        Vector center_intersection = vector_subtract(intersection->intersection_point, intersection->sphere->center);
        Vector N = vector_normalize(center_intersection);
        double I = 0.0;
        double E = 0.0;
        for (int i = 0; i < universal_world -> lights_count; i++)
        {
            LightSource light = universal_world -> light_source_array[i];
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
                if (shadow_intersection == NULL) {
                    // No intersection - light isn't blocked
                    I = I + (cos_theta * intersection->sphere->kd * fatt * light.lp);
                    double cos_angle = vector_dot(R, V);
                    if (cos_angle > 0)
                    {
                        E = E + (pow(cos_angle, intersection->sphere->kn) * intersection->sphere->ks * fatt * light.lp);
                    }
                } else {
                    // Compare the distance to the intersection with the distance to the light
                    double distance_to_obstacle = shadow_intersection->t;
                    
                    if (distance_to_obstacle > distance_to_light) {
                        // Obstacle is beyond the light source, so light isn't blocked
                        I = I + (cos_theta * intersection->sphere->kd * fatt * light.lp);
                        double cos_angle = vector_dot(R, V);
                        if (cos_angle > 0)
                        {
                            E = E + (pow(cos_angle, intersection->sphere->kn) * intersection->sphere->ks * fatt * light.lp);
                        }
                    } // else light is blocked by an object
                    
                    // Free the shadow intersection memory
                    free(shadow_intersection);
                }                
            }
        }
        I = I + (universal_world->ia * intersection->sphere->ka);
        I = fmin(1,I);
        E = fmin(1,E);
        pixel.r = (unsigned char)(pixel.r * I);
        pixel.g = (unsigned char)(pixel.g * I);
        pixel.b = (unsigned char)(pixel.b * I);
        pixel.r = (unsigned char)(pixel.r + E*(255-pixel.r));
        pixel.g = (unsigned char)(pixel.g + E*(255-pixel.g));
        pixel.b = (unsigned char)(pixel.b + E*(255-pixel.b));
        free(intersection);
    } else {      
        // No intersection with any sphere, set pixel to background  
        pixel.r = 192;
        pixel.g = 192;
        pixel.b = 192;
    }
    return pixel;
}

void mapWorldToFrameBuffer()
{
    for (int i = 0; i < RESOLUTION_WIDTH; i++)
    {
        for (int j = 0; j < RESOLUTION_HEIGHT; j++)
        {
            Ray ray;
            ray.origin = universal_world -> eye;

            double xw = (double)(i + 0.5) * (universal_world->projection_maxX -universal_world->projection_minX) / RESOLUTION_WIDTH + universal_world->projection_minX;
            double yw = (double)(j + 0.5) * (universal_world->projection_maxY - universal_world->projection_minY) / RESOLUTION_HEIGHT + universal_world->projection_minY;
            double zw = 0.00;

            Vector point = vector_init(xw, yw, zw);
            Vector direction = vector_subtract(point, ray.origin);
            ray.direction = vector_normalize(direction);
            Pixel color = get_pixel_color(ray);
            setColor(color.r, color.g, color.b);
            plot_pixel(i, j);        
        }
        if (universal_world->output_to_screen) {
            render_frame_buffer_column(i);
        }        
    }
}

void redraw_screen()
{
    mapWorldToFrameBuffer();
    render_frame_buffer();
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

    printf("Universal world created\n");
    printf("Sphere count: %d\n", universal_world->spheres_count);
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

    if(universal_world->output_to_screen)
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
    if(!universal_world->output_to_screen)
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
