//
//  main.cpp
//  OpenCLTest2
//
//  Created by  on 2011/11/7.
//  Copyright (c) 2011年 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

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
    
    clReleaseContext(clContext);
    

    return 0;
}

