CC=g++
CFLAGS = -std=c++11 -lgsl -ggdb -D_GNU_SOURCE -pthread -I/usr/local/include/thrift
GEN = ./gen-cpp/TritonTransfer.cpp ./gen-cpp/TritonTransfer_constants.cpp ./gen-cpp/TritonTransfer_types.cpp
SOURCES = TritonTransfer_client.cpp
UTILS = ../utils/utils.cpp ../utils/sha256.cpp

default: clean all
all: tt-client

tt-client: $(SOURCES) $(UTILS) $(GEN)
	$(CC) $(CFLAGS) -o tt-client $(SOURCES) $(UTILS) $(GEN) -lthrift

clean:
	rm -rf tt-client client.dSYM server.dSYM *.o
