#include "seamcarving.c"
#include <stdio.h>
#include <stdlib.h>

int main() {
    struct rgb_img *grad;
  
    struct rgb_img *im;
    read_in_img(&im, "3x4.bin");

    calc_energy(im,  &grad);
    
    print_grad(grad);
    return 0;
}