#include <iostream>
#include <fstream>
#include <sstream>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include "kernel.h"

std::vector<cl::Platform> pl;
std::vector<cl::Device> devs;
cl::Context ctx;
cl::CommandQueue q;
cl::Program prog;
cl::Kernel ker;
cl::Buffer b_input, b_output;
int current_device;

void SetupDevice(unsigned int ip, unsigned int id);
void LoadOpenCLKernel(const char *kernelfile);

int main(int narg, char **argv)
{
  // select platform and device
  if (narg != 3) {
    SetupDevice(0, 0);
  } else {
    SetupDevice(atoi(argv[1]), atoi(argv[2]));
  }

  // load kernel string
  LoadOpenCLKernel(kernel_str);

  // select a main kernel function
  ker = cl::Kernel(prog, "templateKernel");

  // allocate two OpenCL buffers
  b_output = cl::Buffer(ctx, CL_MEM_READ_WRITE, 1024*sizeof(cl_int));
  b_input  = cl::Buffer(ctx, CL_MEM_READ_WRITE, 1024*sizeof(cl_int));


  // map arguments
  ker.setArg(0, b_output);
  ker.setArg(1, b_input);

  // set "multiplier"
  unsigned int cc = 100;
  ker.setArg(2, (unsigned int)cc);

  // write input data
  unsigned int in[1024];
  for(int i = 0; i < 1024; i++) {
    in[i] = i;
  }
  q.enqueueWriteBuffer(b_input, CL_TRUE, 0, 1024*sizeof(cl_int), in);

  // execute the kernel
  cl::Event event;
  q.enqueueNDRangeKernel(ker, cl::NullRange, cl::NDRange(1024), cl::NDRange(64), NULL, &event);

  // read output data
  unsigned int out[1024];
  q.enqueueReadBuffer(b_output, CL_TRUE, 0, 1024*sizeof(cl_int), out);

  // verify results
  for(int i = 0; i < 16; i++) {
    std::cout << in[i] << "\t" << out[i] << "\n"; 
  }
}

void SetupDevice(unsigned int ip, unsigned int id)
{
  try {
    cl::Platform::get(&pl);

    for(unsigned int i = 0; i < pl.size(); i++) {
      std::cerr << "platform " << i << " " << pl[i].getInfo<CL_PLATFORM_NAME>().c_str() << " "
	      << pl[i].getInfo<CL_PLATFORM_VERSION>().c_str() << "\n";    
      pl[i].getDevices(CL_DEVICE_TYPE_ALL, &devs);
      for(unsigned int j = 0; j < devs.size(); j++) {
	std::cerr << "\tdevice " << j << " " << devs[j].getInfo<CL_DEVICE_NAME>().c_str() << "\n";
      }
    }
    std::cerr << "\n"; 

    if (pl.size() <= ip) throw cl::Error(-1, "FATAL: the specifed platform does not exist");
    std::cerr << pl[ip].getInfo<CL_PLATFORM_NAME>().c_str() << " "
	      << pl[ip].getInfo<CL_PLATFORM_VERSION>().c_str() << "::";    

    pl[ip].getDevices(CL_DEVICE_TYPE_ALL, &devs);
    if (devs.size() <= id) throw cl::Error(-1, "FATAL: the specifed device does not exist");
    std::cerr << devs[id].getInfo<CL_DEVICE_NAME>().c_str() << "\n";

    ctx = cl::Context(devs);
    q = cl::CommandQueue(ctx, devs[id], CL_QUEUE_PROFILING_ENABLE);
  }  catch( cl::Error e ) {
    std::cerr << e.what() << ":" << e.err() << "\n";
    std::cerr << "Abort!\n";
    exit(-1);
  }
  current_device = id;
}

void LoadOpenCLKernel(const char *kernelfile) 
{
  try {
    cl::Program::Sources src(1, std::make_pair(kernelfile, strlen(kernelfile)));
    prog = cl::Program(ctx, src);

    std::stringstream options;
    options << "  -D__DUMMY_BUILD_OPTIONS__  ";
    std::cerr << "Build options :: " << options.str() << "\n";
    prog.build(devs, options.str().c_str());
  }  catch( cl::Error e ) {
    std::string log = prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devs[current_device]);
    std::cerr << e.what() << ":" << e.err() << "\n";
    std::cerr << kernelfile << "\n";
    std::cerr << log << "\n";
    exit(-1);
  }
}
