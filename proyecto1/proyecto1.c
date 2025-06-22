#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "utils.h"

// Definitions

Pixel color;
Pixel **frameBufferPPM;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int resolution;
int numberOfLines;
int numberOfRuns;
int programVersion;
int runningAlgorithm;

// Functions

void setColor(int r, int g, int b)
{
    color.r = r;
    color.g = g;
    color.b = b;
}

void user_pause() {
    if (programVersion == 3) {
        printf("\nPress Enter to continue the program.\n");
        getchar(); 
    }    
}

void save_lines_to_image()
{
    char *fileName;
    if (runningAlgorithm == 1)
    {
        fileName = "BruteForceOutput.ppm";
    }
    else if (runningAlgorithm == 2)
    {
        fileName = "IncrementalOutput.ppm";
    }
    else if (runningAlgorithm == 3)
    {
        fileName = "IncrementalV2Output.ppm";
    }
    else
    {
        fileName = "BresenhamV2Output.ppm";
    }
    Pixel image[resolution][resolution];
    for (int i = 0; i < resolution; i++)
    {
        for (int j = 0; j < resolution; j++)
        {
            image[i][j].r = frameBufferPPM[i][j].r;
            image[i][j].g = frameBufferPPM[i][j].g;
            image[i][j].b = frameBufferPPM[i][j].b;
        }
    }

    FILE *imageFile = fopen(fileName, "wb");
    if (!imageFile)
    {
        fprintf(stderr, "\nError: Cannot create or update file.\n");
        free(frameBufferPPM);
        exit(1);
    }
    fprintf(imageFile, "P6\n%d %d\n255\n", resolution, resolution);
    fwrite(image, sizeof(Pixel), resolution * resolution, imageFile);
    fclose(imageFile);
}

void clear_image()
{
    if (programVersion == 2)
    {
        setColor(255, 255, 255);
        for (int y = 0; y < resolution; y++)
        {
            for (int x = 0; x < resolution; x++)
            {
                frameBufferPPM[y][x].r = color.r;
                frameBufferPPM[y][x].g = color.g;
                frameBufferPPM[y][x].b = color.b;
            }
        }
    }
}

void print_image()
{
    if (programVersion == 2)
    {
        save_lines_to_image();
    }
    if (programVersion == 3)
    {
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

void print_screen()
{
    if (programVersion == 3)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // Nothing Happens
        }

        SDL_RenderPresent(renderer);
    }
}

void set_window_title() {
    if (programVersion == 3) {
        char title[100];
        if (runningAlgorithm == 1)
        {
            sprintf(title, "Brute Force Algorithm");
        }
        else if (runningAlgorithm == 2)
        {
            sprintf(title, "Incremental Algorithm");
        }
        else if (runningAlgorithm == 3)
        {
            sprintf(title, "Incremental V2 Algorithm");
        }
        else
        {
            sprintf(title, "Bresenham Algorithm");
        }
        SDL_SetWindowTitle(window, title);
    }    
}

