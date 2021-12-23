
/**
* Copyright (C) 2020 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

#include "xcl2.hpp"
#include <algorithm>
#include <vector>
#include <stdio.h>
#include "ap_int.h"

//#define DATA_SIZE 4096

#define	NTRU_DATA_SIZE 512
#define DATA unsigned short
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string binaryFile = argv[1];
    cl_int err;
    cl::Context context;
    cl::Kernel krnl_NTRU;
    cl::CommandQueue q;
    // Allocate Memory in Host Memory
    // When creating a buffer with user pointer (CL_MEM_USE_HOST_PTR), under the
    // hood user ptr
    // is used if it is properly aligned. when not aligned, runtime had no choice
    // but to create
    // its own host side buffer. So it is recommended to use this allocator if
    // user wish to
    // create buffer using CL_MEM_USE_HOST_PTR to align user buffer to page
    // boundary. It will
    // ensure that user buffer is used when user create Buffer/Mem object with
    // CL_MEM_USE_HOST_PTR
//    std::vector<int, aligned_allocator<int> > source_in1(DATA_SIZE);
//    std::vector<int, aligned_allocator<int> > source_in2(DATA_SIZE);
//    std::vector<int, aligned_allocator<int> > source_hw_results(DATA_SIZE);
//   std::vector<int, aligned_allocator<int> > source_sw_results(DATA_SIZE);

//    // Create the test data
//    std::generate(source_in1.begin(), source_in1.end(), std::rand);
//    std::generate(source_in2.begin(), source_in2.end(), std::rand);
//    for (int i = 0; i < DATA_SIZE; i++) {
//        source_sw_results[i] = source_in1[i] + source_in2[i];
//        source_hw_results[i] = 0;
//    }

////////////////////////////////////////////////////////////////////////////////

    std::vector<DATA, aligned_allocator<DATA> > input_h(NTRU_DATA_SIZE); // type : short. & has 'NTRU_DATA_SIZE' arrays.
    std::vector<DATA, aligned_allocator<DATA> > input_r(NTRU_DATA_SIZE);
    std::vector<DATA, aligned_allocator<DATA> > input_m(NTRU_DATA_SIZE);
    std::vector<DATA, aligned_allocator<DATA> > output_c(NTRU_DATA_SIZE);
    std::vector<ap_uint<11>, aligned_allocator<ap_uint<11>> > modular_sw(NTRU_DATA_SIZE);
	FILE *fpgold, *fp, *fin1, *fin2, *fin3;
	fin1	= fopen("input_h.dat", "r");
	fin2	= fopen("input_r.dat", "r");
	fin3	= fopen("input_m.dat", "r");
	fpgold	= fopen("output_gold.dat", "w");
	fp		= fopen("output_c.dat", "w");
	if (fin1 == NULL) {
		printf("wrong\n");
	}
	int	i;
		for	(i=0; i<NTRU_DATA_SIZE; i++)	{
			fscanf(fin1, "%hu", &input_h[i]);
			fscanf(fin2, "%hu", &input_r[i]);
			fscanf(fin3, "%hu", &input_m[i]);
		}

		std::vector<DATA, aligned_allocator<DATA> > gold_out(NTRU_DATA_SIZE);
		for (i=0; i<512; i++) {
			gold_out[i] = input_m[i];
		}
/*
		for(int k = 0; k < 509; k++) {
			for (int i=1; i<509 -k; i++) {
				gold_out[k] += input_r[k+i] * input_h[509 - i];
			}
			for (int i=0; i<k+1; i++)
				gold_out[k] += input_r[k-i] * input_h[i];
		}
*/
   for(int j = 0; j < 509; j++) {
		i_loop: for(int i = 0; i < 509; i++) {
			if (input_r[(509 -i) % 509] == 2) {
				gold_out[j] += -input_h[(j+i) % 509]; // Can we use % operator?
			}
			else if (input_r[(509 -i) % 509] == 0) {
				gold_out[j] += 0;
			}
			else if (input_r[(509 -i) % 509] == 1){
				gold_out[j] += input_h[(j+i) % 509];
			}
		}
	}
		printf("\n");
		for (i=0; i<512; i++) {
			modular_sw[i] = gold_out[i];
			gold_out[i] = modular_sw[i];
			fprintf(fpgold, "%hu ", gold_out[i]);
		}
/////////////////////////////////////////////////////////////////////////////////

    // OPENCL HOST CODE AREA START
    // get_xil_devices() is a utility API which will find the xilinx
    // platforms and will return list of devices connected to Xilinx platform
    auto devices = xcl::get_xil_devices();
    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err, krnl_NTRU = cl::Kernel(program, "NTRU", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    // Allocate Buffer in Global Memory
    // Buffers are allocated using CL_MEM_USE_HOST_PTR for efficient memory and
    // Device-to-host communication


    OCL_CHECK(err, cl::Buffer buffer_input1(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,	sizeof(DATA)*NTRU_DATA_SIZE, input_h.data(),		&err));
    OCL_CHECK(err, cl::Buffer buffer_input2(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,	sizeof(DATA)*NTRU_DATA_SIZE, input_r.data(),		&err));
    OCL_CHECK(err, cl::Buffer buffer_input3(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,	sizeof(DATA)*NTRU_DATA_SIZE, input_m.data(), 	&err));
    OCL_CHECK(err, cl::Buffer buffer_output(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY,	sizeof(DATA)*NTRU_DATA_SIZE, output_c.data(),	&err));

    int size = NTRU_DATA_SIZE;
//    int size = DATA_SIZE;

	OCL_CHECK(err, err = krnl_NTRU.setArg(0, buffer_input1));
    OCL_CHECK(err, err = krnl_NTRU.setArg(1, buffer_input2));
    OCL_CHECK(err, err = krnl_NTRU.setArg(2, buffer_input3));
    OCL_CHECK(err, err = krnl_NTRU.setArg(3, buffer_output));
    OCL_CHECK(err, err = krnl_NTRU.setArg(4, size));

    // Copy input data to device global memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_input1, buffer_input2, buffer_input3}, 0 /* 0 means from host*/));
	q.finish();
	auto	start = std::chrono::steady_clock::now();
//=================================================================
    OCL_CHECK(err, err = q.enqueueTask(krnl_NTRU));
//=================================================================
	q.finish();

	auto end = std::chrono::steady_clock::now();
	double	exec_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	printf("FPGA kernel exec time is %f sec\n", exec_time*1e-9);

    // Launch the Kernel
    // For HLS kernels global and local size is always (1,1,1). So, it is
    // recommended
    // to always use enqueueTask() for invoking HLS kernel

    // Copy Result from Device Global Memory to Host Local Memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output}, CL_MIGRATE_MEM_OBJECT_HOST));
    q.finish();


	int err_cnt = 0;
	for(int i = 0; i<NTRU_DATA_SIZE-3; i++){
		fprintf(fp, "%hu ", output_c[i]);
		if(gold_out[i] != output_c[i]) {
			err_cnt++;
			if( err_cnt == 1 ){
				printf("i:%d gold:%d hw:%d\n", i, gold_out[i], output_c[i] );
			}
		}
	}


	if(err_cnt != 0){
		printf("FAILED! Error count : %d\n", err_cnt);
	}
	else{
		printf("PASSED!\n");
	}

	return EXIT_SUCCESS;




}
