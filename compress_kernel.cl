#include <file_info.h>

__kernel void compress_kernel(
    __global const unsigned char *data,
    __global const unsigned char *file_data,
    __global file_info *file_info_arr,
    int data_size,
    int file_data_size,
    int min_data_size
)
{


    long same_data_size;
    int i = get_global_id(0);
    if(i>=file_data_size)return;


    //printf("%ld %ld %ld \n",file_info_arr[i].local_index,file_info_arr[i].dindex,file_info_arr[i].dsize);

    for(int j=0;j<data_size;j++)
    {
        if(file_data[i]==data[j])
        {
            same_data_size=1;
            while(i+same_data_size<file_data_size&&j+same_data_size<data_size&&file_data[i+same_data_size]==data[j+same_data_size])same_data_size++;
            if(same_data_size>=min_data_size)
            {

                file_info_arr[i].local_index= i;
                file_info_arr[i].dindex= j;
                file_info_arr[i].dsize= same_data_size;//
                j+=same_data_size;
            }
        }
    }

}
