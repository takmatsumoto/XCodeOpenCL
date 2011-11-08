//
//  main.cpp
//  OpenCLTest2
//
//  Created by  on 2011/11/7.
//  Copyright (c) 2011年 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <fstream.h>
#include <stdio.h>
#include <vector.h>

#include "HeadFileDef.h"

using namespace std;

cl_program loadProgram(cl_context context, const char* filename, cl_device_id* devices);

int main (int argc, const char * argv[])
{

    // insert code here..OpenCL
    std::cout << "Hello, OpenCL!\n";
    
    // 取得裝置資訊
    cl_context clContext;
//    char* deviceName;
//  cl_device_id* devices;
    size_t cb;
    
    // create GPU context
    clContext = clCreateContextFromType(NULL, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);
    if (clContext==0) {
        printf("Can't create GPU context\n");
        return 0;
    }
    
    
    cl_int err = 0;
    cl_uint num = 0;
    
    // 先取得platform的數量
    err = clGetPlatformIDs(0, 0, &num);
    if (err != CL_SUCCESS) {
        printf("Unable to get platforms");
        return 0;
    }
    
//    cl_platform_id* platforms = (cl_platform_id*)malloc(2);
    std::vector<cl_platform_id> platforms(num);
    err = clGetPlatformIDs(num, &platforms[0], &num);
    if(err != CL_SUCCESS) {
        std::cerr << "Unable to get platform ID\n";
        return 0;
    }
    
    // 建立屬性
    cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0 };
    
    // 依照建立的屬性去create一個裝置
    clContext = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
    if (clContext==0) {
        printf("Can not create OpenCL context");
    }
    
    std::string strDeviceName;
    
    // get list of device
    clGetContextInfo(clContext, CL_CONTEXT_DEVICES, 0, NULL, &cb);
//    devices = (cl_device_id*)malloc(cb);
    std::vector<cl_device_id> devices(cb/sizeof(cl_device_id));
    clGetContextInfo(clContext, CL_CONTEXT_DEVICES, cb, &devices[0], 0);
    
    // show the name of the first device
    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
