// #include <sys/types.h>

#include <string.h>
#include <fstream>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <cstring>
#include <libgen.h>
#include <vector>
#include "sha256.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using std::cerr;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::endl;
using std::string;
using std::vector;

string getFileName(string path);
string getDirName(string path);
string getRealPath(string path);
string readFileFromDisk(string file_name);
vector<string> *getAllFilesInDir(string dir_path);
void writeFileToDisk(string content, string file_name);
