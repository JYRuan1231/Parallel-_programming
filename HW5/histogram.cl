__kernel void histogram(__global const unsigned char* image_rgb,__global unsigned int* histogram_res)
{
        int idx = get_global_id(0);
        atomic_inc(&histogram_res[image_rgb[3*idx]]);
		atomic_inc(&histogram_res[256 + (image_rgb[3*idx+1])]);
		atomic_inc(&histogram_res[256*2 + (image_rgb[3*idx+2])]);
}