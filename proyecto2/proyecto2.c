#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "utils.h"
#include "universal_world.h"
#include "signature.h"

// Definitions

Pixel color;
Pixel **frameBuffer;
Texture **texturesCache;
int numTextures = 0;
IntersectionPoint **intersactions;
int *numberOfIntersactions;
SDL_Window *window = NULL;
Universal_world *universal_world = NULL;
Signature *signature = NULL;
SDL_Renderer *renderer = NULL;

char *default_texture_filepath = "textures/default_texture.ppm";
double x_min_window = 0;
double y_min_window = 1080;
double x_max_window = 1920;
double y_max_window = 0;
double z_scale = 1.0;
int coloring_mode = -1; // -1=None, 0 = Solid Colors, 1 = Different Textures, 2 = One Texture

int RESOLUTION_WIDTH = 1920;
int RESOLUTION_HEIGHT = 1080;

// Functions

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

void initWindow()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(
        "Proyecto 2 - Cristian Arias",
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

void print_screen()
{
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
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
    print_screen();
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

void clearIntersactionsBuffer()
{
    for (int i = 0; i <= RESOLUTION_HEIGHT; i++)
    {
        if (intersactions[i] != NULL)
        {
            free(intersactions[i]);
            intersactions[i] = NULL;
        }
        numberOfIntersactions[i] = 0;
    }
}

void plot_texel(int x, int y, Texture *texture)
{
    int yt = y % texture->resHeight;
    int xt = x % texture->resWidth;

    frameBuffer[x][y].r = texture->texel_array[yt][xt].r;
    frameBuffer[x][y].g = texture->texel_array[yt][xt].g;
    frameBuffer[x][y].b = texture->texel_array[yt][xt].b;
}

void draw_line_bresenham_1th_octant(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    int delta_E, delta_NE, d, xp, yp;
    int dx = x1 - x0;
    int dy = y1 - y0;

    delta_E = 2 * dy;
    delta_NE = 2 * (dy - dx);
    xp = x0;
    yp = y0;
    d = 2 * dy - dx;
    while (xp <= x1)
    {
        plot_pixel(xp, yp);
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
    }
}

void draw_line_bresenham_2th_octant(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    int delta_N, delta_NE, d, xp, yp;
    int dx = x1 - x0;
    int dy = y1 - y0;

    delta_N = 2 * dx;
    delta_NE = 2 * (dx - dy);
    xp = x0;
    yp = y0;
    d = 2 * dx - dy;
    while (yp <= y1)
    {
        plot_pixel(xp, yp);
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
    }
}

void draw_line_bresenham_3th_octant(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    int delta_N, delta_NW, d, xp, yp;
    int dx = x0 - x1;
    int dy = y1 - y0;

    delta_N = 2 * dx;
    delta_NW = 2 * (dx - dy);
    xp = x0;
    yp = y0;
    d = 2 * dx - dy;
    while (yp <= y1)
    {
        plot_pixel(xp, yp);
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
    }
}

void draw_line_bresenham_4th_octant(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    int delta_W, delta_NW, d, xp, yp;
    int dx = x0 - x1;
    int dy = y1 - y0;

    delta_W = 2 * dy;
    delta_NW = 2 * (dy - dx);
    xp = x0;
    yp = y0;
    d = 2 * dy - dx;
    while (xp >= x1)
    {
        plot_pixel(xp, yp);
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
    }
}

void draw_line_bresenham_5th_octant(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    int delta_W, delta_SW, d, xp, yp;
    int dx = x0 - x1;
    int dy = y0 - y1;

    delta_W = 2 * dy;
    delta_SW = 2 * (dy - dx);
    xp = x0;
    yp = y0;
    d = 2 * dy - dx;
    while (xp >= x1)
    {
        plot_pixel(xp, yp);
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
    }
}

void draw_line_bresenham_6th_octant(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    int delta_S, delta_SW, d, xp, yp;
    int dx = x0 - x1;
    int dy = y0 - y1;

    delta_S = 2 * dx;
    delta_SW = 2 * (dx - dy);
    xp = x0;
    yp = y0;
    d = 2 * dx - dy;
    while (yp >= y1)
    {
        plot_pixel(xp, yp);
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
    }
}

void draw_line_bresenham_7th_octant(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    int delta_S, delta_SE, d, xp, yp;
    int dx = x1 - x0;
    int dy = y0 - y1;

    delta_S = 2 * dx;
    delta_SE = 2 * (dx - dy);
    xp = x0;
    yp = y0;
    d = 2 * dx - dy;
    while (yp >= y1)
    {
        plot_pixel(xp, yp);
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
    }
}

void draw_line_bresenham_8th_octant(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;
    int delta_E, delta_SE, d, xp, yp;
    int dx = x1 - x0;
    int dy = y0 - y1;

    delta_E = 2 * dy;
    delta_SE = 2 * (dy - dx);
    xp = x0;
    yp = y0;
    d = 2 * dy - dx;
    while (xp <= x1)
    {
        plot_pixel(xp, yp);
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
    }
}

void draw_line_bresenham(Line *line)
{
    int x0 = line->x0;
    int y0 = line->y0;
    int x1 = line->x1;
    int y1 = line->y1;

    if (abs(x1 - x0) >= abs(y1 - y0))
    {
        // Slope is shallow
        if (x0 <= x1 && y0 <= y1) // 1th octant
        {
            draw_line_bresenham_1th_octant(line);
        }
        else if (x0 <= x1 && y0 > y1) // 8th octant
        {
            draw_line_bresenham_8th_octant(line);
        }
        else if (x0 > x1 && y0 <= y1) // 4th octant
        {
            draw_line_bresenham_4th_octant(line);
        }
        else // 5th octant
        {
            draw_line_bresenham_5th_octant(line);
        }
    }
    else
    {
        // Slope is Steep
        if (x0 <= x1 && y0 <= y1) // 2th octant
        {
            draw_line_bresenham_2th_octant(line);
        }
        else if (x0 <= x1 && y0 > y1) // 7th octant
        {
            draw_line_bresenham_7th_octant(line);
        }
        else if (x0 > x1 && y0 <= y1) // 3th octant
        {
            draw_line_bresenham_3th_octant(line);
        }
        else // 6th octant
        {
            draw_line_bresenham_6th_octant(line);
        }
    }
}

int factorial(int n)
{
    if (n == 0 || n == 1)
        return 1;
    return n * factorial(n - 1);
}

int binomial_coefficient(int n, int i)
{
    return factorial(n) / (factorial(i) * factorial(n - i));
}

void draw_curve_using_bezier(Curve * curve) {
    int n = curve->control_point_count - 1;
    int samples = curve->sample_count;
    for (int j = 0; j <= samples; j++)
    {
        double t = (double)j / (double)samples;
        double x = 0;
        double y = 0;
        
        for (int i = 0; i <= n; i++)
        {
            double coefficient = binomial_coefficient(n, i) * pow(1 - t, n - i) * pow(t, i);
            x += coefficient * curve->control_points[i].x;
            y += coefficient * curve->control_points[i].y;
        }

        plot_pixel((int)(x+0.5),(int)(y+0.5));
    }    
}

void render_signature() {
    setColor(255, 0, 239);
    for (int i = 0; i < signature->curve_count; i++)
    {
        Curve *curve = &signature->curves[i];
        if (curve->control_point_count > 0)
        {
            draw_curve_using_bezier(curve);
        }
    }
    
}

int mapXwtoXf(double xW)
{
    int xf = (int)((RESOLUTION_WIDTH * ((xW - x_min_window) / (x_max_window - x_min_window))) + 0.5);
    return xf;
}

int mapYwtoYf(double yW)
{
    int xy = (int)((RESOLUTION_HEIGHT * ((yW - y_max_window) / (y_min_window - y_max_window))) + 0.5);
    return xy;
}

void draw_lines(Poligon *poligon)
{
    double last_x = mapXwtoXf(poligon->next_vertex_array[0].x);
    double last_y = mapYwtoYf(poligon->next_vertex_array[0].y);
    poligon->next_vertex_array[0].xf = last_x;
    poligon->next_vertex_array[0].yf = last_y;
    double first_x = last_x;
    double first_y = last_y;
    for (int i = 0; i < poligon->next_vertex_count - 1; i++)
    {
        Line line;
        double xW1 = poligon->next_vertex_array[i + 1].x;
        double yW1 = poligon->next_vertex_array[i + 1].y;
        line.x0 = last_x;
        line.y0 = last_y;
        line.x1 = mapXwtoXf(xW1);
        line.y1 = mapYwtoYf(yW1);
        poligon->next_vertex_array[i + 1].xf = line.x1;
        poligon->next_vertex_array[i + 1].yf = line.y1;
        last_x = line.x1;
        last_y = line.y1;
        if (coloring_mode == -1) {
            draw_line_bresenham(&line);
        }
        
    }
    Line line;
    line.x0 = last_x;
    line.y0 = last_y;
    line.x1 = first_x;
    line.y1 = first_y;
    if (coloring_mode == -1) {
        draw_line_bresenham(&line);
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

Border *createBordersFromPoligon(Poligon *poligon, int *borderCount)
{
    *borderCount = poligon->next_vertex_count;
    Border *borders = (Border *)malloc(*borderCount * sizeof(Border));
    if (borders == NULL)
    {
        fprintf(stderr, "Error: Cannot allocate memory for borders.\n");
        exit(1);
    }

    for (int i = 0; i < *borderCount; i++)
    {
        int nextIndex = (i + 1) % *borderCount;
        int xf0 = poligon->next_vertex_array[i].xf;
        int yf0 = poligon->next_vertex_array[i].yf;
        int xf1 = poligon->next_vertex_array[nextIndex].xf;
        int yf1 = poligon->next_vertex_array[nextIndex].yf;
        if (yf0 >= yf1)
        {
            borders[i].x0 = xf0;
            borders[i].y0 = yf0;
            borders[i].x1 = xf1;
            borders[i].y1 = yf1;
        }
        else
        {
            borders[i].x0 = xf1;
            borders[i].y0 = yf1;
            borders[i].x1 = xf0;
            borders[i].y1 = yf0;
        }
    }

    qsort(borders, *borderCount, sizeof(Border), compareBorders);
    return borders;
}

void setActiveBorders(Border *borders, int borderCount, int scanline)
{
    for (int i = 0; i < borderCount; i++)
    {
        // Ignore Horizontal Border
        if (borders[i].y0 == borders[i].y1)
        {
            borders[i].active = false;
        }
        else if (borders[i].y1 == scanline)
        {
            borders[i].active = true;
        }
        else
        {
            borders[i].active = false;
        }
    }
}

void add_intersaction(int x, int y, int yHigh, int yLow)
{
    numberOfIntersactions[y] += 1;
    intersactions[y] = (IntersectionPoint *)realloc(intersactions[y], numberOfIntersactions[y] * sizeof(IntersectionPoint));
    intersactions[y][numberOfIntersactions[y] - 1].x = x;
    intersactions[y][numberOfIntersactions[y] - 1].is_y_high = yHigh == y;
    intersactions[y][numberOfIntersactions[y] - 1].is_y_low = yLow == y;
}

void order_intersactions(int y)
{
    qsort(intersactions[y], numberOfIntersactions[y], sizeof(IntersectionPoint), compareIntersactions);
}

void calculateIntersections(Border *borders, int borderCount, int scanline)
{
    for (int i = 0; i < borderCount; i++)
    {
        if (borders[i].active)
        {
            // Using Incremental algorithm to calculate all the intersactions
            int x0 = borders[i].x0;
            int y0 = borders[i].y0;
            int x1 = borders[i].x1;
            int y1 = borders[i].y1;

            // Vertical line
            if (x0 == x1)
            {
                for (int y = y1; y <= y0; y++)
                {
                    add_intersaction(x0, y, y1, y0);
                }
            }
            else
            {
                double slope = (double)(x1 - x0) / (double)(y0 - y1);
                double x = x1;
                add_intersaction(x1, y1, y1, y0);
                for (int y = y1 + 1; y <= y0; y++)
                {
                    x -= slope;
                    add_intersaction((int)(x + 0.5), y, y1, y0);
                }
            }
        }
    }
}

void color_poligon(Poligon *poligon)
{
    setColor(poligon->r, poligon->g, poligon->b);
    Texture *texture = NULL;
    if (coloring_mode == 1 && poligon->texture_filepath != NULL)
    {
        texture = getTextureFromCache(poligon->texture_filepath);
    }
    if (coloring_mode == 2)
    {
        texture = getTextureFromCache(default_texture_filepath);
    }
    int scanline = RESOLUTION_HEIGHT;
    while (scanline >= 0)
    {
        order_intersactions(scanline);
        int x0 = -1;
        int x1 = -1;
        for (int i = 0; i < numberOfIntersactions[scanline]; i++)
        {

            // ####################################

            if (i < numberOfIntersactions[scanline] - 1 &&
                ((intersactions[scanline][i].is_y_low && intersactions[scanline][i + 1].is_y_low) ||
                 (intersactions[scanline][i].is_y_high && intersactions[scanline][i + 1].is_y_high)))
            {
                if (x0 == -1)
                {
                    x0 = intersactions[scanline][i].x;
                    x1 = intersactions[scanline][i + 1].x;
                }
                i++;
            }
            else if (i < numberOfIntersactions[scanline] - 1 &&
                     ((intersactions[scanline][i].is_y_low && intersactions[scanline][i + 1].is_y_high) ||
                      (intersactions[scanline][i].is_y_high && intersactions[scanline][i + 1].is_y_low)))
            {
                if (x0 == -1)
                {
                    x0 = intersactions[scanline][i + 1].x;
                    i++;
                    continue;
                }

                if (x1 == -1)
                {
                    x1 = intersactions[scanline][i + 1].x;
                    i++;
                }
            }
            else if (x0 == -1)
            {
                x0 = intersactions[scanline][i].x;
                continue;
            }
            else if (x1 == -1)
            {
                x1 = intersactions[scanline][i].x;
            }

            // #####################################

            if (x0 != -1 && x1 != -1)
            {
                for (int x = x0; x <= x1; x++)
                {
                    if (coloring_mode == 0)
                    {
                        plot_pixel(x, scanline);
                    }
                    if (coloring_mode == 1 || coloring_mode == 2)
                    {
                        if (texture != NULL)
                        {
                            plot_texel(x, scanline, texture);
                        }
                        else
                        {
                            plot_pixel(x, scanline);
                        }
                    }
                }
                x0 = -1;
                x1 = -1;
            }
        }
        scanline--;
    }
}

void scanline(Poligon *poligon)
{
    clearIntersactionsBuffer();
    int borderCount;
    Border *borders = createBordersFromPoligon(poligon, &borderCount);
    // Scanline algorithm
    int scanline = RESOLUTION_HEIGHT;
    while (scanline >= 0)
    {
        setActiveBorders(borders, borderCount, scanline);
        calculateIntersections(borders, borderCount, scanline);
        scanline--;
    }
    // Order intersactions and add color to the poligon
    color_poligon(poligon);
    // Free resources
    clearIntersactionsBuffer();
    free(borders);
}

void mapWorldToFrameBuffer()
{
    clearFrameBuffer();
    clipping_universal_world(universal_world, x_min_window, y_min_window, x_max_window, y_max_window);
    for (int i = 0; i < universal_world->poligon_count; i++)
    {
        Poligon *poligon = &universal_world->poligon_array[i];
        if (poligon->is_out_of_window)
        {
            continue;
        }
        setColor(0, 0, 0);
        draw_lines(poligon);
        if (coloring_mode > -1)
        {
            scanline(poligon);
        }
    }
}

void redraw_screen()
{
    mapWorldToFrameBuffer(universal_world);
    render_signature();
    render_frame_buffer();
}

void translate(double dx, double dy)
{
    x_min_window += dx;
    x_max_window += dx;
    y_min_window += dy;
    y_max_window += dy;
}

void zoom(double scale)
{
    // Get center point respect to mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    double xC = x_min_window + (mouseX / (double)RESOLUTION_WIDTH) * (x_max_window - x_min_window);
    double yC = y_max_window + (mouseY / (double)RESOLUTION_HEIGHT) * (y_min_window - y_max_window);

    // Create transformation matrix
    double transform[3][3] = {
        {scale, 0, xC - (scale * xC)},
        {0, scale, yC - (scale * yC)},
        {0, 0, 1}};

    // Apply transformation to window bounds
    double corners[4][3] = {
        {x_min_window, y_min_window, 1},
        {x_max_window, y_min_window, 1},
        {x_min_window, y_max_window, 1},
        {x_max_window, y_max_window, 1}};

    for (int i = 0; i < 4; i++)
    {
        double x_new = transform[0][0] * corners[i][0] + transform[0][1] * corners[i][1] + transform[0][2] * corners[i][2];
        double y_new = transform[1][0] * corners[i][0] + transform[1][1] * corners[i][1] + transform[1][2] * corners[i][2];
        if (i == 0)
        {
            x_min_window = x_new * 1;
            y_min_window = y_new * 1;
        }
        else if (i == 1)
        {
            x_max_window = x_new * 1;
        }
        else if (i == 2)
        {
            y_max_window = y_new * 1;
        }
    }
}

void reset_universal_world()
{
    free(universal_world);
    universal_world = create_universal_world_from_file("car.txt");
}

void interact_key_up(SDL_Event event)
{
    switch (event.key.keysym.sym)
    {
    case SDLK_1: // Draw lines
        coloring_mode = -1;
        redraw_screen(universal_world);
        break;
    case SDLK_2: // Coloring with solid
        coloring_mode = 0;
        redraw_screen(universal_world);
        break;
    case SDLK_3: // Coloring with textures
        coloring_mode = 1;
        redraw_screen(universal_world);
        break;
    case SDLK_4: // Coloring with one texture
        coloring_mode = 2;
        redraw_screen(universal_world);
        break;
    case SDLK_r: // Rotate
        double xC = RESOLUTION_WIDTH / 2;
        double yC = RESOLUTION_HEIGHT / 2;
        rotate(universal_world, xC, yC, 45);
        redraw_screen(universal_world);
        break;
    case SDLK_F5: // Reset
        x_min_window = 0;
        y_min_window = 1080;
        x_max_window = 1920;
        y_max_window = 0;
        z_scale = 1;
        coloring_mode = -1;
        reset_universal_world(universal_world);
        redraw_screen(universal_world);
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    printf("Please interact with the program from the UI window \n\n");

    printf("---------------------------------------------------\n\n");

    // Read Universal world from car.txt

    universal_world = create_universal_world_from_file("car.txt");

    // Read Signature from my_signature.txt
    signature = create_signature_from_file("my_signature.txt");

    // Read Textures

    for (int i = 0; i < universal_world->poligon_count; i++)
    {
        char *texture_filepath = universal_world->poligon_array[i].texture_filepath;
        if (texture_filepath != NULL)
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

    // Load default texture
    texturesCache = (Texture **)realloc(texturesCache, (numTextures) * sizeof(Texture *));
    texturesCache[numTextures - 1] = create_texture_from_ppm(default_texture_filepath);

    printf("Universal world created\n");
    printf("Poligon count: %d\n", universal_world->poligon_count);
    printf("Vertex count: %d\n", universal_world->total_vertex_count);
    printf("---------------------------------------------------\n\n");
    printf("Press the following keys to switch modes:\n");
    printf("  - Press Key '1' to just draw lines.\n");
    printf("  - Press Key '2' to draw lines and coloring.\n");
    printf("  - Press Key '3' to draw lines and fill with different textures.\n");
    printf("  - Press Key '4' to draw lines and fill with  one texture.\n");
    printf("\n\n");
    printf("Additional Controls:\n");
    printf("  - Press Key F5 to reset the image to the initial state..\n");
    printf("\n");
    printf("  - Use the mouse wheel to zoom in or out.\n");
    printf("\n");
    printf("  - Click and drag the mouse to move the image to another position.\n");
    printf("\n");
    printf("  - Press 'r' key to rotate the image 45 degrees on clock direction.\n");
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

    // Init intersactions buffer
    intersactions = (IntersectionPoint **)malloc(RESOLUTION_HEIGHT * sizeof(IntersectionPoint *));
    if (intersactions == NULL)
    {
        fprintf(stderr, "\nError: Cannot reserve memory for the frame buffer.\n");
        exit(1);
    }
    numberOfIntersactions = (int *)calloc(RESOLUTION_HEIGHT, sizeof(int));

    initWindow();

    bool continue_program = true;
    SDL_Event event;
    int zoom_steps = 0;
    bool pan_started = false;
    bool processing = false;
    int pan_start_x, pan_start_y = 0;
    bool change_delta = false;
    bool did_zoom_in = false;
    bool did_zoom_out = false;

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
            // React to key_up event for mode and rotation
            if (event.type == SDL_KEYUP)
            {
                interact_key_up(event);
            }

            // React to mouse wheel event zooming
            if (event.type == SDL_MOUSEWHEEL)
            {
                if (event.wheel.y > 0)
                {
                    zoom_steps++;
                }
                else if (event.wheel.y < 0)
                {
                    zoom_steps--;
                }
                if (zoom_steps == 3)
                {
                    did_zoom_in = true;
                    if (did_zoom_out)
                    {
                        z_scale = (double)(z_scale - z_scale + 1);
                        did_zoom_out = false;
                    }
                    // Zoom in
                    zoom_steps = 0;
                    z_scale = (double)(z_scale - 0.05);
                    if (z_scale == 0)
                    {
                        z_scale = (double)(z_scale - 0.05);
                    }
                    zoom(z_scale);
                    redraw_screen(universal_world);
                }
                else if (zoom_steps == -3)
                {
                    did_zoom_out = true;
                    if (did_zoom_in)
                    {
                        z_scale = (double)(z_scale + z_scale + 1);
                        did_zoom_in = false;
                    }
                    // Zoom out
                    zoom_steps = 0;
                    z_scale = z_scale = (double)(z_scale + 0.05);
                    if (z_scale == 0)
                    {
                        z_scale = z_scale = (double)(z_scale + 0.05);
                    }
                    zoom(z_scale);
                    redraw_screen(universal_world);
                }
            }

            // React for mouse translation event pan
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && !pan_started)
            {
                pan_start_x = event.button.x;
                pan_start_y = event.button.y;
                pan_started = true;
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && pan_started)
            {
                pan_started = false;

                // Calculate distance
                int deltaX = pan_start_x - event.button.x;
                int deltaY = pan_start_y - event.button.y;

                pan_start_x = 0;
                pan_start_y = 0;

                if (deltaX != 0 || deltaY != 0)
                {
                    // Adjust deltaX and deltaY to keep same distance no matter the scale
                    deltaX = (deltaX / (double)RESOLUTION_WIDTH) * (x_max_window - x_min_window);
                    deltaY = (deltaY / (double)RESOLUTION_HEIGHT) * (y_min_window - y_max_window);

                    processing = true;
                    translate(deltaX, deltaY);
                    redraw_screen(universal_world);
                    processing = false;
                }
            }
            processing = false;
        }
    }

    // Free allocated memory

    for (int i = 0; i <= RESOLUTION_WIDTH; i++)
    {
        free(frameBuffer[i]);
    }
    free(frameBuffer);

    free(intersactions);
    free(numberOfIntersactions);

    for (int i = 0; i < numTextures; i++)
    {
        freeTextureMemory(texturesCache[i]);
    }
    free(texturesCache); 

    free_universal_world(universal_world);

    free_signature(signature);

    exit(0);
}