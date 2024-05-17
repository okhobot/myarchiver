#ifndef MY_GPU_HPP_
#define MY_GPU_HPP_

#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#define console_logs 1
#include <file_info.h>
#define custom_type file_info
using namespace std;

class GPU
{
private:
    map<string, cl::Buffer> variables;
    map<string,cl::Kernel> kernels;

    vector<cl::Device> contextDevices;

    cl::CommandQueue gpu_queue;
    cl::Context context;
    cl::Kernel kernel;
    cl::Program::Sources source;
    cl::Program program;

    std::ifstream sourceFile;
    std::string sourceCode;

    int iArg;

public:

    void init_gpu(vector<string> kernel_names);

    void operator = (GPU &_gpu);


    void add_variable(string key, cl_mem_flags mem_flag, size_t bufsize);
    cl::Buffer* get_variable(string key);
    void set_variable(string key,cl::Buffer* variable);


    void write_variable(string key, size_t bufsize, vector<custom_type> &data);
    void write_variable(string key, size_t bufsize, vector<int> &data);
    void write_variable(string key, size_t bufsize, vector<float> &data);
    void write_variable(string key, size_t bufsize, vector<unsigned char> &data);
    void write_variable(string key, size_t bufsize, string &data);

    void read_variable(string key, size_t bufsize, vector<custom_type> &data);
    void read_variable(string key, size_t bufsize, vector<int> &data);
    void read_variable(string key, size_t bufsize, vector<float> &data);
    void read_variable(string key, size_t bufsize, vector<unsigned char> &data);

    void process_gpu(string kernel_name, vector<string> variable_names, vector<float> floats, vector<int> ints, int s1, int s2=0, int s3=0);


};
#endif
