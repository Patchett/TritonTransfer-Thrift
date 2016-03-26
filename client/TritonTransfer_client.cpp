#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "./gen-cpp/TritonTransfer.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <unordered_map>
#include <stdio.h>
// #include <unistd.h>
// #include <sys/types.h>

#include "../utils/utils.hpp"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using boost::shared_ptr;
using namespace  ::no::podcasts::no::learning;
using std::unordered_map;

typedef struct ServerAttributes {
    int port;
    string server_name;
    TritonTransferClient *server;
    boost::shared_ptr<TTransport> transport;
} ServerAttributesStruct;

static SHA256 sha256;
static const int MAX_BLOCK_SIZE = 16000;
static unordered_map<string, string> *hash_to_block;
static unordered_map<string, vector<string> *> *file_names_to_hashes;
static string file_path;
static string static_file_name;
static string path_and_file_name;
static string md_server_name;
static int md_port;
static boost::shared_ptr<TTransport> transport_static;

void cleanupServerConnection(ServerAttributesStruct *server_attribs);
void bootstrapClient();
bool handleUploadFile(TritonTransferClient md_server);
void getAllHashesForFile(const std::string &file_name, std::vector<HashValue> &hashes_for_file);
bool handleDownloadFile(TritonTransferClient client);
bool downloadMissingBlocks(ServerInfo download_server, vector<HashValue> *hashes_needed);
void printStatics();
void writeFileToDisk(string content, string file_name);
bool assembleAndWriteFile(string file_name);
bool uploadBlocksToBlockServer(ServerInfo upload_server);
ServerInfo getHashesFromDownloadServer(vector<ServerInfo> servers_and_hashes);
ServerAttributesStruct *connectToBlockServer(ServerAddr server_name, int32_t server_port);

// struct ServerInfo {
//     1: required list<HashValue> server_hash_list,
//     2: required i32 port,
//     3: required ServerAddr server_name,
//     4: required bool file_exists,
// }

int main(int argc, char **argv)
{
    // tt-client <md_server_name> <md_server_port> upload <filename>
    // tt-client <t<md_server_name> <md_server_port> download <filename> <download_dir>
    if (argc != 6 && argc != 5) { // Test for correct number of arguments
        // cerr << "To download, use: <md_server_name> <md_server_port> download <filename> <download_dir>\n";
        // cerr << "To upload, use: <md_server_name> <md_server_port> upload <filename> \n";
        return 0;
    }
    try {
        string sn(argv[1]);
        md_server_name = sn;
        md_port = atoi(argv[2]); // First arg:  local port
        string action(argv[3]);
        boost::shared_ptr<TTransport> socket(new TSocket(md_server_name, md_port));
        boost::shared_ptr<TTransport> tr(new TBufferedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(tr));
        TritonTransferClient md_server(protocol);
        transport_static = tr;
        transport_static->open();
        md_server.ping();
        // cerr << "Successfully connected to MDServer." << endl;
        if ("download" == action) {
            file_path = getRealPath(string(argv[5]));
            static_file_name = getFileName(string(argv[4]));
            path_and_file_name = getRealPath(file_path + "/" + static_file_name);
            // printStatics();
            bootstrapClient();
            if (handleDownloadFile(md_server)) {
                // cerr << "Downloaded " << static_file_name << " to " << path_and_file_name << " successfully.\n";
                cout << "OK" << endl;
            } else {
                // cerr << "ERROR: Downloading " << static_file_name << " to " << path_and_file_name << " failed.\n";
                cout << "ERROR" << endl;
            }
        } else if ("upload" == action) {
            file_path = getDirName(string(argv[4]));
            static_file_name = getFileName(string(argv[4]));
            path_and_file_name = getRealPath(file_path + "/" + static_file_name);
            // printStatics();
            bootstrapClient();
            if (handleUploadFile(md_server)) {
                cout << "OK" << endl;
            } else {
                cout << "ERROR" << endl;
            }
        }
        transport_static->close();
        delete hash_to_block;
        delete file_names_to_hashes;
        return 0;
    } catch (...) {
        // cerr << "ERROR: Server did not respond to ping!\n" << endl;
        cout << "ERROR" << endl;
        transport_static->close();
        return 0;
    }
}

