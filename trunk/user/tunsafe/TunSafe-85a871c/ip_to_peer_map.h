// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#include "tunsafe_types.h"
#include <vector>

class RoutingTrie32 {
  friend class FreeableNodeCollector;
public:
  typedef void *Value;
  struct Node;

  RoutingTrie32();
  ~RoutingTrie32();
  NOINLINE Value Lookup(uint32 ip);
  NOINLINE Value LookupExact(uint32 ip, int cidr);
  bool Insert(uint32 ip, int cidr, Value *value);
  bool Delete(uint32 ip, int cidr);

private:
  Node *root_;

  void Rebalance(Node *n);
  bool Resize(Node *n);
  bool Inflate(Node **n);
  bool Halve(Node **n);
  void UpdateParent(Node *n);
  void ResizeChildren(Node *n);
  static void Collapse(Node **n);
  static void PutChild(Node *pn, uint32 i, Node *n);
  static void ReplaceChild(Node **pnp, Node *n);
  static Node *ConvertOleafToLeaf(Node *pn, uint32 i, Node *n);
  static bool InsertLeafInto(Node **n, uint8 leaf_pos, RoutingTrie32::Value *value);
};


// Maps CIDR addresses to a peer, always returning the longest match
// IPv6 has a slow O(n) implementation
class IpToPeerMap {
public:
  IpToPeerMap();
  ~IpToPeerMap();

  // Inserts an IP address of a given CIDR length into the lookup table, pointing to peer.
  void *InsertV4(uint32 ip, int cidr, void *peer);
  void *InsertV6(const void *addr, int cidr, void *peer);

  // Lookup the peer matching the IP Address
  void *LookupV4(uint32 ip);
  void *LookupV6(const void *addr);

  void RemoveV4(uint32 ip, int cidr);
  void RemoveV6(const void *addr, int cidr);
private:
  struct Entry6 {
    uint8 ip[16];
    uint8 cidr_len;
    void *peer;
  };
  std::vector<Entry6> ipv6_;

  RoutingTrie32 ipv4_;
};
