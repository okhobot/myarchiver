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


    struct myfile
    {
        std::string name;
        bool isdir;
    };

    struct file_info
    {
        long local_index;
        long dindex;
        long dsize;
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

    std::vector<BYTE> file_to_bytes(std::string filePath)
    {
        std::vector<BYTE> fileBuf;

        long prev,fileSize;

        FILE *file = NULL;
        if ((file = fopen(filePath.c_str(), "rb")) == NULL)
        {
            if(debug)std::cout << "Failed to load file \""<<filePath<<"\""<<std::endl;
            return fileBuf;
        }


        prev=ftell(file);
        fseek(file, 0L, SEEK_END);
        fileSize =ftell(file);
        fseek(file,prev,SEEK_SET);

        fileBuf .resize(fileSize);
        if(debug)std::cout << " " << fileSize<<std::endl;
        fread(fileBuf.data(), fileSize, 1, file);
        fclose(file);

        return fileBuf;
    }

    void bytes_to_file(std::vector<BYTE>fileBuf, std::string filePath)
    {
        FILE* file_copy = fopen( filePath.c_str(), "wb" );
        fwrite( fileBuf.data(), 1, fileBuf.size(), file_copy);
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

    file_info find_same_data(std::vector<BYTE> &data, std::vector<BYTE> &file_data, long min_data_size=25)
    {
        return {-1,-1,0};

        long same_data_size=1;
        //std::cout<<"Q"<<std::endl;
        for(int i=0;i<file_data.size();i++)
        {
            //if(i%10000==0)std::cout<<i<<"/"<<file_data.size()<<std::endl;
            for(int j=0;j<data.size();j++)
            {
                //if(j>17240000&&j%10000==0)std::cout<<j<<std::endl;
                //if(i%10000==0&&j%10000==0)std::cout<<i<<"/"<<file_data.size()<<" | "<<j<<"/"<<data.size()<<std::endl;
                if(file_data[i]==data[j])
                {
                    same_data_size=1;
                    //if(i%10000==0&&j%10000==0)std::cout<<"qq"<<std::endl;
                    while(i+same_data_size<file_data.size()&&j+same_data_size<data.size()&&file_data[i+same_data_size]==data[j+same_data_size])same_data_size++;
                    //if(i%10000==0&&j%10000==0)std::cout<<i<<"/"<<file_data.size()<<" | "<<j<<"/"<<data.size()<<std::endl;
                    if(same_data_size>=min_data_size)
                    {
                        if(debug)std::cout<<"found: "<<same_data_size<<std::endl;
                        return {i,j,same_data_size};
                    }
                }
            }
        }
        //std::cout<<"done"<<std::endl;
        return {-1,-1,0};
    }

    std::vector<file_info> add_file_to_data(std::vector<BYTE> &data, std::vector<BYTE> file_data)
    {
        std::vector<file_info> res_file_data;

        if(data.size()==0&&false)
        {
            res_file_data.push_back({0,0,file_data.size()/2});
            data.insert(data.end(),file_data.begin(),file_data.begin()+file_data.size()/2);
            file_data.erase(file_data.begin(),file_data.begin()+file_data.size()/2);
        }

        file_info same_data_info=find_same_data(data,file_data);
        while(same_data_info.local_index!=-1&&file_data.size()>0)
        {
            res_file_data.push_back({0,data.size(),same_data_info.local_index});
            data.insert(data.end(),file_data.begin(),file_data.begin()+same_data_info.local_index);
            res_file_data.push_back({0,same_data_info.dindex,same_data_info.dsize});
            file_data.erase(file_data.begin(),file_data.begin()+same_data_info.local_index+same_data_info.dsize);
            same_data_info=find_same_data(data,file_data);
        }
        res_file_data.push_back({0,data.size(),file_data.size()});
        data.insert(data.end(),file_data.begin(),file_data.end());
        file_data.clear();

        return res_file_data;
    }


    void generate_data(std::string base_path_load, std::string data_name)
    {
        std::vector<file_info> file_data;
        std::vector<std::string> out_files_data;
        std::vector<BYTE> data;

        std::vector<myfile> folder_data=read_folder(base_path_load);

        std::stringstream ss;
        if(debug)std::cout<<"\n";

        for(int i=0; i<folder_data.size(); i++)
        {
            if(debug)std::cout<<folder_data[i].name<<" ";

            ss<<folder_data[i].name<<std::endl;
            if(folder_data[i].isdir)
            {
                ss<<0<<std::endl;
                if(debug)std::cout<<0<<std::endl;
            }
            else //odata<<file_to_bin(base_path_load+folder_data[i].name)<<std::endl;
            {
                file_data=add_file_to_data(data, file_to_bytes(base_path_load+folder_data[i].name));
                ss<<file_data.size()<<" ";
                for(int i=0;i<file_data.size();i++)ss<<file_data[i].dindex<<" "<<file_data[i].dsize<<std::endl;

            }
            out_files_data.push_back(ss.str());
            ss.str("");
        }
        std::ofstream odata(data_name, std::ios::binary);
        odata<<folder_data.size()<<" "<<data.size()<<std::endl;
        for(int i=0;i<data.size();i++)odata<<data[i];
        odata<<std::endl;
        for(int i=0;i<out_files_data.size();i++)
        {
            odata<<out_files_data[i];
        }
        odata.close();
    }

    void generate_directory(std::string base_path_save, std::string data_name)
    {
        long data_size;
        std::vector<char> data;
        std::vector<BYTE> file_data;
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
        long files_count,file_data_size;
        file_info fi;
        std::string fname;
        idata>>files_count>>data_size;
        if(debug)std::cout<<"files_count: "<<files_count<<std::endl;

        data.resize(data_size);
        idata.get();
        idata.read(data.data(), data_size);

        for(int i=0; i<files_count; i++)
        {
            fname="";
            getline(idata,fname);
            while(fname=="")
                getline(idata,fname);
            idata>>file_data_size;
            if(debug)std::cout<<fname<<"; is directory: "<<(file_data_size==0)<<std::endl;
            fname=base_path_save+fname;
            if(file_data_size==0)CreateDirectoryA (fname.c_str(), NULL);
            else
            {
                file_data.clear();
                for(int i=0;i<file_data_size;i++)
                {
                    idata>>fi.dindex>>fi.dsize;
                    file_data.insert(file_data.end(),data.begin()+fi.dindex,data.begin()+fi.dindex+fi.dsize);
                }
                bytes_to_file(file_data,fname);
            }

            //else bin_to_data(idata,file_size,fname);
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
    bool run=true;

    achiver.generate_data("./0/","data.ok");
    if(debug)std::cout<<std::endl;
    achiver.generate_directory("./test/","data.ok");
    std::cout<<"done"<<std::endl<<std::endl;


    while(run)
    {
        std::cin>>com;
        run=(com=="setup"||com=="delete"||com=="help");
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
                achiver.generate_directory(path,data_name);
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

        std::cout<<"done"<<std::endl<<std::endl;

    }
    return 0;
}
