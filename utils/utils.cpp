#include "utils.hpp"

string getFileName(string path)
{
    // cerr << "Entering getFileName.\n";
    string real_path = getRealPath(path);
    // cerr << "unreal_path: " << path << " became filename: " <<  string(basename((char *)real_path.c_str())) << endl;
    return string(basename((char *)real_path.c_str()));
}

string getDirName(string path)
{
    // cerr << "Entering getDirName.\n";
    string real_path = getRealPath(path);
    string unreal_dirname = string(dirname((char *)real_path.c_str()));
    string real_dirname = getRealPath(unreal_dirname);
    // cerr << "unreal_path: " << path << " became dirname: " << real_dirname << endl;
    return real_dirname;
}

string getRealPath(string path)
{
    // cerr << "Entering getRealPath.\n";
    char unreal_path[path.length() + 1];
    char real_path[PATH_MAX + 1];
    strcpy(unreal_path, path.c_str());
    realpath(unreal_path, real_path);
    // cerr << "unreal_path: " << path << " became real_path: " << real_path << endl;
    return string(real_path);
}

void writeFileToDisk(string content, string file_name)
{
    // cerr << "writeFileToDisk is writing " << file_name << " with size " << content.size() << endl;
    ofstream file_on_disk(file_name);
    for (int i = 0; i < content.size(); i++) {
        file_on_disk << content[i];
    }
    file_on_disk.close();
    return;
}

string readFileFromDisk(string file_name)
{
    // cerr << "Reading " << file_name << " from disk.\n";
    ifstream file_on_disk(file_name, ifstream::in);
    if (!file_on_disk) {
        // cerr << "ERROR: Could not open " << file_name << endl;
    }
    string file = "";
    while (file_on_disk.good()) {
        file += file_on_disk.get();
    }
    file_on_disk.close();
    file.pop_back();
    return file;
}

vector<string> *getAllFilesInDir(string dir_path)
{
    DIR *dir;
    struct dirent *current_file;
    vector<string> *file_names = new vector<string>();
    if (NULL != (dir = opendir(dir_path.c_str()))) {
        // cerr << "Printing all files in: " << dir_path << endl;
        while ((current_file = readdir(dir)) != NULL) {
            // Don't include current or parent directories
            if (!strcmp(current_file->d_name, ".")) {
                continue;
            }
            if (!strcmp(current_file->d_name, "..")) {
                continue;
            }
            file_names->push_back(getRealPath(dir_path + "/" + string(current_file->d_name)));
            // cerr << getRealPath(dir_path + "/" + string(current_file->d_name)) << endl;
        }
        closedir(dir);
        // cerr << "Done printing files in " << dir_path << endl;
    } else {
        // unable to open dir_path
        // cerr << "Unable to open " << dir_path << endl;
        exit(1);
    }
    return file_names;
}

