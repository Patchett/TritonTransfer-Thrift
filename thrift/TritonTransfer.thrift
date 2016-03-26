
namespace cpp no.podcasts.no.learning

typedef string HashValue
typedef binary Block
typedef string ServerAddr

struct ErrorOrBlock {
    1: required bool error = false,
    2: required Block block,
}

struct ServerInfo {
    1: required list<HashValue> server_hash_list,
    2: required i32 port,
    3: required ServerAddr server_name,
    4: required bool file_exists,
}

service TritonTransfer {
    void ping(),

    /*
     * uploadFile
     * To upload a file f, the client invokes an uploadFile RPC call, which takes two arguments:
     *
     * filename: the name of the file being uploaded
     * hashlist: a list of block hashes making up the file
     *
     * Return value: uploadFile returns a list containing the hashes
     * of blocks it still needs, if any. If the returned list is empty,
     * that means that the file has been successfully uploaded. If the
     * list is not empty, then additional blocks must be uploaded, using the next RPC call
     */
    list<ServerInfo> uploadFile(1:string file_name, 2:list<HashValue> hash_list),

    /*
     * uploadBlock
     * To upload a block b, the client invokes an uploadBlock RPC call, which takes two arguments:
     *
     * hash: the hash value identifying the block
     * block: the byte array making up the block
     *
     * Return value: uploadBlock returns 'OK' if the block was stored
     * successfully, or 'ERROR' if there was an error. Errors could occur if
     * the block is longer than 16KB, or if the hash value doesn't match the
     * hash of the actual block itself.
     */
    string uploadBlock(1:HashValue hv, 2:Block block),

    /*
     * downloadFile
     * To download a file f, the client invokes a downloadFile RPC call, which takes ones argument:
     *
     * filename: the name of the file being uploaded
     *
     * Return value: downloadFile returns a list containing the hashes of blocks
     * making up the file.
     */
    list<ServerInfo> downloadFile(1:string file_name),

    /*
     * downloadBlock
     * To download a block b, the client invokes a downloadBlock RPC call, which takes one argument:
     *
     * hash: the hash value identifying the block
     *
     * Return value: downloadBlock returns the contents of the block if it is
     * stored on the server, or 'ERROR' if the block does not exist on the server.
     */
    ErrorOrBlock downloadBlock(1: HashValue hv),

    void bootstrapBlockServer(1: i32 port, 2: ServerAddr server_addr),
}



