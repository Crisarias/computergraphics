#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "universal_world.h"

bool debug = false;

void update_poligon_boundaries(Poligon *poligon)
{
    poligon->minX = poligon->original_vertex_array[0].x;
    poligon->minY = poligon->original_vertex_array[0].y;
    poligon->maxX = poligon->original_vertex_array[0].x;
    poligon->maxY = poligon->original_vertex_array[0].y;

    for (int i = 1; i < poligon->original_vertex_count; i++)
    {
        if (poligon->original_vertex_array[i].x < poligon->minX)
            poligon->minX = poligon->original_vertex_array[i].x;
        if (poligon->original_vertex_array[i].y > poligon->minY)
            poligon->minY = poligon->original_vertex_array[i].y;
        if (poligon->original_vertex_array[i].x > poligon->maxX)
            poligon->maxX = poligon->original_vertex_array[i].x;
        if (poligon->original_vertex_array[i].y < poligon->maxY)
            poligon->maxY = poligon->original_vertex_array[i].y;
    }
}

void free_universal_world(Universal_world *universal_world)
{
    for (int i = 0; i < universal_world->poligon_count; i++)
    {
        free(universal_world->poligon_array[i].original_vertex_array);
        free(universal_world->poligon_array[i].next_vertex_array);
        if (universal_world->poligon_array[i].texture_filepath != NULL)
        {
            free(universal_world->poligon_array[i].texture_filepath);
        }
    }
    free(universal_world->poligon_array);
    free(universal_world);
    universal_world = NULL;
}

Universal_world *create_universal_world_from_file(char *filepath_str)
{
    int vertex_count = 1;
    FILE *file = fopen(filepath_str, "r");
    if (!file)
    {
        perror("Failed to open file");
        exit(1);
    }

    Universal_world *world = (Universal_world *)malloc(sizeof(Universal_world));
    world->poligon_array = NULL;
    world->total_vertex_count = 0;
    int poligon_count = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    bool read_color = false;
    bool read_texture = true;
    int *r, *g, *b;
    char *texture_filepath = NULL;
    r = (int *)malloc(sizeof(int));
    g = (int *)malloc(sizeof(int));
    b = (int *)malloc(sizeof(int));
    while ((read = getline(&line, &len, file)) != -1)
    {
        if (line[0] == '#')
        {
            continue; // Skip comment lines
        }

        if (line[0] == 'C')
        {
            read_color = true;
            continue; // Move to next line
        }

        if (line[0] == 'T')
        {
            read_texture = true;
            continue; // Move to next line
        }

        if (read_color)
        {
            const char *p = line;
            for (int i = 0; i < 3; i++)
            {
                sscanf(p, "%d,%d,%d", r, g, b);
            }
            read_color = false;
            continue; // Move to next line
        }

        if (read_texture)
        {
            texture_filepath = (char *)realloc(texture_filepath, (strlen(line) + 1) * sizeof(char));
            sscanf(line, "%s", texture_filepath);
            read_texture = false;
            continue; // Move to next line
        }

        // Count the number of vertices in the current line
        int vertex_count = 1;
        for (const char *p = line; *p; p++)
        {
            if (*p == ' ')
            {
                vertex_count++;
            }
        }

        // Allocate memory for the new Poligon and its vertices
        world->poligon_array = (Poligon *)realloc(world->poligon_array, (poligon_count + 1) * sizeof(Poligon));
        Poligon *poligon = &world->poligon_array[poligon_count];
        poligon->original_vertex_array = (Vertex *)malloc(vertex_count * sizeof(Vertex));
        poligon->original_vertex_count = vertex_count;
        poligon->next_vertex_array = (Vertex *)malloc(vertex_count * 2 * sizeof(Vertex));
        poligon->next_vertex_count = vertex_count;

        // Parse the line and fill the original_vertex_array
        const char *p = line;
        for (int i = 0; i < vertex_count; i++)
        {
            sscanf(p, "%lf,%lf", &poligon->original_vertex_array[i].x, &poligon->original_vertex_array[i].y);
            sscanf(p, "%lf,%lf", &poligon->next_vertex_array[i].x, &poligon->next_vertex_array[i].y);
            while (*p && *p != ' ')
                p++;
            if (*p)
                p++;
        }

        // Set poligon solid color
        poligon->r = *r;
        poligon->g = *g;
        poligon->b = *b;

        if (texture_filepath == NULL)
        {
            poligon->texture_filepath = NULL;
        }
        else
        {
            poligon->texture_filepath = (char *)malloc((strlen(texture_filepath) + 1) * sizeof(char));
            sscanf(texture_filepath, "%s", poligon->texture_filepath);
            free(texture_filepath);
            texture_filepath = NULL;
        }

        update_poligon_boundaries(poligon);

        world->total_vertex_count += vertex_count;
        poligon_count++;
    }

    world->poligon_count = poligon_count;

    free(line);
    free(r);
    free(g);
    free(b);
    fclose(file);

    return world;
}

