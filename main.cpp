#include <iostream>
#include <fstream>
#include<vector>
#include<windows.h>
#include <dirent.h>
#include <sstream>
#include <algorithm>
using namespace std;
BYTE *fileBuf;

struct myfile
{
    string name;
    bool isdir;
};

string myreplace(string s, char from, char to)
{
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == from)
            s[i] = to;
    }
    return s;
}

string file_to_bin(string filePath)
{
    stringstream data;

    long prev,fileSize;

    FILE *file = NULL;
    if ((file = fopen(filePath.c_str(), "rb")) == NULL)
    {
        cout << "Failed to load file \""<<filePath<<"\""<<endl;
        return "0\n";
    }


    prev=ftell(file);
    fseek(file, 0L, SEEK_END);
    fileSize =ftell(file);
    fseek(file,prev,SEEK_SET);

    fileBuf = new BYTE[fileSize];
    cout << " " << fileSize<<endl;
    fread(fileBuf, fileSize, 1, file);
    fclose(file);

    data<<fileSize<<endl;
    for(int i=0;i<fileSize;i++)
    {
        data<<fileBuf[i];
        //if(i<10)cout<<(int)fileBuf[i]<<" ";
    }


        //cout<<endl;
    delete[] fileBuf;
    return data.str();
}

void bin_to_data(ifstream &data, long fileSize, string filePath)
{

    char *tmp=new char[fileSize];

    cout<<"loading_file: "<<filePath<<endl;

    fileBuf = new BYTE[fileSize];
    data.get();
    data.read(tmp, fileSize);

    for(int i=0;i<fileSize;i++)
    {
        fileBuf[i]=tmp[i];
        //if(i<10)cout<<(int)fileBuf[i]<<" ";
    }
    //cout<<endl;

    FILE* file_copy = fopen( filePath.c_str(), "wb" );
    fwrite( fileBuf, 1, fileSize, file_copy);
    fclose(file_copy);
}

vector<myfile> read_folder(string start_path, string filename="")
{

    vector<myfile> files(0);
    DIR *dpdf;
    struct dirent *epdf;

    dpdf = opendir((start_path+filename).c_str());
    if (dpdf != NULL){

        for (int i=0;epdf = readdir(dpdf);i++){
            if(i>=2)
            {
                vector<myfile> sub_dir=read_folder(start_path,filename+epdf->d_name+"/");
                files.push_back({filename+epdf->d_name,sub_dir.size()!=0});
                files.insert(files.end(),sub_dir.begin(),sub_dir.end());
                cout << epdf->d_name<<" "<<sub_dir.size()<<endl;
            }

        }
    }
    closedir(dpdf);

    return files;
}

void generate_data(string base_path_load, string data_name)
{
    vector<myfile> folder_data=read_folder(base_path_load);
    cout<<"\n";

    ofstream odata(data_name, ios::binary);
    odata<<folder_data.size()<<endl;
    for(int i=0;i<folder_data.size();i++)
    {
        cout<<folder_data[i].name<<" ";
        odata<<folder_data[i].name<<endl;
        if(folder_data[i].isdir){odata<<0<<endl;cout<<0<<endl;}
        else odata<<file_to_bin(base_path_load+folder_data[i].name)<<endl;
    }
    odata.close();
}

void generate_folder(string base_path_save, string data_name)
{
    ifstream idata(data_name, ios::binary);
    long file_count,file_size;
    string fname;
    idata>>file_count;
    cout<<"file_count: "<<file_count<<endl;
    for(int i=0;i<file_count;i++)
    {
        fname="";
        getline(idata,fname);
        while(fname=="")
        getline(idata,fname);
        idata>>file_size;
        cout<<fname<<"; is directory: "<<(file_size==0)<<endl;
        fname=base_path_save+fname;

        if(file_size==0)CreateDirectory (fname.c_str(), NULL);
        else bin_to_data(idata,file_size,fname);
    }
    idata.close();
}







int main()
{
    setlocale(1251,"Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    string com,path, data_name;

    //generate_data("./0/","d.ok");
    //cout<<endl;
    //generate_folder("./t/","d.ok");

    while(true)
    {
        cin>>com>>data_name;
        cin.get();
        getline(cin, path);
        cout<<path<<endl;
        path=myreplace(path,'\\','/');
        if(path[path.size()-1]!='/')
        {
            cout<<"warning: the path must end in \"/\""<<endl;
            path+="/";
        }

        if(com=="save")
            generate_data(path,data_name);
        else if(com=="load")
            generate_folder(path,data_name);
        cout<<"done"<<endl<<endl;
    }

}
