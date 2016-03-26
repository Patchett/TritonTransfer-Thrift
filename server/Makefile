CC=g++
CFLAGS = -std=c++11 -lgsl -ggdb -D_GNU_SOURCE -pthread -I/usr/local/include/thrift
GEN = ./gen-cpp/TritonTransfer.cpp ./gen-cpp/TritonTransfer_constants.cpp ./gen-cpp/TritonTransfer_types.cpp
MD_SERVER = MDServerHandler.cpp TritonTransfer_MDServer.cpp
BLOCK_SERVER = BlockServerHandler.cpp TritonTransfer_BlockServer.cpp
UTILS = ../utils/utils.cpp ../utils/sha256.cpp

default: clean all
all: tt-md-server tt-block-server
# all: tt-md-server


tt-md-server: $(MD_SERVER) $(UTILS) $(GEN)
	$(CC) $(CFLAGS) -o tt-md-server $(MD_SERVER) $(UTILS) $(GEN) -lthrift

tt-block-server: $(BLOCK_SERVER) $(UTILS) $(GEN)
	$(CC) $(CFLAGS) -o tt-block-server $(BLOCK_SERVER) $(UTILS) $(GEN) -lthrift

clean:
	rm -rf tt-md-server tt-client tt-block-server client.dSYM server.dSYM *.o