bool isInside(double x0, double y0, double x1, double y1, int xMin, int yMin, int xMax, int yMax)
{
    return x0 >= xMin && x0 <= xMax && y0 <= yMin && y0 >= yMax && x1 >= xMin && x1 <= xMax && y1 <= yMin && y1 >= yMax;
}

bool isOutside(double x0, double y0, double x1, double y1, int xMin, int yMin, int xMax, int yMax)
{
    return (x0 < xMin && x1 < xMin) || (x0 > xMax && x1 > xMax) || (y0 > yMin && y1 > yMin) || (y0 < yMax && y1 < yMax);
}

void copy_to_next_vertex_array(Vertex *source_array, Poligon *poligon, int count)
{
    for (int i = 0; i < count; i++)
    {
        poligon->next_vertex_array[i].x = source_array[i].x;
        poligon->next_vertex_array[i].y = source_array[i].y;
    }
    poligon->next_vertex_count = count;
}

void rotate(Universal_world *universal_world, double xC, double yC, int angle_degree)
{

    // Calculate sine and cosine of the angle
    double cos_angle = cos(angle_degree * M_PI / 180);
    double sin_angle = sin(angle_degree * M_PI / 180);

    // Create transformation matrix
    double transform[3][3] = {
        {cos_angle, -sin_angle, xC - xC * cos_angle + yC * sin_angle},
        {sin_angle, cos_angle, yC - xC * sin_angle - yC * sin_angle},
        {0, 0, 1}};

    // Apply transformation to universal world
    for (int i = 0; i < universal_world->poligon_count; i++)
    {

        Poligon *poligon = &universal_world->poligon_array[i];
        poligon->minX = 0;
        poligon->minY = 0;
        poligon->maxX = 0;
        poligon->maxY = 0;
        for (int j = 0; j < poligon->original_vertex_count; j++)
        {
            double x_new = transform[0][0] * poligon->original_vertex_array[j].x + transform[0][1] * poligon->original_vertex_array[j].y + transform[0][2];
            double y_new = transform[1][0] * poligon->original_vertex_array[j].x + transform[1][1] * poligon->original_vertex_array[j].y + transform[1][2];
            poligon->original_vertex_array[j].x = x_new;
            poligon->original_vertex_array[j].y = y_new;
            update_poligon_boundaries(poligon);
        }
    }
}

void markPoligonNeedsClipping(Universal_world *universal_world, double xMin, double yMin, double xMax, double yMax)
{
    for (int i = 0; i < universal_world->poligon_count; i++)
    {
        Poligon *poligon = &universal_world->poligon_array[i];
        copy_to_next_vertex_array(poligon->original_vertex_array, poligon, poligon->original_vertex_count);
        bool isPoligonOutside = poligon->maxX < xMin || poligon->minX > xMax || poligon->maxY > yMin || poligon->minY < yMax;
        bool isPoligonInside = poligon->minX >= xMin && poligon->maxX <= xMax && poligon->minY <= yMin && poligon->maxY >= yMax;
        if (isPoligonOutside)
        {
            poligon->needs_clipping = false;
            poligon->is_out_of_window = true;
            continue;
        }
        if (isPoligonInside)
        {
            poligon->needs_clipping = false;
            poligon->is_out_of_window = false;
            continue;
        }
        poligon->needs_clipping = true;
        poligon->is_out_of_window = false;
    }
}

double get_y_intercept(double x0, double y0, double x1, double y1, double x)
{
    if (y0 == y1)
    {
        return y0; // Horizontal line
    }
    double m = (y1 - y0) / (x1 - x0);
    return m * (x - x0) + y0;
}

double get_x_intercept(double x0, double y0, double x1, double y1, double y)
{
    if (x0 == x1)
    {
        return x0; // vertical line
    }
    double m = (x1 - x0) / (y1 - y0);
    return m * (y - y0) + x0;
}

