#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "signature.h"

void free_signature(Signature *signature)
{
    for (int i = 0; i < signature->curve_count; i++)
    {
        free(signature->curves[i].control_points);
    }
    free(signature->curves);
    free(signature);
}

Signature *create_signature_from_file(char *filepath_str)
{
    FILE *file = fopen(filepath_str, "r");
    if (!file)
    {
        perror("Failed to open file");
        exit(1);
    }

    Signature *signature = (Signature *)malloc(sizeof(Signature));
    signature->curves = NULL;
    signature->curve_count = 0;
    int curves_count = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    bool read_sample = false;
    int *s;
    s = (int *)malloc(sizeof(int));
    while ((read = getline(&line, &len, file)) != -1)
    {
        if (line[0] == '#')
        {
            continue; // Skip comment lines
        }

        if (line[0] == 'S')
        {
            read_sample = true;
            continue; // Move to next line
        }        

        if (read_sample)
        {
            const char *p = line;
            sscanf(p, "%d", s);
            read_sample = false;
            continue; // Move to next line
        }

        // Count the number of control points in the current line
        int control_points_count = 1;
        for (const char *p = line; *p; p++)
        {
            if (*p == ' ')
            {
                control_points_count++;
            }
        }

        // Allocate memory for the new Poligon and its vertices
        signature->curves = (Curve *)realloc(signature->curves, (curves_count + 1) * sizeof(Curve));
        Curve *curve = &signature->curves[curves_count];
        curve->control_points = (ControlPoint *)malloc(control_points_count * sizeof(ControlPoint));
        curve->control_point_count = control_points_count;
        curve->sample_count = *s;

        // Parse the line and fill the original_vertex_array
        const char *p = line;
        for (int i = 0; i < control_points_count; i++)
        {
            sscanf(p, "%lf,%lf", &curve->control_points[i].x, &curve->control_points[i].y);
            while (*p && *p != ' ')
                p++;
            if (*p)
                p++;
        }
        curves_count++;
    }

    signature->curve_count = curves_count;

    free(line);
    free(s);
    fclose(file);

    return signature;
}
