#include <fstream>
#include <iostream>
#include <string>
#include <ios>
#include <vector>
#include <CL/cl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
typedef struct
{
    uint8_t R=0;
    uint8_t G=0;
    uint8_t B=0;
    uint8_t align;
} RGB;

typedef struct
{
    bool type;
    uint32_t size;
    uint32_t height;
    uint32_t weight;
    RGB *data;
} Image;

cl_program loadProgram(cl_context context, const char* filename)
{
        std::ifstream in(filename, std::ios_base::binary);
        if(!in.good()) {
                return 0;
        }

        // get file length
        in.seekg(0, std::ios_base::end);
        size_t length = in.tellg();
        in.seekg(0, std::ios_base::beg);

        // read program source
        std::vector<char> data(length + 1);
        in.read(&data[0], length);
        data[length] = 0;

        // create and build program
        const char* source = &data[0];
        cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);
        if(program == 0) {
                return 0;
        }

        if(clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) {
                return 0;
        }

        return program;


}


/*unsigned int *readbmp(const char *filename,int &weight,int &height,int &input_size)
{
    std::ifstream bmp(filename, std::ios::binary);
    char header[54];
    bmp.read(header, 54);
    uint32_t size = *(int *)&header[2];
    uint32_t offset = *(int *)&header[10];
    uint32_t w = *(int *)&header[18];
    uint32_t h = *(int *)&header[22];
	unsigned int *image= new unsigned int[w*h*3];
    uint16_t depth = *(uint16_t *)&header[28];
    if (depth != 24 && depth != 32)
    {
        printf("we don't suppot depth with %d\n", depth);
        exit(0);
    }
    bmp.seekg(offset, bmp.beg);
	weight = w;
	height = h;
	input_size = w*h;
    Image *ret = new Image();
    ret->type = 1;
    ret->height = h;
    ret->weight = w;
    ret->size = w * h;
    ret->data = new RGB[w * h]{};
	
	double START,END;
    START = clock();
    for (int i = 0; i < ret->size; i++)
    {
        bmp.read((char *)&ret->data[i], depth / 8);
		image[3*i] = ret->data[i].R;
		image[3*i+1] = ret->data[i].G;
		image[3*i+2] = ret->data[i].B;

    }
	END = clock();
	cout << endl << "程式執行所花費：" << (END - START)/CLOCKS_PER_SEC << " S" ;
	
    return image;
}*/

unsigned char *readbmp(const char *filename,int &weight,int &height,int &input_size){
	FILE* f = fopen(filename, "rb");
	unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	weight = *(int*)&info[18];
    height = *(int*)&info[22];
	input_size = weight * height;
	int size = 4 * weight * height;
	unsigned char* data = new unsigned char[size];
	fread(data, sizeof(unsigned char), size, f);
	fclose(f);

    return data;
}

