#ifndef TUNSAFE_NETWORK_COMMON_H_
#define TUNSAFE_NETWORK_COMMON_H_

#include "netapi.h"
#include "crypto/chacha20poly1305.h"

class PacketProcessor;
class WgPacketObfuscator;

// A simple singlethreaded pool of packets used on windows where 
// FreePacket / AllocPacket are multithreded and thus slightly slower
#if defined(OS_WIN)
class SimplePacketPool {
public:
  explicit SimplePacketPool() {
    freed_packets_ = NULL;
    freed_packets_count_ = 0;
  }
  ~SimplePacketPool() {
    FreePacketList(freed_packets_);
  }
  Packet *AllocPacketFromPool() {
    if (Packet *p = freed_packets_) {
      freed_packets_ = Packet_NEXT(p);
      freed_packets_count_--;
      p->Reset();
      return p;
    }
    return AllocPacket();
  }
  void FreePacketToPool(Packet *p) {
    Packet_NEXT(p) = freed_packets_;
    freed_packets_ = p;
    freed_packets_count_++;
  }
  void FreeSomePackets() {
    if (freed_packets_count_ > 32)
      FreeSomePacketsInner();
  }
  void FreeSomePacketsInner();


  int freed_packets_count_;
  Packet *freed_packets_;
};
#else
class SimplePacketPool {
public:
  Packet *AllocPacketFromPool() {
    return AllocPacket();
  }
  void FreePacketToPool(Packet *packet) {
    return FreePacket(packet);
  }
};
#endif


class TcpPacketQueue {
public:
  explicit TcpPacketQueue(SimplePacketPool *pool) : rqueue_bytes_(0), rqueue_(NULL), rqueue_end_(&rqueue_), pool_(pool) {}
  ~TcpPacketQueue();

  Packet *Read(uint num);
  Packet *ReadUpTo(uint num);
  void Read(uint8 *dst, uint num);
  uint PeekUint16();

  void Add(Packet *packet);

  uint32 size() const { return rqueue_bytes_; }
 
  SimplePacketPool *pool() { return pool_; }
private:
  // Total # of bytes queued
  uint rqueue_bytes_;

  // Buffered data
  Packet *rqueue_, **rqueue_end_;

  SimplePacketPool *pool_;
};


// Aids with prefixing and parsing incoming and outgoing
// packets with the tcp protocol header.
class TcpPacketHandler {
public:
  enum {
    kObfuscationMode_Unspecified = -1,
    kObfuscationMode_None = 0,
    kObfuscationMode_Encrypted = 1,
    kObfuscationMode_TlsFirefox = 2,
    kObfuscationMode_TlsChrome = 3,
    kObfuscationMode_Autodetect = 4,
  };

  explicit TcpPacketHandler(SimplePacketPool *packet_pool, WgPacketObfuscator *obfuscator, bool is_incoming);
  ~TcpPacketHandler();

  // Adds a tcp header to a data packet so it can be transmitted on the wire
  void PrepareOutgoingPackets(Packet *p);

  // Add a new chunk of incoming data to the packet list
  void QueueIncomingPacket(Packet *p) {
    queue_.Add(p);
  }

  // Attempt to extract the next packet, returns NULL when complete.
  Packet *GetNextWireguardPacket();
  
  bool error() const { return error_flag_; }

private:
  void PrepareOutgoingPacketsNormal(Packet *p);
  void PrepareOutgoingPacketsObfuscate(Packet *p);
  void PrepareOutgoingPacketsTLS13(Packet *p);
  void PrepareOutgoingPacketsWithHeader(Packet *p);

  Packet *GetNextWireguardPacketNormal();
  Packet *GetNextWireguardPacketObfuscate(TcpPacketQueue *queue);
  Packet *GetNextWireguardPacketTLS13();

  size_t CreateTls13ClientHello(uint8 *dst);
  size_t CreateTls13ServerHello(uint8 *dst);
     

  // Set if there's a fatal error
  bool error_flag_;
  uint8 obfuscation_mode_;
  uint8 read_state_, write_state_, tls_read_state_;
  bool decryptor_initialized_;

  uint8 packet_header_[2];

  // Number of data bytes left
  uint tls_bytes_left_;

  TcpPacketQueue queue_;

  // There's a separate queue for tls since it unwraps stuff
  TcpPacketQueue tls_queue_;

  uint32 predicted_key_in_, predicted_key_out_;
  uint64 predicted_serial_in_, predicted_serial_out_;

  // For obfuscating
  chacha20_streaming encryptor_, decryptor_;

  uint8 tls_session_id_[32];

};

#endif  // TUNSAFE_NETWORK_COMMON_H_