bool handleDownloadFile(TritonTransferClient md_server)
{
    if (!access(path_and_file_name.c_str(), F_OK)) {
        // cerr << "File already exists on client." << endl;
        return false;
    }
    vector<ServerInfo> servers_and_hashes;
    md_server.downloadFile(servers_and_hashes, static_file_name);
    ServerInfo download_server = getHashesFromDownloadServer(servers_and_hashes);
    if (download_server.file_exists == false) {
        // cerr << "ERROR: The requested file was not found on any of the block servers.\n";
        return false;
    }
    if (file_names_to_hashes->find(path_and_file_name) == file_names_to_hashes->end()) {
        // cerr << path_and_file_name << " was not in memory. Creating an entry in file_names_to_hashes.\n";
        file_names_to_hashes->insert(std::make_pair(path_and_file_name, new vector<HashValue>(download_server.server_hash_list)));
    }
    vector<HashValue> *hashes_needed = new vector<HashValue>();
    for (int i = 0; i < download_server.server_hash_list.size(); i++) { // check what blocks are in memory
        HashValue curr_hash = download_server.server_hash_list.at(i);
        auto curr_hash_in_mem = hash_to_block->find(curr_hash);
        if (curr_hash_in_mem == hash_to_block->end()) {
            hashes_needed->push_back(download_server.server_hash_list.at(i));
        }
    }
    // cerr << "Client needs " << hashes_needed->size() << " blocks to build " << path_and_file_name << endl;
    bool success = downloadMissingBlocks(download_server, hashes_needed);
    if (!success) {
        return success;
    } else {
        return assembleAndWriteFile(path_and_file_name);
    }
}

ServerInfo getHashesFromDownloadServer(vector<ServerInfo> servers_and_hashes)
{
    for (int i = 0; i < servers_and_hashes.size(); i++) {
        ServerInfo curr_server = servers_and_hashes.at(i);
        if (curr_server.file_exists) {
            // cerr << "Download server found with name " << servers_and_hashes.at(i).server_name << " and port " << servers_and_hashes.at(i).port << endl;
            // cerr << path_and_file_name << " has " << servers_and_hashes.at(i).server_hash_list.size() << " total blocks.\n";
            return servers_and_hashes.at(i);
        }
    }
    ServerInfo dummy;
    dummy.file_exists = false;
    return dummy;
}

bool assembleAndWriteFile(string file_name)
{
    auto file_entry = file_names_to_hashes->find(file_name);
    if (file_names_to_hashes->end() ==  file_entry) {
        // cerr << file_name << " was not found in file_names_to_hashes in assembleAndWriteFile().\n";
        return false;
    }
    // cerr << "Assembling " << file_name << " from " << file_entry->second->size() << " blocks"  "..." << endl;
    string assembled_file = "";
    for (int i = 0; i < file_entry->second->size(); i++) {
        assembled_file += hash_to_block->find(file_entry->second->at(i))->second;
    }
    // cerr << "About to write downloaded file to disk: " << file_name << " with size: " << assembled_file.size() << endl;
    writeFileToDisk(assembled_file, file_name);
    return true;
}

bool downloadMissingBlocks(ServerInfo download_server, vector<HashValue> *hashes_needed)
{
    ServerAttributesStruct *download_server_attribs = connectToBlockServer(download_server.server_name, download_server.port);
    if (NULL == download_server_attribs) {
        return false;
    }
    bool error_occurred = false;
    for (int i = 0; i < hashes_needed->size(); i++) {
        ErrorOrBlock curr_block;
        download_server_attribs->server->downloadBlock(curr_block, hashes_needed->at(i));
        if (curr_block.error) {
            error_occurred = true;
            // cleanupServerConnection(download_server_attribs);
            // return false;
        } else {
            // cerr << "Downloaded block of size " << curr_block.block.length() << endl;
            hash_to_block->insert(std::make_pair(hashes_needed->at(i), curr_block.block));
        }
    }
    // if (error_occurred) {
    //     cout << "ERROR" << endl;
    // } else {
    //     cout << "OK" << endl;
    // }
    cleanupServerConnection(download_server_attribs);
    return !error_occurred;
}

void cleanupServerConnection(ServerAttributesStruct *server_attribs)
{
    // cerr << "Closing remote connection with " << server_attribs->server_name << " on port " << server_attribs->port << endl;
    server_attribs->transport->close();
    delete server_attribs;
}

void bootstrapClient()
{
    // cerr << "Bootstrapping client...\n";
    hash_to_block = new unordered_map<string, string>();
    file_names_to_hashes = new unordered_map<string, vector<string> *>();
    vector<string> *file_names = getAllFilesInDir(file_path);
    for (int i = 0; i < file_names->size(); i++) {
        std::vector<HashValue> hashes_for_file;
        getAllHashesForFile(file_names->at(i), hashes_for_file);
    }
    delete file_names;
}