int writebmp(const char *filename, Image *img)
{

    uint8_t header[54] = {
        0x42,        // identity : B
        0x4d,        // identity : M
        0, 0, 0, 0,  // file size
        0, 0,        // reserved1
        0, 0,        // reserved2
        54, 0, 0, 0, // RGB data offset
        40, 0, 0, 0, // struct BITMAPINFOHEADER size
        0, 0, 0, 0,  // bmp width
        0, 0, 0, 0,  // bmp height
        1, 0,        // planes
        32, 0,       // bit per pixel
        0, 0, 0, 0,  // compression
        0, 0, 0, 0,  // data size
        0, 0, 0, 0,  // h resolution
        0, 0, 0, 0,  // v resolution
        0, 0, 0, 0,  // used colors
        0, 0, 0, 0   // important colors
    };

    // file size
    uint32_t file_size = img->size * 4 + 54;
    header[2] = (unsigned char)(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    // width
    uint32_t width = img->weight;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    // height
    uint32_t height = img->height;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    std::ofstream fout;
    fout.open(filename, std::ios::binary);
    fout.write((char *)header, 54);
    fout.write((char *)img->data, img->size * 4);
    fout.close();
}




int main(int argc, char *argv[])
{
    char *filename;
	
    if (argc >= 2)
    {			
		cl_int err;
		cl_uint num;
		err = clGetPlatformIDs(0, 0, &num);
		if(err != CL_SUCCESS) {

			std::cerr << "Unable to get platforms\n";

			return 0;

		}		
		std::vector<cl_platform_id> platforms(num);
		err = clGetPlatformIDs(num, &platforms[0], &num);
		if(err != CL_SUCCESS) {

			std::cerr << "Unable to get platform ID\n";

			return 0;

		}
		
		
		cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0 };

		cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);

		if(context == 0) {

			std::cerr << "Can't create OpenCL context\n";

			return 0;

		}
		
		size_t cb;
		clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
		std::vector<cl_device_id> devices(cb / sizeof(cl_device_id));
		clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);
		clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
		std::string devname;
		devname.resize(cb);
		clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);
		std::cout << "Device: " << devname.c_str() << "\n";
		cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, 0);
		if(queue == 0) {

			std::cerr << "Can't create command queue\n";

			clReleaseContext(context);

			return 0;

		}

		const int DATA_SIZE = 256;								
		int many_img = argc - 1;
		
        for (int i = 0; i < many_img; i++)
        {
			
			
            filename = argv[i + 1];
         
			
			int img_weight,img_height,input_size;
			unsigned char *image;
			image=readbmp(filename,img_weight,img_height,input_size);
			std::cout << img_weight << ":" << img_height <<"\n";
						
											
			cl_mem image_rgb = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned char) * input_size * 4, &image[0], NULL);
			cl_mem historgram_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned int) * 256 * 3, NULL, NULL);
			
			if(image_rgb == 0 || historgram_res == 0) {
				std::cerr << "Can't create OpenCL buffer\n";
				clReleaseMemObject(image_rgb);
				clReleaseMemObject(historgram_res);
				clReleaseCommandQueue(queue);
				clReleaseContext(context);
				return 0;
			}

			cl_program program = loadProgram(context, "histogram.cl");
			if(program == 0) {
				std::cerr << "Can't load or build program\n";
				clReleaseMemObject(image_rgb);
				clReleaseMemObject(historgram_res);
				clReleaseCommandQueue(queue);
				clReleaseContext(context);

				return 0;
			}
			
			cl_kernel histogram = clCreateKernel(program, "histogram", 0);
			if(histogram == 0) {
				std::cerr << "Can't load kernel\n";
				clReleaseProgram(program);
				clReleaseMemObject(image_rgb);
				clReleaseMemObject(historgram_res);
				clReleaseCommandQueue(queue);
				clReleaseContext(context);
				return 0;
			}
			
			clSetKernelArg(histogram, 0, sizeof(cl_mem), &image_rgb);
			clSetKernelArg(histogram, 1, sizeof(cl_mem), &historgram_res);					
			size_t work_size = input_size;
			std::vector<unsigned int> res(256 * 3);
			err = clEnqueueNDRangeKernel(queue, histogram, 1, 0, &work_size, 0, 0, 0, 0);
			if(err == CL_SUCCESS) {
				err = clEnqueueReadBuffer(queue, historgram_res, CL_TRUE, 0, sizeof(unsigned int) * 256*3, &res[0], 0, 0, 0);
				if(err != CL_SUCCESS) {
					std::cerr << "Enqueue kernel command error! \n";	
				}					
			}			
			if(i==many_img-1){
				clReleaseMemObject(image_rgb);
				clReleaseMemObject(historgram_res);
				clReleaseProgram(program);
				clReleaseKernel(histogram);
				clReleaseContext(context);
				clReleaseCommandQueue(queue);
			}
																
			
			int max = 0;
			for(int i=0;i<256;i++){
				max = res[i] > max ? res[i] : max;
				max = res[256+i] > max ? res[256+i] : max;
				max = res[512+i] > max ? res[512+i] : max;				
			}
			
			Image *ret = new Image();
			ret->type = 1;
			ret->height = 256;
			ret->weight = 256;
			ret->size = 256 * 256;
			ret->data = new RGB[256 * 256];

			for(int i=0;i<256;i++){
				for(int j=0;j<256;j++){
					
					if(res[j]*256/max > i){
					ret->data[256*i+j].R = 255;}
					if(res[256+j]*256/max > i){
					ret->data[256*i+j].G = 255;}
					if(res[512+j]*256/max > i){
					ret->data[256*i+j].B = 255;}			
				}				
			}
			std::string newfile = "hist_" + std::string(filename); 
			writebmp(newfile.c_str(), ret);				
        }
    }else{
        printf("Usage: ./hist <img.bmp> [img2.bmp ...]\n");
    }
    return 0;
}