void plot_pixel(int x, int y)
{
    if (programVersion == 2)
    {
        frameBufferPPM[x][y].r = color.r;
        frameBufferPPM[x][y].g = color.g;
        frameBufferPPM[x][y].b = color.b;
    }
    if (programVersion == 3)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

void draw_line_brute_force(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    long double m, b, y;

    if (abs(line->x1 - line->x0) >= abs(line->y1 - line->y0))
    {
        // Slope is shallow
        if (x0 > x1)
        {
            x0 = line->x1;
            y0 = line->y1;
            x1 = line->x0;
            y1 = line->y0;
        }
        m = (double)(y1 - y0) / (double)(x1 - x0);
        b = y0 - m * x0;

        for (int i = x0; i <= x1; i++)
        {
            y = m * i + b;
            plot_pixel(i, round(y));
        }
    }
    else
    {
        // Slope is Steep
        if (y0 > y1)
        {
            x0 = line->x1;
            y0 = line->y1;
            x1 = line->x0;
            y1 = line->y0;
        }
        m = (double)(x1 - x0) / (double)(y1 - y0);
        b = x0 - m * y0;
        for (int i = y0; i <= y1; i++)
        {
            y = m * i + b;
            plot_pixel(round(y), i);
        }
    }
}

void draw_line_incremental(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    long double m, y;

    if (abs(line->x1 - line->x0) >= abs(line->y1 - line->y0))
    {
        // Slope is shallow
        if (x0 > x1)
        {
            x0 = line->x1;
            y0 = line->y1;
            x1 = line->x0;
            y1 = line->y0;
        }
        m = (double)(y1 - y0) / (double)(x1 - x0);
        y = y0;
        for (int i = x0; i <= x1; i++)
        {
            plot_pixel(i, round(y));
            y += m;
        }
    }
    else
    {
        // Slope is Steep
        if (y0 > y1)
        {
            x0 = line->x1;
            y0 = line->y1;
            x1 = line->x0;
            y1 = line->y0;
        }
        m = (double)(x1 - x0) / (double)(y1 - y0);
        y = x0;
        for (int i = y0; i <= y1; i++)
        {
            plot_pixel(round(y), i);
            y += m;
        }
    }
}

int max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

void draw_line_incremental_v2(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    long double x, y, x_step, y_step;
    int width;

    width = max(abs(x1 - x0), abs(y1 - y0));
    x_step = (double)(x1 - x0) / (double)width;
    y_step = (double)(y1 - y0) / (double)width;
    x = x0;
    y = y0;
    for (int i = 0; i <= width; i++)
    {
        plot_pixel(round(x), round(y));
        x = x + x_step;
        y = y + y_step;
    }
}

void draw_line_bresenham_1th_octant(int x0, int y0, int x1, int y1)
{
    int delta_E, delta_NE, d, xp, yp;

    delta_E = 2 * (y1 - y0);
    delta_NE = 2 * ((y1 - y0) - (x1 - x0));
    xp = x0;
    yp = y0;
    plot_pixel(xp, yp);
    d = 2 * (y1 - y0) - (x1 - x0);
    while (xp < x1)
    {
        if (d <= 0)
        {
            xp++;
            d = d + delta_E;
        }
        else
        {
            xp++;
            yp++;
            d = d + delta_NE;
        }
        plot_pixel(xp, yp);
    }
}

void draw_line_bresenham_2th_octant(int x0, int y0, int x1, int y1)
{
    int delta_N, delta_NE, d, xp, yp;

    delta_N = 2 * (x1 - x0);
    delta_NE = 2 * ((x1 - x0) - (y1 - y0));
    xp = x0;
    yp = y0;
    plot_pixel(xp, yp);
    d = 2 * (x1 - x0) - (y1 - y0);
    while (yp < y1)
    {
        if (d <= 0)
        {
            yp++;
            d = d + delta_N;
        }
        else
        {
            yp++;
            xp++;
            d = d + delta_NE;
        }
        plot_pixel(xp, yp);
    }
}

void draw_line_bresenham_3th_octant(int x0, int y0, int x1, int y1)
{
    int delta_N, delta_NW, d, xp, yp;

    delta_N = 2 * (x0 - x1);
    delta_NW = 2 * ((x0 - x1) - (y1 - y0));
    xp = x0;
    yp = y0;
    plot_pixel(xp, yp);
    d = 2 * (x0 - x1) - (y1 - y0);
    while (yp < y1)
    {
        if (d <= 0)
        {
            yp++;
            d = d + delta_N;
        }
        else
        {
            yp++;
            xp--;
            d = d + delta_NW;
        }
        plot_pixel(xp, yp);
    }
}

void draw_line_bresenham_4th_octant(int x0, int y0, int x1, int y1)
{
    int delta_W, delta_NW, d, xp, yp;

    delta_W = 2 * (y1 - y0);
    delta_NW = 2 * ((y1 - y0) - (x0 - x1));
    xp = x0;
    yp = y0;
    plot_pixel(xp, yp);
    d = 2 * (y1 - y0) - (x0 - x1);
    while (xp > x1)
    {
        if (d <= 0)
        {
            xp--;
            d = d + delta_W;
        }
        else
        {
            xp--;
            yp++;
            d = d + delta_NW;
        }
        plot_pixel(xp, yp);
    }
}

void draw_line_bresenham_5th_octant(int x0, int y0, int x1, int y1)
{
    int delta_W, delta_SW, d, xp, yp;

    delta_W = 2 * (y0 - y1);
    delta_SW = 2 * ((y0 - y1) - (x0 - x1));
    xp = x0;
    yp = y0;
    plot_pixel(xp, yp);
    d = 2 * (y0 - y1) - (x0 - y1);
    while (xp > x1)
    {
        if (d <= 0)
        {
            xp--;
            d = d + delta_W;
        }
        else
        {
            xp--;
            yp--;
            d = d + delta_SW;
        }
        plot_pixel(xp, yp);
    }
}

void draw_line_bresenham_6th_octant(int x0, int y0, int x1, int y1)
{
    int delta_S, delta_SW, d, xp, yp;

    delta_S = 2 * (x0 - x1);
    delta_SW = 2 * ((x0 - x1) - (y0 - y1));
    xp = x0;
    yp = y0;
    plot_pixel(xp, yp);
    d = 2 * (x0 - x1) - (y0 - y1);
    while (yp > y1)
    {
        if (d <= 0)
        {
            yp--;
            d = d + delta_S;
        }
        else
        {
            yp--;
            xp--;
            d = d + delta_SW;
        }
        plot_pixel(xp, yp);
    }
}

void draw_line_bresenham_7th_octant(int x0, int y0, int x1, int y1)
{
    int delta_S, delta_SE, d, xp, yp;

    delta_S = 2 * (x1 - x0);
    delta_SE = 2 * ((x1 - x0) - (y0 - y1));
    xp = x0;
    yp = y0;
    plot_pixel(xp, yp);
    d = 2 * (x1 - x0) - (y0 - y1);
    while (yp > y1)
    {
        if (d <= 0)
        {
            yp--;
            d = d + delta_S;
        }
        else
        {
            yp--;
            xp++;
            d = d + delta_SE;
        }
        plot_pixel(xp, yp);
    }
}

void draw_line_bresenham_8th_octant(int x0, int y0, int x1, int y1)
{
    int delta_E, delta_SE, d, xp, yp;

    delta_E = 2 * (y0 - y1);
    delta_SE = 2 * ((y0 - y1) - (x1 - x0));
    xp = x0;
    yp = y0;
    plot_pixel(xp, yp);
    d = 2 * (y0 - y1) - (x1 - x0);
    while (xp < x1)
    {
        if (d <= 0)
        {
            xp++;
            d = d + delta_E;
        }
        else
        {
            xp++;
            yp--;
            d = d + delta_SE;
        }
        plot_pixel(xp, yp);
    }
}

void draw_line_bresenham(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;

    if (abs(line->x1 - line->x0) >= abs(line->y1 - line->y0))
    {
        // Slope is shallow
        if (x0 <= x1 && y0 <= y1) // 1th octant
        {
            draw_line_bresenham_1th_octant(x0, y0, x1, y1);
        }
        else if (x0 <= x1 && y0 > y1) // 8th octant
        {
            draw_line_bresenham_8th_octant(x0, y0, x1, y1);
        }
        else if (x0 > x1 && y0 <= y1) // 4th octant
        {
            draw_line_bresenham_4th_octant(x0, y0, x1, y1);
        }
        else // 5th octant
        {
            draw_line_bresenham_5th_octant(x0, y0, x1, y1);
        }
    }
    else
    {
        // Slope is Steep
        if (x0 <= x1 && y0 <= y1) // 2th octant
        {
            draw_line_bresenham_2th_octant(x0, y0, x1, y1);
        }
        else if (x0 <= x1 && y0 > y1) // 7th octant
        {
            draw_line_bresenham_7th_octant(x0, y0, x1, y1);
        }
        else if (x0 > x1 && y0 <= y1) // 3th octant
        {
            draw_line_bresenham_3th_octant(x0, y0, x1, y1);
        }
        else // 6th octant
        {
            draw_line_bresenham_6th_octant(x0, y0, x1, y1);
        }
    }
}

void draw_lines()
{
    Line linesToDraw[numberOfLines];
    srand(time(NULL));

    // Generating Lines to draw

    for (int i = 0; i < numberOfLines; i++)
    {
        linesToDraw[i].x0 = rand() % (resolution - 1);
        linesToDraw[i].y0 = rand() % (resolution - 1);
        linesToDraw[i].x1 = rand() % (resolution - 1);
        linesToDraw[i].y1 = rand() % (resolution - 1);
    }

    // Running Brute force algorithm

    printf("Running Brute force algorithm. ");
    clock_t start1 = clock();
    runningAlgorithm = 1;
    clear_image();
    set_window_title();
    setColor(255, 0, 0);
    for (int i = 0; i < numberOfRuns; i++)
    {
        for (int i = 0; i < numberOfLines; i++)
        {
            draw_line_brute_force(&linesToDraw[i]);
        }
        print_screen();
    }
    print_image();
    clock_t stop1 = clock();
    double timetaken1 = ((double)(stop1 - start1) / CLOCKS_PER_SEC);
    printf("Algorithm took %.6f seconds to execute. \n\n", timetaken1);

    // Running Incremental algorithm

    user_pause();
    printf("Running Incremental algorithm. ");
    clock_t start2 = clock();
    runningAlgorithm = 2;
    clear_image();
    set_window_title();
    setColor(0, 0, 255);
    for (int i = 0; i < numberOfRuns; i++)
    {
        for (int i = 0; i < numberOfLines; i++)
        {
            draw_line_incremental(&linesToDraw[i]);
        }
        print_screen();
    }
    print_image();
    clock_t stop2 = clock();
    double timetaken2 = ((double)(stop2 - start2) / CLOCKS_PER_SEC);
    printf("Algorithm took %.6f seconds to execute. \n\n", timetaken2);

    // Running Incremental Version 2 algorithm

    user_pause();
    printf("Running Incremental v2 algorithm. ");
    clock_t start3 = clock();
    runningAlgorithm = 3;
    clear_image();
    set_window_title();
    setColor(0, 255, 0);
    for (int i = 0; i < numberOfRuns; i++)
    {
        for (int i = 0; i < numberOfLines; i++)
        {
            draw_line_incremental_v2(&linesToDraw[i]);
        }
        print_screen();
    }
    print_image();
    clock_t stop3 = clock();
    double timetaken3 = ((double)(stop3 - start3) / CLOCKS_PER_SEC);
    printf("Algorithm took %f seconds to execute. \n\n", timetaken3);

    // Running Incremental Bresenham algorithm

    user_pause();
    printf("Running Bresenham algorithm. ");
    clock_t start4 = clock();
    runningAlgorithm = 4;
    clear_image();
    set_window_title();
    setColor(0, 0, 0);
    for (int i = 0; i < numberOfRuns; i++)
    {
        for (int i = 0; i < numberOfLines; i++)
        {
            draw_line_bresenham(&linesToDraw[i]);
        }
        print_screen();
    }
    print_image();
    clock_t stop4 = clock();
    double timetaken4 = ((double)(stop4 - start4) / CLOCKS_PER_SEC);
    printf("Algorithm took %f seconds to execute. \n\n", timetaken4);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Error: This program must receive at least four arguments in this order <resolution> <numberOfLines> <numberOfRuns> <programVersion>.\n");
        return 1;
    }
    printf("------------------------------------\n");
    printf("Reading arguments in this order <resolution> <numberOfLines> <numberOfRuns> \n\n");
    for (int i = 1; i < argc; i++)
    {
        validate_number_argument(argv[i]);
        if (i == 1)
        {
            resolution = atoi(argv[i]);
            validate_number_value(resolution, "resolution");
            printf("Resolution set to %d x %d\n", resolution, resolution);
        }
        if (i == 2)
        {
            numberOfLines = atoi(argv[i]);
            validate_number_value(numberOfLines, "numberOfLines");
            printf("Number of lines set to %d\n", numberOfLines);
        }
        if (i == 3)
        {
            numberOfRuns = atoi(argv[i]);
            validate_number_value(numberOfRuns, "numberOfRuns");
            printf("Number of runs set to %d\n", numberOfRuns);
        }
    }
    printf("------------------------------------\n\n");

    for (int i = 1; i <= 3; i++)
    {
        programVersion = i;
        if (i == 1)
        {
            printf("Running version of program with no output\n");
            printf("*****************************************\n");

            draw_lines();

            printf("------------------------------------\n\n");
        }
        else if (programVersion == 2)
        {
            printf("Running version of program with image output\n");
            printf("********************************************\n");

            // Initializing required space in memory
            frameBufferPPM = (Pixel **)malloc(resolution * sizeof(Pixel *));
            if (frameBufferPPM == NULL)
            {
                fprintf(stderr, "\nError: Cannot reserve memory for the frame buffer.\n");
                exit(1);
            }
            for (int i = 0; i <= resolution; i++)
            {
                frameBufferPPM[i] = (Pixel *)malloc(resolution * sizeof(Pixel *));
                if (frameBufferPPM[i] == NULL)
                {
                    fprintf(stderr, "\nError: Cannot reserve memory for the frame buffer.\n");
                    free(frameBufferPPM);
                    exit(1);
                }
            }

            draw_lines();

            free(frameBufferPPM);
            printf("------------------------------------\n\n");
        }
        else
        {
            printf("Running version of program with on-screen output\n");
            printf("************************************************\n");
            if (SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
                exit(1);
            }
            window = SDL_CreateWindow(
                "Drawing Lines SDL",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                resolution,
                resolution,
                SDL_WINDOW_SHOWN);
            if (window == NULL)
            {
                printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
                SDL_Quit();
                exit(1);
            }
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

            draw_lines();

            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();       
            printf("------------------------------------\n\n");
        }
    }

    printf("\nPress Enter to continue the program.\n");
    getchar(); 
    return 0;
}