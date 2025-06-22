#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "utils.h"
#include "universal_world.h"

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

    double D_dot_Q = vector_dot(delta, Q); // Dot product of delta with axis

    double Dx_dot_D = vector_dot(delta, direction); // Dot product of delta with axis    
    
    // Quadratic equation coefficients based on the mathematical derivation

    // a = |D|² - (D·Q)²
    double a = vector_dot(direction, direction) - pow(Q_dot_D, 2);
    
    // b = 2(D·delta - (D·Q)(delta·Q))
    double b = 2.0 * (Dx_dot_D- Q_dot_D * D_dot_Q);
    
    // c = |delta|² - (delta·Q)² - radius²
    double c = vector_dot(delta, delta) - pow(D_dot_Q, 2) - pow(radius, 2);
    
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


K
720,0,675,720,170,675
60,1,1,1,5,1,2,1,0,0
51,51,62
K
1175,0,675,1175,170,675
60,1,1,1,5,1,2,1,0,0
51,51,62