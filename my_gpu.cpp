#include "my_gpu.hpp"


using namespace std;

void GPU::operator = (GPU &_gpu)
{
        gpu_queue=_gpu.gpu_queue;
        context=_gpu.context;
        contextDevices=_gpu.contextDevices;
        kernels=_gpu.kernels;
        kernel=_gpu.kernel;
        sourceCode=_gpu.sourceCode;
        source=_gpu.source;
        program=_gpu.program;
        iArg=_gpu.iArg;
}

void GPU::init_gpu(vector<string> kernel_names)
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    std::vector<cl::Device> devices;

    //vector<string> kernel_names= {"test_kernel", "fcl_predict_gpu", "fcl_calculate_error_main_lay_gpu", "fcl_calculate_error_gpu",
    //                              "cnn_predict_gpu", "cnn_calculate_error_main_lay_gpu", "cnn_calculate_error_gpu", "fcl_train_weights_gpu", "cnn_train_weights_gpu", "process_error_gpu", "set_errors_gpu"
    //                            };
    /*cout<<"device_names: ";
    for(int i=0;i<platforms.size();i++)
    {
        devices.clear();
        platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
        for(int j=0;j<devices.size();j++)
        cout<<devices[j].getInfo<CL_DEVICE_NAME>()<<"; ";
    }
    cout<<endl<<"USING_GPU"<<endl;*/



    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    cl::Device device=devices[0];

    cout<<"GPU_name: "<<device.getInfo<CL_DEVICE_NAME>()<<endl;
    contextDevices.push_back(device);
    cout<<"device initialized"<<endl;
    context=cl::Context(contextDevices);
    cout<<"context initialized"<<endl;
    //For the selected device create a context and command queue
    gpu_queue=cl::CommandQueue(context, device);


    cout<<"gpu_queue initialized"<<endl;

    for(int i=0; i<kernel_names.size(); i++)
    {
        if(console_logs)cout<<"initializing the kernel: "<<kernel_names[i]<<endl;
        sourceFile.open((kernel_names[i]+".cl"));
        sourceCode=string(std::istreambuf_iterator<char>(sourceFile),(std::istreambuf_iterator<char>()));
        source= cl::Program::Sources(1, std::make_pair(sourceCode.c_str(), sourceCode.length()+1));
        program = cl::Program(context, source);
        program.build(contextDevices);
        kernels[kernel_names[i]]=cl::Kernel(program, kernel_names[i].c_str());
        sourceFile.close();
    }
    cout<<"done"<<endl<<endl;

}


void GPU::add_variable(string key, cl_mem_flags mem_flag, size_t bufsize)
{
    variables[key]=cl::Buffer(context, mem_flag, bufsize);
}
cl::Buffer* GPU::get_variable(string key)
{
    return &variables[key];
}
void GPU::set_variable(string key, cl::Buffer* variable)
{
    variables[key]=*variable;
}


void GPU::write_variable(string key, size_t bufsize, vector<custom_type>&data)
{
    gpu_queue.enqueueWriteBuffer(variables[key], CL_TRUE, 0, bufsize, data.data());
}
void GPU::write_variable(string key, size_t bufsize, vector<int>&data)
{
    gpu_queue.enqueueWriteBuffer(variables[key], CL_TRUE, 0, bufsize, data.data());
}
void GPU::write_variable(string key, size_t bufsize, vector<float>&data)
{
    gpu_queue.enqueueWriteBuffer(variables[key], CL_TRUE, 0, bufsize, data.data());
}
void GPU::write_variable(string key, size_t bufsize, vector<unsigned char>&data)
{
    gpu_queue.enqueueWriteBuffer(variables[key], CL_TRUE, 0, bufsize, data.data());
}
void GPU::write_variable(string key, size_t bufsize, string &data)
{
    gpu_queue.enqueueWriteBuffer(variables[key], CL_TRUE, 0, bufsize, data.c_str());
}

void GPU::read_variable(string key, size_t bufsize, vector<custom_type> &data)
{
    gpu_queue.enqueueReadBuffer(variables[key], CL_TRUE, 0, bufsize, data.data());
}
void GPU::read_variable(string key, size_t bufsize, vector<int> &data)
{
    gpu_queue.enqueueReadBuffer(variables[key], CL_TRUE, 0, bufsize, data.data());
}
void GPU::read_variable(string key, size_t bufsize, vector<float> &data)
{
    gpu_queue.enqueueReadBuffer(variables[key], CL_TRUE, 0, bufsize, data.data());
}
void GPU::read_variable(string key, size_t bufsize, vector<unsigned char> &data)
{
    gpu_queue.enqueueReadBuffer(variables[key], CL_TRUE, 0, bufsize, data.data());
}

void GPU::process_gpu(string kernel_name, vector<string> variable_names, vector<float> floats, vector<int> ints, int s1, int s2, int s3)
{
    iArg = 0;
    if(kernels.find(kernel_name)==kernels.end())
    {
        cout<<"process_gpu - kernel not found: "<<kernel_name<<endl;
        return;
    }
    kernel=kernels[kernel_name];

    for(int i=0; i<variable_names.size(); i++)
    {
        if(variables.find(variable_names[i])==variables.end())cout<<"process_gpu - null variable error: "<<kernel_name<<endl;//call_error(1,"process_gpu","null variable error: ",variable_names[i]);
        kernel.setArg(iArg++, variables[variable_names[i]]);
    }
    for(int i=0; i<floats.size(); i++)kernel.setArg(iArg++, floats[i]);
    for(int i=0; i<ints.size(); i++)kernel.setArg(iArg++, ints[i]);

    if(s2==0)gpu_queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(s1));
    else if(s3==0)gpu_queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(s1,s2));
    else gpu_queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(s1,s2,s3));
    gpu_queue.finish();
}






