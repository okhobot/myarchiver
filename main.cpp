#include <iostream>
#include <fstream>
#include<vector>
#include <dirent.h>
#include <sstream>
#include <algorithm>
#define  _WIN32_WINNT  0x0600
#include <Windows.h>
#define debug 1

class Archiver
{

public:
    BYTE *fileBuf;

    struct myfile
    {
        std::string name;
        bool isdir;
    };

    std::string myreplace(std::string s, std::string from, std::string to)
    {
        size_t pos=s.find(from);
        while(pos!=std::string::npos)
        {
            s.replace(pos,from.size(),to);
            pos+=to.size();
            pos=s.find(from,pos);
        }
        return s;
    }

    std::string file_to_bin(std::string filePath)
    {
        std::stringstream data;

        long prev,fileSize;

        FILE *file = NULL;
        if ((file = fopen(filePath.c_str(), "rb")) == NULL)
        {
            if(debug)std::cout << "Failed to load file \""<<filePath<<"\""<<std::endl;
            return "0\n";
        }


        prev=ftell(file);
        fseek(file, 0L, SEEK_END);
        fileSize =ftell(file);
        fseek(file,prev,SEEK_SET);

        fileBuf = new BYTE[fileSize];
        if(debug)std::cout << " " << fileSize<<std::endl;
        fread(fileBuf, fileSize, 1, file);
        fclose(file);

        data<<fileSize<<std::endl;
        for(int i=0; i<fileSize; i++)
        {
            data<<fileBuf[i];
            //if(i<10)if(debug)cout<<(int)fileBuf[i]<<" ";
        }


        //if(debug)cout<<endl;
        delete[] fileBuf;
        return data.str();
    }

    void bin_to_data(std::ifstream &data, long fileSize, std::string filePath)
    {

        char *tmp=new char[fileSize];

        if(debug)std::cout<<"loading_file: "<<filePath<<std::endl;

        fileBuf = new BYTE[fileSize];
        data.get();
        data.read(tmp, fileSize);

        for(int i=0; i<fileSize; i++)
        {
            fileBuf[i]=tmp[i];
            //if(i<10)if(debug)cout<<(int)fileBuf[i]<<" ";
        }
        //if(debug)cout<<endl;

        FILE* file_copy = fopen( filePath.c_str(), "wb" );
        fwrite( fileBuf, 1, fileSize, file_copy);
        fclose(file_copy);
    }

    std::vector<myfile> read_folder(std::string start_path, std::string filename="")
    {

        std::vector<myfile> files(0);
        DIR *dpdf;
        struct dirent *epdf;

        dpdf = opendir((start_path+filename).c_str());
        if (dpdf != NULL)
        {

            for (int i=0; epdf = readdir(dpdf); i++)
            {
                if(i>=2)
                {
                    std::vector<myfile> sub_dir=read_folder(start_path,filename+epdf->d_name+"/");
                    files.push_back({filename+epdf->d_name,sub_dir.size()!=0});
                    files.insert(files.end(),sub_dir.begin(),sub_dir.end());
                    if(debug)std::cout << epdf->d_name<<" "<<sub_dir.size()<<std::endl;
                }

            }
        }
        closedir(dpdf);

        return files;
    }

    void generate_data(std::string base_path_load, std::string data_name)
    {
        std::vector<myfile> folder_data=read_folder(base_path_load);
        if(debug)std::cout<<"\n";

        std::ofstream odata(data_name, std::ios::binary);
        odata<<folder_data.size()<<std::endl;
        for(int i=0; i<folder_data.size(); i++)
        {
            if(debug)std::cout<<folder_data[i].name<<" ";
            odata<<folder_data[i].name<<std::endl;
            if(folder_data[i].isdir)
            {
                odata<<0<<std::endl;
                if(debug)std::cout<<0<<std::endl;
            }
            else odata<<file_to_bin(base_path_load+folder_data[i].name)<<std::endl;
        }
        odata.close();
    }

    void generate_folder(std::string base_path_save, std::string data_name)
    {
        if(base_path_save=="-/")
        {
            base_path_save=data_name;
            base_path_save.erase(base_path_save.begin()+base_path_save.rfind('.'),base_path_save.end());
            base_path_save+="/";
            //base_path_save=myreplace(data_name,".ok","/");
            if(debug)std::cout<<"saving to "<<base_path_save<<std::endl;
        }
        if(GetFileAttributesA(base_path_save.c_str())==INVALID_FILE_ATTRIBUTES)CreateDirectoryA (base_path_save.c_str(), NULL);

        std::ifstream idata(data_name, std::ios::binary);
        long file_count,file_size;
        std::string fname;
        idata>>file_count;
        if(debug)std::cout<<"file_count: "<<file_count<<std::endl;
        for(int i=0; i<file_count; i++)
        {
            fname="";
            getline(idata,fname);
            while(fname=="")
                getline(idata,fname);
            idata>>file_size;
            if(debug)std::cout<<fname<<"; is directory: "<<(file_size==0)<<std::endl;
            fname=base_path_save+fname;

            if(file_size==0)CreateDirectoryA (fname.c_str(), NULL);
            else bin_to_data(idata,file_size,fname);
        }
        idata.close();
    }