bool handleUploadFile(TritonTransferClient md_server)
{
    // cerr << "Entered handleUploadFile.\n";
    std::vector<HashValue> hash_list;
    getAllHashesForFile(path_and_file_name, hash_list);
    std::vector<HashValue> needed_blocks;
    // cerr << path_and_file_name << " has " << hash_list.size() << " blocks.\n";
    std::vector<ServerInfo> block_server_info;
    md_server.uploadFile(block_server_info, static_file_name, hash_list);
    // cerr << "Server still needs " << needed_blocks.size() << " blocks for " << path_and_file_name << endl;
    // cerr << "Starting to upload blocks to " << block_server_info.size() << " available servers." << endl;
    int i;
    bool error_occurred = false;
    for (i = 0; i < block_server_info.size(); i++) {
        if (i >= 2) {
            // cerr << "Uploaded to 2 servers before reaching the end of the block server list.\n";
            break;
        }
        if (!uploadBlocksToBlockServer(block_server_info.at(i))) {
            error_occurred = true;
        }
    }
    // cerr << "File uploaded to " << i << " block servers.\n";
    return !error_occurred;
}

bool uploadBlocksToBlockServer(ServerInfo upload_server)
{
    bool error_occurred = false;
    // cerr << "Uploading " << path_and_file_name << " to " << upload_server.server_name << " on port " << upload_server.port << endl;
    ServerAttributesStruct *upload_server_attribs = connectToBlockServer(upload_server.server_name, upload_server.port);
    if (NULL == upload_server_attribs) {
        // cout << "ERROR" << endl;
        return false;
    }
    for (int i = 0; i < upload_server.server_hash_list.size(); i++) {
        string status;
        Block block_to_upload = hash_to_block->find(upload_server.server_hash_list.at(i))->second;
        upload_server_attribs->server->uploadBlock(status, upload_server.server_hash_list.at(i), block_to_upload);
        if (status == "ERROR") {
            error_occurred = true;
        }
    }
    // if (error_occurred) {
    //     cout << "ERROR" << endl;
    // } else {
    //     cout << "OK" << endl;
    //     // cerr << "Successfully uploaded " << path_and_file_name << " to " << upload_server.server_name << endl;
    // }
    cleanupServerConnection(upload_server_attribs);
    return !error_occurred;
}

void getAllHashesForFile(const std::string &file_name, std::vector<HashValue> &hashes_for_file)
{
    if (file_names_to_hashes->end() == file_names_to_hashes->find(file_name)) {
        string file = readFileFromDisk(file_name);
        // cerr << "Getting all hashes for " << file_name << " with size: " << file.size() << endl;
        unsigned long long pos = 0;
        HashValue hash;
        while (pos < file.size()) {
            if (pos + MAX_BLOCK_SIZE < file.size()) {
                string block = file.substr(pos, MAX_BLOCK_SIZE);
                hash = sha256(block);
                if (hash_to_block->end() == hash_to_block->find(hash)) {
                    hash_to_block->insert(std::make_pair(hash, block));
                }
                pos += MAX_BLOCK_SIZE;
            } else {
                string block = file.substr(pos, file.size() - pos);
                hash = sha256(block);
                if (hash_to_block->end() == hash_to_block->find(hash)) {
                    hash_to_block->insert(std::make_pair(hash, block));
                }
                pos += file.size() - pos;
            }
            hashes_for_file.push_back(hash);
        }
        file_names_to_hashes->insert(std::make_pair(file_name, new vector<HashValue>(hashes_for_file)));
    } else {
        // cerr << file_name << " is already in memory." << endl;
        auto file_in_memory = file_names_to_hashes->find(file_name);
        for (int i = 0; i < file_in_memory->second->size(); i++) {
            hashes_for_file.push_back(file_in_memory->second->at(i));
        }
    }
    return;
}

ServerAttributesStruct *connectToBlockServer(ServerAddr server_name, int32_t server_port)
{
    try {
        boost::shared_ptr<TTransport> socket(new TSocket(server_name, server_port));
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        TritonTransferClient *tt_block_server = new TritonTransferClient(protocol);
        transport->open();
        // cerr << "Transport opened.\n";
        tt_block_server->ping();
        // cerr << "Successfully pinged " << server_name << endl;
        ServerAttributesStruct *server_attribs = new ServerAttributesStruct();
        server_attribs->port = server_port;
        server_attribs->server_name = server_name;
        server_attribs->server = tt_block_server;
        server_attribs->transport = transport;
        return server_attribs;
    } catch (...) {
        // cerr << "ERROR: Server did not respond to ping!\n" << endl;
        return NULL;
    }
}

void printStatics()
{
    // cerr << "file_path: " << file_path << endl;
    // cerr << "static_file_name: " << static_file_name << endl;
    // cerr << "path_and_file_name: " << path_and_file_name << endl;
}
// TODO check spec for grading point rubric and make sure i have everything
