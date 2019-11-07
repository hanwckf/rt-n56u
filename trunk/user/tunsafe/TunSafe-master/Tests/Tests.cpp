// Tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "../ip_to_peer_map.h"
#include "../tunsafe_endian.h"
#include "../bit_ops.h"
#include <assert.h>
class RoutingTrie32Ref {
  typedef void *Value;
  typedef uint32 NodePtr;
  enum {
    BITS = 32
  };

  struct Node {
    Value value;
    NodePtr nodes[2];
    uint8 shift, cidr;
    uint32 key;
  };

  std::vector<Node> nodes_;
  NodePtr root_;

#define NEXT_NODE(n, key) n->nodes[(key >> n->shift) & 1]

  static int CommonBits(const uint32 a, const uint32 b) {
    return 32 - FindHighestSetBit32(a ^ b);
  }

  NodePtr NewNode(uint32 key, int cidr, Value value) {
    nodes_.emplace_back();
    Node *n = &nodes_.back();
    if (n) {
      n->nodes[0] = n->nodes[1] = 0;
      n->cidr = cidr;
      n->shift = (~cidr & 31);
      n->value = value;
      n->key = key;
    }
    return (uint32)nodes_.size();
  }

public:
  RoutingTrie32Ref() : root_() {}
  ~RoutingTrie32Ref() {

  }

  __declspec(noinline) Value Lookup(const uint8 *key_in) {
    NodePtr ni = root_;
    Node *n;
    Value value = NULL;
    uint32 key = ReadBE32(key_in);
    while (ni != NULL && (n = &nodes_[ni - 1], CommonBits(key, n->key) >= n->cidr)) {
      value = n->value ? n->value : value;
      ni = n->nodes[(key >> n->shift) & 1];
    }
    return value;
  }

  Node *deref(NodePtr x) { return &nodes_[x - 1]; }

  bool Insert(const uint8 *key_in, int cidr, Value value) {
    NodePtr p = 0, no = root_;
    uint32 key = ReadBE32(key_in);
    while (no != NULL && (deref(no)->cidr <= cidr && CommonBits(key, deref(no)->key) >= deref(no)->cidr)) {
      if (deref(no)->cidr == cidr) {
        deref(no)->value = value;
        return true;
      }
      p = no;// &NEXT_NODE(deref(no), key);
      no = NEXT_NODE(deref(p), key);
    }
    NodePtr n = NewNode(key, cidr, value);
    if (!n)
      return false;

    if (no != NULL) {
      int cbits = CommonBits(deref(no)->key, key);
      if (cbits < cidr) {
        NodePtr nn = NewNode(key, cbits, NULL);
        if (!nn) {
          //          delete n;
          return false;
        }
        NEXT_NODE(deref(nn), key) = n;
        n = nn;
      }
      NEXT_NODE(deref(n), deref(no)->key) = no;
    }
    if (p == NULL)
      root_ = n;
    else
      NEXT_NODE(deref(p), key) = n;
    return true;
  }

};



int main() {
  RoutingTrie32Ref ref;
  RoutingTrie32 test;

  for (int i = 0; i < 256; i++) {
    uint8 ip[4];
    uint32 ipv = 0x10200000 + i * 256;
    WriteBE32(ip, ipv);
    ref.Insert(ip, 24, (void*)(i * 2));
    void*x = (void*)(i * 2);
    test.Insert(ipv, 24, &x);
  }

  uint32 iters = 0x800000;
  for (uint32 i = 0; i < iters; i++) {
    uint32 j = i * 944676833;// (i & 0xF) + (i & ~0xF) * 16 + 0xF0;
    uint8 xx[4] = { (uint8)(j >> 24),(uint8)(j >> 16),(uint8)(j >> 8),(uint8)j };
    void *ans1 = ref.Lookup(xx);
    void *ans2 = test.Lookup(j);
    assert(ans1 == ans2);
  }

}