int cohen_suhterland_min_x(Vertex *new_vertex_array, int currentVextexCount, double xMin, double x0, double y0, double x1, double y1)
{
    bool isCase1 = x0 >= xMin && x1 >= xMin;
    bool isCase2 = x0 >= xMin && x1 < xMin;
    bool isCase3 = x0 < xMin && x1 < xMin;
    bool isCase4 = x0 < xMin && x1 >= xMin;
    int newVertexCount = currentVextexCount;
    if (isCase1)
    {
        new_vertex_array[newVertexCount].x = x1;
        new_vertex_array[newVertexCount].y = y1;
        newVertexCount++;
    }
    else if (isCase2)
    {
        new_vertex_array[newVertexCount].x = xMin;
        new_vertex_array[newVertexCount].y = get_y_intercept(x0, y0, x1, y1, xMin);
        newVertexCount++;
    }
    else if (isCase3)
    {
        // Do nothing
    }
    else if (isCase4)
    {
        new_vertex_array[newVertexCount].x = xMin;
        new_vertex_array[newVertexCount].y = get_y_intercept(x0, y0, x1, y1, xMin);
        newVertexCount++;
        new_vertex_array[newVertexCount].x = x1;
        new_vertex_array[newVertexCount].y = y1;
        newVertexCount++;
    }
    return newVertexCount;
}

int cohen_suhterland_max_x(Vertex *new_vertex_array, int currentVextexCount, double xMax, double x0, double y0, double x1, double y1)
{
    bool isCase1 = x0 <= xMax && x1 <= xMax;
    bool isCase2 = x0 <= xMax && x1 > xMax;
    bool isCase3 = x0 > xMax && x1 > xMax;
    bool isCase4 = x0 > xMax && x1 <= xMax;
    int newVertexCount = currentVextexCount;
    if (isCase1)
    {
        new_vertex_array[newVertexCount].x = x1;
        new_vertex_array[newVertexCount].y = y1;
        newVertexCount++;
    }
    else if (isCase2)
    {
        new_vertex_array[newVertexCount].x = xMax;
        new_vertex_array[newVertexCount].y = get_y_intercept(x0, y0, x1, y1, xMax);
        newVertexCount++;
    }
    else if (isCase3)
    {
        // Do nothing
    }
    else if (isCase4)
    {
        new_vertex_array[newVertexCount].x = xMax;
        new_vertex_array[newVertexCount].y = get_y_intercept(x0, y0, x1, y1, xMax);
        newVertexCount++;
        new_vertex_array[newVertexCount].x = x1;
        new_vertex_array[newVertexCount].y = y1;
        newVertexCount++;
    }
    return newVertexCount;
}

int cohen_suhterland_min_y(Vertex *new_vertex_array, int currentVextexCount, double yMin, double x0, double y0, double x1, double y1)
{
    bool isCase1 = y0 <= yMin && y1 <= yMin;
    bool isCase2 = y0 <= yMin && y1 > yMin;
    bool isCase3 = y0 > yMin && y1 > yMin;
    bool isCase4 = y0 > yMin && y1 <= yMin;

    int newVertexCount = currentVextexCount;
    if (isCase1)
    {
        new_vertex_array[newVertexCount].x = x1;
        new_vertex_array[newVertexCount].y = y1;
        newVertexCount++;
    }
    else if (isCase2)
    {
        new_vertex_array[newVertexCount].x = get_x_intercept(x0, y0, x1, y1, yMin);
        new_vertex_array[newVertexCount].y = yMin;
        newVertexCount++;
    }
    else if (isCase3)
    {
        // Do nothing
    }
    else if (isCase4)
    {
        new_vertex_array[newVertexCount].x = get_x_intercept(x0, y0, x1, y1, yMin);
        new_vertex_array[newVertexCount].y = yMin;
        newVertexCount++;
        new_vertex_array[newVertexCount].x = x1;
        new_vertex_array[newVertexCount].y = y1;
        newVertexCount++;
    }
    return newVertexCount;
}

int cohen_suhterland_max_y(Vertex *new_vertex_array, int currentVextexCount, double yMax, double x0, double y0, double x1, double y1)
{
    bool isCase1 = y0 >= yMax && y1 >= yMax;
    bool isCase2 = y0 >= yMax && y1 < yMax;
    bool isCase3 = y0 < yMax && y1 < yMax;
    bool isCase4 = y0 < yMax && y1 >= yMax;
    int newVertexCount = currentVextexCount;
    if (isCase1)
    {
        new_vertex_array[newVertexCount].x = x1;
        new_vertex_array[newVertexCount].y = y1;
        newVertexCount++;
    }
    else if (isCase2)
    {
        new_vertex_array[newVertexCount].x = get_x_intercept(x0, y0, x1, y1, yMax);
        new_vertex_array[newVertexCount].y = yMax;
        newVertexCount++;
    }
    else if (isCase3)
    {
        // Do nothing
    }
    else if (isCase4)
    {
        new_vertex_array[newVertexCount].x = get_x_intercept(x0, y0, x1, y1, yMax);
        new_vertex_array[newVertexCount].y = yMax;
        newVertexCount++;
        new_vertex_array[newVertexCount].x = x1;
        new_vertex_array[newVertexCount].y = y1;
        newVertexCount++;
    }
    return newVertexCount;
}

