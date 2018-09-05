### TritonTransfer-Thrift RPC
This project is a DropBox like block storage file redundant remote file system which uses the Thrift RPC framework.
Each file is stored in 64 byte blocks across multiple block servers like the below diagram:
Every block is stored on at least 2 block servers for redundancy. Each block is also hashed which allows to detect duplicate
blocks and reduce storage overhead.



## Client
The client is a command-line tool that the user uses to upload and download files from TritonTransfer. It points at a directory
on the user's local file system which is used as the user's local storage when they download a file

## Block Server
The block server is an in-memory data store that stores blocks of data, indexed by the hash value. Thus it is a key-value store. It supports a basic get(), put(), and delete() interface to get a block, add a new block, or remove a block. The block server only knows about blocks–it doesn’t know anything about how blocks relate to files.

## Metadata Server
The metadata server maintains the mapping of filenames to blocklists. All metadata is stored in memory, and no database systems or files
are used to maintain the data. Thus the Metadata Server is stateless and could easily be replicated or loadbalanced. The Metadata Server
is capable of rebuilding its index from scratch on each start.

The Metadata Server also orchestrates uploads and downloads. The client never talks directly to a block server except when it needs to download
a specific block. 

# Downloads
To download a file, the client asks the Metadata Server which Block Server is holding the file. The Metadata Server returns
a Block Server connection object and a list of hashes corresponding to blocks that the client needs to ask the Block Server for.
The client then downloads each hashed block directly from the Block Server

# Uploads
To upload a file, the client sends a list of file hashes to the Metadata Server in an upload request. The Metadata Server checks which blocks
are already present on a target Block Server, and returns a list of only the hashes that are missing from that Block so that the client knows
exactly which blocks to upload without creating duplicate blocks. The Metadata Server also orchestrates round robin uploads.

# Redundancy
Every block is stored on at least 2 Block Servers. If one Block Server goes down, all files should still be available and there should be no service interruption

# Sequence Diagrams
