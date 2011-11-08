//
//  shader.c
//  OpenCLTest2
//
//  Created by  on 2011/11/7.
//  Copyright (c) 2011年 __MyCompanyName__. All rights reserved.
//

__kernel void addr (__global const float* a, __global const float* b, __global float* result) 
{
    int idx = get_global_id(0);
    result[idx] = a[idx] + b[idx];
}
