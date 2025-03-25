#include "seamcarving.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    // Allocate memory for the gradient image
    int height = im->height;
    int width = im->width;
    create_img(grad, height, width);
    // Calculate the energy of each pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int x_left = (x == 0) ? width - 1 : x - 1;
            int x_right = (x == width - 1) ? 0 : x + 1;
            int y_up = (y == 0) ? height - 1 : y - 1;
            int y_down = (y == height - 1) ? 0 : y + 1;

            int rx = get_pixel(im, y, x_right, 0) - get_pixel(im, y, x_left, 0);
            int gx = get_pixel(im, y, x_right, 1) - get_pixel(im, y, x_left, 1);
            int bx = get_pixel(im, y, x_right, 2) - get_pixel(im, y, x_left, 2);

            int ry = get_pixel(im, y_down, x, 0) - get_pixel(im, y_up, x, 0);
            int gy = get_pixel(im, y_down, x, 1) - get_pixel(im, y_up, x, 1);
            int by = get_pixel(im, y_down, x, 2) - get_pixel(im, y_up, x, 2);

            double energy = sqrt((double)(rx * rx + gx * gx + bx * bx + ry * ry + gy * gy + by * by));
            uint8_t scaled_energy = (uint8_t)(energy / 10);
            set_pixel(*grad, y, x, scaled_energy, scaled_energy, scaled_energy);
        }
    }
}  

void dynamic_seam(struct rgb_img *grad, double **best_arr) {
    // Allocate memory for the best_arr
    int height = grad->height;
    int width = grad->width;
    *best_arr = (double*) malloc(height * width * sizeof(double));
    // Initialize the first row of best_arr
    for (int x = 0; x < width; x++) {
        (*best_arr)[x] = get_pixel(grad, 0, x, 0);
    }
    // Calculate the rest of best_arr
    for (int y = 1; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double min = (*best_arr)[(y - 1) * width + x];
            if (x > 0 && (*best_arr)[(y - 1) * width + x - 1] < min) {
                min = (*best_arr)[(y - 1) * width + x - 1];
            }
            if (x < width - 1 && (*best_arr)[(y - 1) * width + x + 1] < min) {
                min = (*best_arr)[(y - 1) * width + x + 1];
            }
            (*best_arr)[y * width + x] = get_pixel(grad, y, x, 0) + min;
        }
    }
}

void recover_path(double *best, int height, int width, int **path) {
    // Allocate memory for the path
    *path = (int*) malloc(height * sizeof(int));
    // Find the smallest value from the last row
    double min = best[(height - 1) * width];
    (*path)[height - 1] = 0;
    for (int x = 1; x < width; x++) {
        if (best[(height - 1) * width + x] < min) {
            min = best[(height - 1) * width + x];
            (*path)[height - 1] = x;
        }
    }
    // Recover the rest of the path by working bottom up
    for (int y = height - 2; y >= 0; y--) {
        int x = (*path)[y + 1];
        int min_x = x;
        if (x > 0 && best[y * width + x - 1] < best[y * width + min_x]) {
            min_x = x - 1;
        }
        if (x < width - 1 && best[y * width + x + 1] < best[y * width + min_x]) {
            min_x = x + 1;
        }
        (*path)[y] = min_x;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {
    // Allocate memory for the destination image
    int height = src->height;
    int width = src->width;
    create_img(dest, height, width - 1);
    // Copy the source image to the destination without the pixels in the path
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 1; x++) {
            if (x < path[y]) {
                set_pixel(*dest, y, x, get_pixel(src, y, x, 0), get_pixel(src, y, x, 1), get_pixel(src, y, x, 2));
            } else {
                set_pixel(*dest, y, x, get_pixel(src, y, x + 1, 0), get_pixel(src, y, x + 1, 1), get_pixel(src, y, x + 1, 2));
            }
        }
    }
}
