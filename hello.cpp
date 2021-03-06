#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

// Compute c = a + b.
static const char source[] =
    "#if defined(cl_khr_fp64)\n"
    "#  pragma OPENCL EXTENSION cl_khr_fp64: enable\n"
    "#elif defined(cl_amd_fp64)\n"
    "#  pragma OPENCL EXTENSION cl_amd_fp64: enable\n"
    "#else\n"
    "#  error double precision is not supported\n"
    "#endif\n"
    "kernel void add(\n"
    "       ulong n,\n"
    "       global const double *a,\n"
    "       global const double *b,\n"
    "       global double *c\n"
    "       )\n"
    "{\n"
    "    size_t i = get_global_id(0);\n"
    "    if (i < n) {\n"
    "       c[i] = a[i] + b[i];\n"
    "    }\n"
    "}\n";



int main() {
    const size_t N = (1 << 8) + 250 ;

    std:: cout << "N is " <<  N << std::endl;
    bool profile=true;
    bool block = false;
    cl_int  status = 0;
    int  tmp = profile & status;
    std::cout << tmp << std::endl;
    size_t source_size = 0;
    ifstream in("./test.cl", ios::in);

    istreambuf_iterator<char> start(in), end;
    string sources (start, end);

    if(sources.empty()) {
        std::cout << "opencl source files is empty\n" << std::endl;
    }
    source_size = sources.size();
    std::cout << source_size << std::endl;

    in.close();


    try {
	// Get list of OpenCL platforms.
	std::vector<cl::Platform> platform;
	cl::Platform::get(&platform);

	if (platform.empty()) {
	    std::cerr << "OpenCL platforms not found." << std::endl;
	    return 1;
	}

	// Get first available GPU device which supports double precision.
	cl::Context context;
	std::vector<cl::Device> device;
	for(auto p = platform.begin();  p != platform.end(); p++) {
	    std::vector<cl::Device> pldev;

	    try {
		p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

		for(auto d = pldev.begin();  d != pldev.end(); d++) {
		    if (!d->getInfo<CL_DEVICE_AVAILABLE>()) continue;

		    std::string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();

		    if (
			    ext.find("cl_khr_fp64") == std::string::npos &&
			    ext.find("cl_amd_fp64") == std::string::npos
		       ) continue;

		    device.push_back(*d);
		    context = cl::Context(device);
		}
	    } catch(...) {
		device.clear();
	    }
	}

	if (device.empty()) {
	    std::cerr << "GPUs with double precision not found." << std::endl;
	    return 1;
	}

	std::cout << "Found " << device.size() << " devices" << std::endl;
	std::cout << device[0].getInfo<CL_DEVICE_NAME>() << std::endl;
	std::cout << device[1].getInfo<CL_DEVICE_NAME>() << std::endl;

	// Create command queue.
	cl::CommandQueue queue(context, device[1]);

	//cl_program  _program = clCreateProgramWithSource((*_pContext), 1, (const char **)&source, &src_size, &_status);



	// Compile OpenCL program for found device.
	/*cl::Program program(context, cl::Program::Sources(
		    1, std::make_pair(source, strlen(source))
		    ));*/

	cl::Program program(context, cl::Program::Sources(
           1, std::make_pair(sources.c_str(), sources.size())
    ));

	try {
	    program.build(device);
	} catch (const cl::Error&) {
	    std::cerr
		<< "OpenCL compilation error" << std::endl
		<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[1])
		<< std::endl;
	    return 1;
	}

	cl::Kernel add(program, "add");
	cl::Kernel readInfo(program, "readInfo");

	// Prepare input data.
	std::vector<double> a(N, 1);
	std::vector<double> b(N, 2);
	std::vector<double> c(N);
	std::size_t info;
	size_t global_id =0.f;


	// Allocate device buffers and transfer input data to device.
	cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		a.size() * sizeof(double), a.data());

	cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		b.size() * sizeof(double), b.data());

	cl::Buffer C(context, CL_MEM_READ_WRITE,
		c.size() * sizeof(double));


	cl::Buffer Info(context, CL_MEM_READ_WRITE, sizeof(info));
    std::cout << "sizeof(info) is " << sizeof(info) << std::endl;

    cl::Buffer Global_id(context, CL_MEM_READ_WRITE,  sizeof(size_t));

	// Set kernel parameters.
	add.setArg(0, static_cast<cl_ulong>(N));
	add.setArg(1, A);
	add.setArg(2, B);
	add.setArg(3, C);
	add.setArg(4, Global_id);

	readInfo.setArg(0, Info);
	
	// Launch kernel on the compute device.
	queue.enqueueNDRangeKernel(add, cl::NullRange, N, cl::NullRange);
    queue.enqueueNDRangeKernel(readInfo,cl::NullRange,1, cl::NullRange);

    size_t group_size ;
    size_t ret;

    clGetKernelWorkGroupInfo(add.operator()(), device[0].operator()(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),&group_size, &ret);


	// Get result back to host.
	queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());
	queue.enqueueReadBuffer(Global_id, CL_TRUE,0, sizeof(size_t) , &global_id);

    queue.enqueueReadBuffer(Info, CL_TRUE,0, sizeof(info) , &info);




	// Should get '3' here.
	std::cout << c[42] << std::endl;
	std::cout << "info is :" << info << std::endl;
    std::cout << "global_id is :" << global_id << std::endl;

    } catch (const cl::Error &err) {
	std::cerr
	    << "OpenCL error: "
	    << err.what() << "(" << err.err() << ")"
	    << std::endl;
	return 1;
    }
}
