#include <stdio.h>
#include <stdlib.h>
#include "c_img.h"
#include "seamcarving.h"

double sqr_root(double square) //helper function 

{
    double root = square / 3;
    int i;

    if (square <= 0)return 0;

    for (i = 0; i < 32; i++)

        root = (root + square / root) / 2;

    return root;
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad)
{
    create_img(grad, im->height, im->width);
    //printf("%ld %ld", im->width, im->height);

    for(int i = 0; i < im->width; i++)
    {
        for(int j = 0; j < im->height; j++)
        {
            //printf("%d\n", dx_r);
            int x1 = (i + 1) % (im->width);
            int x2 = (i - 1 + im->width) % (im->width);
            
            int y1 = (j + 1) % (im->height);
            int y2 = (j - 1 + im->height) % (im->height);
            //printf("x1: %d, x2: %d\n", y1, y2);

            int dx_r = get_pixel(im, j, x1, 0) - get_pixel(im, j, x2, 0);
            int dx_g = get_pixel(im, j, x1, 1) - get_pixel(im, j, x2, 1);
            int dx_b = get_pixel(im, j, x1, 2) - get_pixel(im, j, x2, 2);

            int dy_r = get_pixel(im, y1, i, 0) - get_pixel(im, y2, i, 0);
            int dy_g = get_pixel(im, y1, i, 1) - get_pixel(im, y2, i, 1);
            int dy_b = get_pixel(im, y1, i, 2) - get_pixel(im, y2, i, 2);

            int x_energy = (dx_r * dx_r) + (dx_g * dx_g) + (dx_b * dx_b);
            int y_energy = (dy_r * dy_r) + (dy_g * dy_g) + (dy_b * dy_b); 

            double dual_energy = sqr_root((double)(x_energy + y_energy));
            double grad_10 = (dual_energy) / 10;
            uint8_t energy = (uint8_t) grad_10;

            set_pixel(*grad, j, i, energy, energy, energy);

        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr)
{
    *best_arr = (double *)malloc(sizeof(double) * grad->height * grad->width);

    if (*best_arr == NULL)
    {
        return;
    } 

    for(int j = 0; j < grad->width; j++)
    {
        (*best_arr)[j] = get_pixel(grad, 0, j, 0); //copy each pixel from the gradient (first row) into tbe best array 
    }

    for(int i = 1; i < grad->height; i++) //skip first row 
    {
        for(int j = 0; j < grad->width; j++)
        {
            int left; //declaring left right and center to make my life easier
            if(j - 1 < 0)
            {
                left = 0;
            }
            else
            {
                left = j - 1;
            }

            int mid = j;

            int right;
            if(j + 1 > grad->width)
            {
                right = grad->width;
            }
            else
            {
                right = j + 1;
            }

            double min_val = (*best_arr)[(i - 1) * grad->width + mid]; //set min val to index directly behind the target
            if ((*best_arr)[(i - 1) * grad->width + left] < min_val)
            {
                min_val = (*best_arr)[(i - 1) * grad->width + left]; //min is left
            }

            if ((*best_arr)[(i - 1) * grad->width + right] < min_val)
            {
                min_val = (*best_arr)[(i - 1) * grad->width + right]; //min is right
            }

            (*best_arr)[i * grad->width + j] = (double)get_pixel(grad, i, j, 0) + min_val; //add pixel to next row
        }
    }

}

void recover_path(double *best, int height, int width, int **path)
{
    *path = (int *)malloc(height * sizeof(int));

    for (int y = 0; y < height; y++)
    {
        int cur_best = 0;
        int cur_best_index = 0;
        for (int x = 0; x < width; x++)
        {
            int cur_index = y * width + x;

            if (x == 0){
            cur_best = best[cur_index];
            cur_best_index = x;
            }

            else
            {
                if (cur_best > best[cur_index])
                {
                    cur_best = best[cur_index];
                    cur_best_index = x;
                }
            }  
        }
        (*path)[y] = cur_best_index;

    }

}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path)
{
    create_img(dest, src->height, src->width - 1);

    for (int y = 0; y < src->height; y++)
    {
        for (int x = 0; x <src->width;x++)
        {
            if (x < path[y])
            {
                int r = get_pixel(src, y, x, 0);
                int g = get_pixel(src, y, x, 1);
                int b = get_pixel(src, y, x, 2);
                set_pixel(*dest, y, x, r, g, b);
            }

            else if (x >path[y])
            {
                int r = get_pixel(src, y, x, 0);
                int g = get_pixel(src, y, x, 1);
                int b = get_pixel(src, y, x, 2);
                set_pixel(*dest, y, x - 1, r, g, b);
            }

        }
    }

}


// int main()
// {
//     struct rgb_img *im;
//     struct rgb_img *grad;
//     //create_img(&im, 4, 3);
//     //read_in_img(&im, "3x4.bin");
//     //calc_energy(im,  &grad);
//     //print_grad(grad);

//     create_img(&im, 6, 5);
//     read_in_img(&im, "6x5.bin");
//     calc_energy(im, &grad);
//     print_grad(grad);
//     double *best_arr;
//     dynamic_seam(grad, &best_arr);
//     for (int i = 0; i < grad->height; i++)
//     {
//         for (int j = 0; j < grad->width; j++)
//         {
//             printf("%.0f   ", best_arr[i * grad->width + j]);
//         }
        
//         printf("\n");
//     }
    

// }

