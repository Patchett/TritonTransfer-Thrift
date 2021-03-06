/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef TritonTransfer_TYPES_H
#define TritonTransfer_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace no { namespace podcasts { namespace no { namespace learning {

typedef std::string HashValue;

typedef std::string Block;

typedef std::string ServerAddr;


class ErrorOrBlock {
 public:

  static const char* ascii_fingerprint; // = "1767FFB0CB3D9275BC64B198AB3B8A8B";
  static const uint8_t binary_fingerprint[16]; // = {0x17,0x67,0xFF,0xB0,0xCB,0x3D,0x92,0x75,0xBC,0x64,0xB1,0x98,0xAB,0x3B,0x8A,0x8B};

  ErrorOrBlock() : error(false), block() {
  }

  virtual ~ErrorOrBlock() throw() {}

  bool error;
  Block block;

  void __set_error(const bool val) {
    error = val;
  }

  void __set_block(const Block& val) {
    block = val;
  }

  bool operator == (const ErrorOrBlock & rhs) const
  {
    if (!(error == rhs.error))
      return false;
    if (!(block == rhs.block))
      return false;
    return true;
  }
  bool operator != (const ErrorOrBlock &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ErrorOrBlock & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(ErrorOrBlock &a, ErrorOrBlock &b);


class ServerInfo {
 public:

  static const char* ascii_fingerprint; // = "4E6823FAF28E23B01A9E7F276B12377B";
  static const uint8_t binary_fingerprint[16]; // = {0x4E,0x68,0x23,0xFA,0xF2,0x8E,0x23,0xB0,0x1A,0x9E,0x7F,0x27,0x6B,0x12,0x37,0x7B};

  ServerInfo() : port(0), server_name(), file_exists(0) {
  }

  virtual ~ServerInfo() throw() {}

  std::vector<HashValue>  server_hash_list;
  int32_t port;
  ServerAddr server_name;
  bool file_exists;

  void __set_server_hash_list(const std::vector<HashValue> & val) {
    server_hash_list = val;
  }

  void __set_port(const int32_t val) {
    port = val;
  }

  void __set_server_name(const ServerAddr& val) {
    server_name = val;
  }

  void __set_file_exists(const bool val) {
    file_exists = val;
  }

  bool operator == (const ServerInfo & rhs) const
  {
    if (!(server_hash_list == rhs.server_hash_list))
      return false;
    if (!(port == rhs.port))
      return false;
    if (!(server_name == rhs.server_name))
      return false;
    if (!(file_exists == rhs.file_exists))
      return false;
    return true;
  }
  bool operator != (const ServerInfo &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ServerInfo & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(ServerInfo &a, ServerInfo &b);

}}}} // namespace

#endif