//    deviceName = (char*)malloc(cb);
    std::string deviceName;
    deviceName.resize(cb);
    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &deviceName[0], 0);
    printf("My Device : %s\n",deviceName.c_str());
    
    cl_command_queue clCommandQueue = clCreateCommandQueue(clContext, devices[0], 0, 0);
    if (clCommandQueue==0) {
        std::cerr << "Can't create command queue\n";
        clReleaseContext(clContext);
        return 0;
    }
    
    
    // 亂數產生測試資料
    const int DATA_SIZE = 1048576;
    std::vector<float> a(DATA_SIZE), b(DATA_SIZE), res(DATA_SIZE);
    for (int i=0; i<DATA_SIZE; i++) {
        a[i] = std::rand()%1000;
        b[i] = std::rand()%1000;
    }
    
    // 配製記憶體並複製資料
    cl_mem_flags clFlag = CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR;
    size_t size = sizeof(cl_float)*DATA_SIZE;
    cl_mem clA = clCreateBuffer(clContext, clFlag, size, &a[0], NULL);
    cl_mem clB = clCreateBuffer(clContext, clFlag, size, &b[0], NULL);
    cl_mem clRes = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY, size, NULL, NULL);
    if (clA==0 || clB==0 || clRes==0) {
        std::cerr << "Can't create OpenCL buffer\n";
        clReleaseMemObject(clA);
        clReleaseMemObject(clB);
        clReleaseMemObject(clRes);
        clReleaseCommandQueue(clCommandQueue);
        clReleaseContext(clContext);
    }
    
    
    cl_program clProgram = loadProgram(clContext, "/shader.cl",&devices[0]);
    if (clProgram==0) {
        std::cerr << "Can't load or build program";
        clReleaseMemObject(clA);
        clReleaseMemObject(clB);
        clReleaseMemObject(clRes);
        clReleaseProgram(clProgram);
        clReleaseCommandQueue(clCommandQueue);
        clReleaseContext(clContext);
        return 0;
    }
    
    cl_kernel clKernel = clCreateKernel(clProgram, "addr", &err);
    if (clKernel==0) {
        std::cerr << "Can't load kernel : " << err << "\n";
        switch (err) {
            case CL_INVALID_PROGRAM:
                std::cerr<<"program is not a valid program object";
                break;
            case CL_INVALID_PROGRAM_EXECUTABLE:
                std::cerr<<"there is no successfully built executable for program";
                break;
            case CL_INVALID_KERNEL_NAME:
                std::cerr<<"kernel_name is not found in program";
                break;
            case CL_INVALID_KERNEL_DEFINITION:
                std::cerr<<"the function definition for __kernel function given by kernel_name such as the number of arguments, the argument types are not the same for all devices for which the program executable has been built";
                break;
            case CL_INVALID_VALUE:
                std::cerr<<"kernel_name is NULL";
                break;
            case CL_OUT_OF_RESOURCES:
                std::cerr<<"there is a failure to allocate resources required by the OpenCL implementation on the device";
                break;
            case CL_OUT_OF_HOST_MEMORY:
                std::cerr<<"there is a failure to allocate resources required by the OpenCL implementation on the host.";
                break;
            default:
                std::cerr<<"unknown error";
                break;
        }
        clReleaseProgram(clProgram);
        clReleaseMemObject(clA);
        clReleaseMemObject(clB);
        clReleaseMemObject(clRes);
        clReleaseCommandQueue(clCommandQueue);
        clReleaseContext(clContext);
        return 0;
    }
    
    clSetKernelArg(clKernel, 0, sizeof(cl_mem), &clA);
    clSetKernelArg(clKernel, 1, sizeof(cl_mem), &clB);
    clSetKernelArg(clKernel, 2, sizeof(cl_mem), &clRes);
    size_t work_size = DATA_SIZE;
    err = clEnqueueNDRangeKernel(clCommandQueue, clKernel, 1, 0, &work_size, 0, 0, 0, 0);
    if (err==CL_SUCCESS) {
        err = clEnqueueReadBuffer(clCommandQueue, clRes, CL_TRUE, 0, sizeof(float)*DATA_SIZE, &res[0], 0, 0, 0);
    }
    
    if(err == CL_SUCCESS) {
        bool correct = true;
        for(int i = 0; i < DATA_SIZE; i++) {
            if(a[i] + b[i] != res[i]) {
                correct = false;
                break;
            }
//            std::cout<< "Res["<<i<<"] : "<<res[i]<<"\n";
//            printf("Res[%07d] : %f\n",i,res[i]);
        }
        
        if(correct) {
            std::cout << "Data is correct\n";
        }
        else {
            std::cout << "Data is incorrect\n";
        }
    }
    else {
        std::cerr << "Can't run kernel or read back data\n";
    }
    
    
    // 將裝置,command queue, program釋放掉
    clReleaseKernel(clKernel);
	clReleaseProgram(clProgram);
	clReleaseMemObject(clA);
	clReleaseMemObject(clB);
	clReleaseMemObject(clRes);
	clReleaseCommandQueue(clCommandQueue);
	clReleaseContext(clContext);
    

    return 0;
}

cl_program loadProgram(cl_context context, const char* filename, cl_device_id* devices)
{
    std::ifstream ifs ( filename , ios_base::binary );
    
    // get file lenght
    if (!ifs.good()) {
        return 0;
    }
    
    ifs.seekg(0, std::ios_base::end);
    size_t length = ifs.tellg();
    ifs.seekg(0, std::ios_base::beg);
    
    std::vector<char> data(length+1);
    ifs.read(&data[0], length);
    data[length]=0;
    
    // create build program
    cl_int err = 0;
    const char* source = &data[0];
    cl_program program = clCreateProgramWithSource(context, 1, &source, 0, &err);
    if (program==0) {
        return 0;
    }
    
    if(clBuildProgram(program, 1, devices, 0, 0, 0)!=CL_SUCCESS) {
        return 0;
    }
    
    return program;
}