void clipping_universal_world(Universal_world *universal_world, double xMin, double yMin, double xMax, double yMax)
{
    markPoligonNeedsClipping(universal_world, xMin, yMin, xMax, yMax);
    for (int i = 0; i < universal_world->poligon_count; i++)
    {
        Poligon *poligon = &universal_world->poligon_array[i];
        if (!poligon->needs_clipping)
        {
            continue;
        }
        Vertex *new_vertex_array = (Vertex *)malloc(poligon->original_vertex_count * 2 * sizeof(Vertex));

        int new_vertex_count = 0;

        // Left Border Xmin
        for (int j = 1; j < poligon->next_vertex_count; j++)
        {
            new_vertex_count = cohen_suhterland_min_x(new_vertex_array, new_vertex_count, xMin,
                                                      poligon->next_vertex_array[j - 1].x, poligon->next_vertex_array[j - 1].y,
                                                      poligon->next_vertex_array[j].x, poligon->next_vertex_array[j].y);
        }
        new_vertex_count = cohen_suhterland_min_x(new_vertex_array, new_vertex_count, xMin,
                                                  poligon->next_vertex_array[poligon->next_vertex_count - 1].x, poligon->next_vertex_array[poligon->next_vertex_count - 1].y,
                                                  poligon->next_vertex_array[0].x, poligon->next_vertex_array[0].y);
        copy_to_next_vertex_array(new_vertex_array, poligon, new_vertex_count);

        // Right Border Xmax
        new_vertex_count = 0;
        for (int j = 1; j < poligon->next_vertex_count; j++)
        {
            new_vertex_count = cohen_suhterland_max_x(new_vertex_array, new_vertex_count, xMax,
                                                      poligon->next_vertex_array[j - 1].x, poligon->next_vertex_array[j - 1].y,
                                                      poligon->next_vertex_array[j].x, poligon->next_vertex_array[j].y);
        }
        new_vertex_count = cohen_suhterland_max_x(new_vertex_array, new_vertex_count, xMax,
                                                  poligon->next_vertex_array[poligon->next_vertex_count - 1].x, poligon->next_vertex_array[poligon->next_vertex_count - 1].y,
                                                  poligon->next_vertex_array[0].x, poligon->next_vertex_array[0].y);
        copy_to_next_vertex_array(new_vertex_array, poligon, new_vertex_count);

        // Top Border Ymax
        new_vertex_count = 0;
        for (int j = 1; j < poligon->next_vertex_count; j++)
        {
            new_vertex_count = cohen_suhterland_max_y(new_vertex_array, new_vertex_count, yMax,
                                                      poligon->next_vertex_array[j - 1].x, poligon->next_vertex_array[j - 1].y,
                                                      poligon->next_vertex_array[j].x, poligon->next_vertex_array[j].y);
        }
        new_vertex_count = cohen_suhterland_max_y(new_vertex_array, new_vertex_count, yMax,
                                                  poligon->next_vertex_array[poligon->next_vertex_count - 1].x, poligon->next_vertex_array[poligon->next_vertex_count - 1].y,
                                                  poligon->next_vertex_array[0].x, poligon->next_vertex_array[0].y);
        copy_to_next_vertex_array(new_vertex_array, poligon, new_vertex_count);

        // Bottom Border Ymin
        new_vertex_count = 0;
        for (int j = 1; j < poligon->next_vertex_count; j++)
        {
            new_vertex_count = cohen_suhterland_min_y(new_vertex_array, new_vertex_count, yMin,
                                                      poligon->next_vertex_array[j - 1].x, poligon->next_vertex_array[j - 1].y,
                                                      poligon->next_vertex_array[j].x, poligon->next_vertex_array[j].y);
        }
        new_vertex_count = cohen_suhterland_min_y(new_vertex_array, new_vertex_count, yMin,
                                                  poligon->next_vertex_array[poligon->next_vertex_count - 1].x, poligon->next_vertex_array[poligon->next_vertex_count - 1].y,
                                                  poligon->next_vertex_array[0].x, poligon->next_vertex_array[0].y);
        copy_to_next_vertex_array(new_vertex_array, poligon, new_vertex_count);
        free(new_vertex_array);
    }
}