    void check_path(std::string &path)
    {
        path=myreplace(path,"\\","/");
        if(path[path.size()-1]!='/')
        {
            if(debug)std::cout<<"warning: the path must end in \"/\""<<std::endl;
            path+="/";
        }
    }

    std::string get_exe_path()
    {
        char result[ MAX_PATH ];
        return std::string( result, GetModuleFileName( NULL, result, MAX_PATH ) );
    }
    void check_registry_errors(long errorc)
    {
        if(errorc!=ERROR_SUCCESS)
        {
            if(debug)std::cout<<"error in registry: "<<errorc<<std::endl;
            return;
        }
    }
    void setup()
    {
        HKEY hKey;
        std::string data;
        std::string exe_path=get_exe_path();

        std::ofstream bat("open.bat");
        bat<<"echo load %1 -| "<<exe_path<<std::endl;
        bat.close();



        //open the registry with key_write access
        check_registry_errors (RegCreateKeyExW (HKEY_CLASSES_ROOT,L"*\\shell\\openWithMyarchiver",0,L"",REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,NULL));
        if(hKey)
        {
            data="Открыть с помощью myarchiver";
            check_registry_errors (RegSetValueEx (hKey, "MUIVerb", 0, REG_SZ, (LPBYTE)data.c_str(), strlen(data.c_str())));
            RegCloseKey(hKey);
        }

        check_registry_errors (RegCreateKeyExW (HKEY_CLASSES_ROOT,L"*\\shell\\openWithMyarchiver\\command",0,L"",REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,NULL));
        if(hKey)
        {
            data=myreplace(exe_path,"archiver.exe","open.bat")+" %1";
            check_registry_errors (RegSetValueEx (hKey, "", 0, REG_SZ, (LPBYTE)data.c_str(), strlen(data.c_str())));
            RegCloseKey(hKey);
        }





        check_registry_errors (RegCreateKeyExW (HKEY_CLASSES_ROOT,L"Folder\\shell\\archiveWithMyarchiver",0,L"",REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,NULL));
        if(hKey)
        {
            data="Архивировать с помощью myarchiver";
            check_registry_errors (RegSetValueEx (hKey, "MUIVerb", 0, REG_SZ, (LPBYTE)data.c_str(), strlen(data.c_str())));
            RegCloseKey(hKey);
        }

        check_registry_errors (RegCreateKeyExW (HKEY_CLASSES_ROOT,L"Folder\\shell\\archiveWithMyarchiver\\command",0,L"",REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,NULL));
        if(hKey)
        {

            data="cmd /c \"cd /d %1 &&echo save %1.ng %1| "+exe_path+" \"";
            check_registry_errors (RegSetValueEx (hKey, "", 0, REG_SZ, (LPBYTE)data.c_str(), strlen(data.c_str())));
            RegCloseKey(hKey);
        }


    }

    void delete_from_registry()
    {
        check_registry_errors (RegDeleteTreeW (HKEY_CLASSES_ROOT,L"*\\shell\\openWithMyarchiver"));//RegDeleteTree

        check_registry_errors (RegDeleteTreeW (HKEY_CLASSES_ROOT,L"Folder\\shell\\archiveWithMyarchiver"));
    }
};



int main()
{
    setlocale(1251,"Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    //std::cout<<"проверка языка"<<std::endl;

    Archiver achiver;
    std::string com,path, data_name;
    //achiver.generate_data("./0/","data.ok");
    //if(debug)std::cout<<std::endl;
    //achiver.generate_folder("./t/","data.ok");


    ///while(true)
    {
        std::cin>>com;
        if(com=="save"||com=="load")
        {
            std::cin>>data_name;
            std::cin.get();
            getline(std::cin, path);
            if(debug)std::cout<<path<<std::endl;
            achiver.check_path(path);

            if(com=="save")
                achiver.generate_data(path,data_name);
            else if(com=="load")
                achiver.generate_folder(path,data_name);
        }
        else if(com=="setup")
            achiver.setup();
        else if(com=="delete")
            achiver.delete_from_registry();
        else if(com=="help")
        {
            const char* msg =
            "Usage: myarchiver.exe [args]: \n"
            "|\thelp   THE ALL MIGHTY HELP\n"
            "|\tsetup   Setup the program to the registry\n"
            "|\tdelete Uninstall the program from the registry\n"
            "|\tsave [Archive name] [path to save from] Load to archive\n"
            "|\tload [Archive name] [path to save to] Load from archive\n"
            ;
            std::cout<<msg<<std::endl;
        }

        if(debug)std::cout<<"done"<<std::endl<<std::endl;

    }
    return 0;
}
