// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "ip_to_peer_map.h"
#include "bit_ops.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "util.h"

IpToPeerMap::IpToPeerMap() {

}

IpToPeerMap::~IpToPeerMap() {
}

void *IpToPeerMap::InsertV4(uint32 ip, int cidr, void *peer) {
  ipv4_.Insert(ip, cidr, &peer);
  return peer;
}

void *IpToPeerMap::InsertV6(const void *addr, int cidr, void *peer) {
  Entry6 e;
  for (auto it = ipv6_.begin(); it != ipv6_.end(); ++it) {
    if (it->cidr_len == cidr && memcmp(it->ip, addr, 16) == 0)
      return exch(it->peer, peer);
  }
  e.cidr_len = cidr;
  e.peer = peer;
  memcpy(e.ip, addr, 16);
  ipv6_.push_back(e);
  return NULL;
}

void *IpToPeerMap::LookupV4(uint32 ip) {
  return ipv4_.Lookup(ip);
}

void IpToPeerMap::RemoveV4(uint32 ip, int cidr) {
  ipv4_.Delete(ip, cidr);
}

void IpToPeerMap::RemoveV6(const void *addr, int cidr) {
  for (auto it = ipv6_.begin(); it != ipv6_.end(); ++it) {
    if (it->cidr_len == cidr && memcmp(it->ip, addr, 16) == 0) {
      ipv6_.erase(it);
      return;
    }
  }
}

static int CalculateIPv6CommonPrefix(const uint8 *a, const uint8 *b) {
  uint64 x = ToBE64(*(uint64*)&a[0] ^ *(uint64*)&b[0]);
  uint64 y = ToBE64(*(uint64*)&a[8] ^ *(uint64*)&b[8]);
  return x ? 64 - FindHighestSetBit64(x) : 128 - FindHighestSetBit64(y);
}

void *IpToPeerMap::LookupV6(const void *addr) {
  int best_len = 0;
  void *best_peer = NULL;
  for (auto it = ipv6_.begin(); it != ipv6_.end(); ++it) {
    int len = CalculateIPv6CommonPrefix((const uint8*)addr, it->ip);
    if (len >= it->cidr_len && len >= best_len) {
      best_len = len;
      best_peer = it->peer;
    }
  }
  return best_peer;
}

#pragma warning (disable: 4200)  // warning C4200: nonstandard extension used: zero-sized array in struct/union
struct RoutingTrie32::Node {
  uint32 key;
  // bits == 0 if this is a leaf
  uint8 pos, bits;
  union {
    struct {
      Node *parent;
      uint32 full_children, empty_children;
      Node *child[0];
    };
    struct {
      Node *leaf_next;
      Value leaf_value;
    };
  };
};


static inline uint32 prefix_mismatch(uint32 key, RoutingTrie32::Node *n) {
  uint32 prefix = n->key;
  return (key ^ prefix) & (prefix | (uint32)-(int32)prefix);
}

static uint32 make_cidr_mask(uint8 cidr) {
  return cidr == 0 ? 0 : 0xffffffff << (32 - cidr);
}

#define IS_LEAF(n) (n->bits == 0)
#define GET_INDEX(k, n) ((n->key ^ k) >> n->pos)

#define NODE_IS_OLEAF(n) ((intptr_t)(n) & 1)
#define NODE_IS_NULL_OR_OLEAF(n) ((n) == 0 || NODE_IS_OLEAF(n))
#define VALUE_TO_OLEAF(n) ((Node*)((intptr_t)(n) + 1))
#define VALUE_FROM_OLEAF(n) ((void*)((intptr_t)(n) - 1))

static RoutingTrie32::Node *NewNode(uint32 key, int pos, int bits) {
  RoutingTrie32::Node *n = (RoutingTrie32::Node *)malloc(offsetof(RoutingTrie32::Node, child[(uint32)(1U << bits)]));
  if (n) {
    n->parent = NULL;
    n->pos = pos;
    n->bits = bits;
    n->full_children = 0;
    n->empty_children = 1U << bits;
    uint32 s = pos + bits;
    n->key = (s < 32) ? key >> s << s : 0;
    memset(n->child, 0, n->empty_children * sizeof(RoutingTrie32::Node*));
  }
  return n;
}

static RoutingTrie32::Node *NewLeaf(uint32 key, uint8 leaf_pos, RoutingTrie32::Value value) {
  RoutingTrie32::Node *n = (RoutingTrie32::Node *)malloc(sizeof(RoutingTrie32::Node));
  if (n) {
    n->key = key;
    n->bits = 0;
    n->pos = leaf_pos;
    n->leaf_value = value;
    n->leaf_next = NULL;
  }
  return n;
}

static void FreeNode(RoutingTrie32::Node *n) {
  free(n);
}

static void RecursiveFreeNode(RoutingTrie32::Node *n) {
  RoutingTrie32::Node *cn;

  if (n->bits == 0) {
    while ((cn = n->leaf_next) != NULL) {
      n->leaf_next = cn->leaf_next;
      FreeNode(cn);
    }
  } else {
    uint32 items = 1 << n->bits;
    for (uint32 i = 0; i != items; i++) {
      RoutingTrie32::Node *cn = n->child[i];
      if (!NODE_IS_NULL_OR_OLEAF(cn))
        RecursiveFreeNode(cn);
    }
  }
  FreeNode(n);
}


RoutingTrie32::RoutingTrie32()
  : root_(NULL) {
}

RoutingTrie32::~RoutingTrie32() {
  if (root_)
    RecursiveFreeNode(root_);
}

RoutingTrie32::Value RoutingTrie32::Lookup(uint32 ip) {
  uint32 key = ip;
  Node *n = root_, *pn = n, *ppn;
  uint32 cindex = 0;
  if (!n)
    return NULL;
  // Find the longest prefix match
  for (;;) {
    uint32 index = GET_INDEX(key, n);
    if (index >> n->bits)
      break; // mismatch in skipped bits
    if (IS_LEAF(n))
      return n->leaf_value;
    pn = n;
    cindex = index;
    n = n->child[index];
    if (NODE_IS_NULL_OR_OLEAF(n)) {
      if (!n)
        goto backtrace;
      // node is an optimized leaf
      return VALUE_FROM_OLEAF(n);
    }
  }
  // backtrace for longest prefix
  for (;;) {
    if (prefix_mismatch(key, n))
      goto backtrace;
    if (IS_LEAF(n)) {
      for (;;) {
        if (((n->key ^ key) >> n->pos) == 0)
          return n->leaf_value;
        if (n->leaf_next == NULL) {
          if (n->pos == 32)
            return n->leaf_value;
          break;
        }
        n = n->leaf_next;
      }
      goto backtrace;
    }

    ppn = n;
    n = n->child[0];
    if (NODE_IS_NULL_OR_OLEAF(n)) {
      if (n) {
        if (((ppn->key ^ key) >> ppn->pos) == 0)
          return VALUE_FROM_OLEAF(n);
      }
      for (;;) {
backtrace:
        // step up to previous parent when we used all bits in current
        while (cindex == 0) {
          uint32 pkey = pn->key;
          pn = pn->parent;
          if (!pn)
            return 0;
          cindex = (pn->key ^ pkey) >> pn->pos;
        }
        // strip lsb of cindex and find child
        cindex &= cindex - 1;
        assert(cindex < (1U << pn->bits));
        n = pn->child[cindex];
        if (!NODE_IS_NULL_OR_OLEAF(n))
          break;
        if (n) {
          uint32 nkey = pn->key + (cindex << pn->pos);
          if (((nkey ^ key) >> pn->pos) == 0)
            return VALUE_FROM_OLEAF(n);
        }
      }
    }
  }
}

bool RoutingTrie32::InsertLeafInto(Node **nn, uint8 leaf_pos, Value *valuep) {
  // put higher cidr higher up
  Node *n = *nn;
  assert(IS_LEAF(n));
  uint32 key = n->key;
  do {
    if (leaf_pos < n->pos)
      break;
    if (leaf_pos == n->pos) {
      std::swap(n->leaf_value, *valuep);
      return true;
    }
    nn = &n->leaf_next;
  } while ((n = *nn) != NULL);
  Node *leaf = NewLeaf(key, leaf_pos, *valuep);
  if (leaf == NULL)
    return false;
  leaf->leaf_next = *nn;
  *nn = leaf;
  return true;
}

static inline bool IsFull(RoutingTrie32::Node *pn, RoutingTrie32::Node *n) {
  return !NODE_IS_NULL_OR_OLEAF(n) && (n->pos + n->bits) == pn->pos && !IS_LEAF(n);
}

void RoutingTrie32::PutChild(Node *pn, uint32 i, Node *n) {
  Node *on = pn->child[i];
  pn->child[i] = n;

  pn->empty_children += (n == NULL) - (on == NULL);
  pn->full_children += IsFull(pn, n) - IsFull(pn, on);

  assert(pn->empty_children < 0x80000000);
  assert(pn->full_children < 0x80000000);
}

bool RoutingTrie32::Insert(uint32 ip, int cidr, Value *valuep) {
  uint32 key = ip;
  Node **nn = &root_, *n = root_, *pn = NULL, *leaf, *tn = NULL, *leaf_to_free = NULL;
  uint8 leaf_pos = 32 - cidr;
  
  if (n == NULL) {
    root_ = NewLeaf(key, leaf_pos, exch_null(*valuep));
    return false;
  }
  assert(!NODE_IS_OLEAF(n));

  for (;;) {
    uint32 index = GET_INDEX(key, n);
    if (index >> n->bits) {
force_add:
      // n is a node and the key doesn't match, allocate a new node
      // with two elements and insert it.
      if (!(tn = NewNode(key, FindLastSetBit32(key ^ n->key), 1)))
        return false;
      tn->parent = pn;
      // can convert leaf node to oleaf?
      if (IS_LEAF(n)) {
        if (tn->pos == n->pos && n->leaf_next == NULL) {
          leaf_to_free = n;
          n = VALUE_TO_OLEAF(n->leaf_value);
        }
      }
      PutChild(tn, GET_INDEX(key, tn) ^ 1, n);
      break;
    }
    if (IS_LEAF(n)) {
      if (key != n->key)
        goto force_add;
      return InsertLeafInto(nn, leaf_pos, valuep);
    }
    pn = n;
    nn = &n->child[index];
    if ((n = *nn) == NULL) {
      tn = pn;
      break;
    }
    if (NODE_IS_OLEAF(n)) {
      if (!(n = NewLeaf(pn->key + (index << pn->pos), pn->pos, VALUE_FROM_OLEAF(n))))
        return false;
      *nn = n;
    }
  }
  Value value = *valuep;
  // Create either leaf or oleaf
  if (tn->pos == leaf_pos) {
    leaf = VALUE_TO_OLEAF(value);
  } else if (!(leaf = NewLeaf(key, leaf_pos, value))) {
    if (tn != pn)
      FreeNode(tn);
    return false;
  }
  // -- Start making irreversible changes here
  *valuep = NULL;
  if (leaf_to_free)
    FreeNode(leaf_to_free);

  if (tn != pn) {
    if (!NODE_IS_OLEAF(n) && !IS_LEAF(n))
      n->parent = tn;

    if (pn) {
      PutChild(pn, GET_INDEX(key, pn), tn);
    } else {
      root_ = tn;
    }
    pn = tn;
  }

  PutChild(pn, GET_INDEX(key, pn), leaf);

  Rebalance(pn);
  return true;
}

bool RoutingTrie32::Delete(uint32 ip, int cidr) {
  uint32 key = ip;

  Node *n = root_, *pn = NULL;
  uint32 pn_index = 0;

  if (n == NULL)
    return false;

  uint8 leaf_pos = 32 - cidr;

  for (;;) {
    uint32 index = GET_INDEX(key, n);
    if (index >> n->bits)
      return false;
    if (IS_LEAF(n)) {
      if (n->key != key)
        return false;
      if (n->pos == leaf_pos) {
        if (pn == NULL) {
          root_ = n->leaf_next;
        } else {
          PutChild(pn, pn_index, n->leaf_next);
          if (n->leaf_next == NULL)
            Rebalance(pn);
        }
        FreeNode(n);
        return true;
      }
      Node **nn = &n->leaf_next;
      while (*nn) {
        if ((*nn)->pos == leaf_pos) {
          *nn = (*nn)->leaf_next;
          FreeNode(*nn);
          return true;
        }
        nn = &(*nn)->leaf_next;
      }
      return false;
    }
    pn = n;
    pn_index = index;
    n = n->child[index];
    if (NODE_IS_NULL_OR_OLEAF(n)) {
      if (n && key == pn->key + (index << pn->pos) && pn->pos == leaf_pos) {
        PutChild(pn, index, NULL);
        Rebalance(pn);
        return true;
      }
      return false;
    }
  }
}

RoutingTrie32::Value RoutingTrie32::LookupExact(uint32 ip, int cidr) {
  uint32 key = ip;
  Node *n = root_, *pn;
  if (n == NULL)
    return NULL;
  uint8 leaf_pos = 32 - cidr;
  for (;;) {
    uint32 index = GET_INDEX(key, n);
    if (index >> n->bits)
      return NULL;
    if (IS_LEAF(n)) {
      if (n->key != key)
        return NULL;
      do {
        if (n->pos == leaf_pos)
          return n->leaf_value;
        n = n->leaf_next;
      } while (n);
      return NULL;
    }
    pn = n;
    n = n->child[index];
    if (NODE_IS_NULL_OR_OLEAF(n))
      return (n && key == pn->key + (index << pn->pos) && pn->pos == leaf_pos) ? n->leaf_value : NULL;
  }
}

void RoutingTrie32::Rebalance(Node *n) {
  // Always resize |n| and its parent.  For each parent where
  // Resize returns true resize also its parent.
  Node *np = n->parent;
  Resize(n);
  while (np) {
    n = np;
    np = n->parent;
    if (!Resize(n))
      break;
  }
}

void RoutingTrie32::ResizeChildren(Node *pn) {
  for (uint32 i = 0, i_end = 1U << pn->bits; i != i_end; i++) {
    Node *n = pn->child[i];
    if (IsFull(pn, n))
      Resize(n);
  }
}

enum {
  kHalveThreshold = 25,
  kInflateThreshold = 50,
  kHalveThresholdRoot = 15,
  kInflateThresholdRoot = 30,
};

bool RoutingTrie32::Resize(Node *n) {
  assert(!IS_LEAF(n));

  Node **pn = n->parent ? &n->parent->child[GET_INDEX(n->key, n->parent)] : &root_;
  bool did_work = false;

  if (n->empty_children >= (1U << n->bits) - 1) {
    Collapse(pn);
    n = *pn;
    if (n == NULL || IS_LEAF(n))
      return true;
    did_work = true;
  }

  bool did_inflate = false;

  // Double as long as the resulting node has a number of
  // nonempty nodes that are above the threshold.
  while ((n->full_children > 0 && 50 * (n->full_children + (1U << n->bits) - n->empty_children) >=
    (n->parent && !(n->pos == 9) ? kInflateThreshold : kInflateThresholdRoot) * (1U << n->bits)) && n->bits < 16) {
    if (!Inflate(pn))
      break;
    n = *pn;
    did_work = true;
    did_inflate = true;
  }

  // Halve as long as the number of empty children in this
  // node is above threshold.
  while (n->bits > 1 && 100 * ((1U << n->bits) - n->empty_children) <
    (n->parent && !(n->pos == 8) ? kHalveThreshold : kHalveThresholdRoot) * (1U << n->bits)) {
    assert(!did_inflate);
    if (!Halve(pn))
      break;

    n = *pn;
    did_work = true;
  }

  if (n->empty_children >= (1U << n->bits) - 1) {
    Collapse(pn);
    n = *pn;
    if (n == NULL || IS_LEAF(n))
      return true;
    did_work = true;
  }

  if (did_work)
    ResizeChildren(n);

  return did_work;
}

void RoutingTrie32::UpdateParent(Node *pn) {
  uint32 i_end = 1U << pn->bits;
  uint32 mask_of_halves = 0;

  for (uint32 i = 0; i != i_end; i++) {
    Node *n = pn->child[i];
    if (n == NULL)
      continue;

    mask_of_halves |= (i & 1) + 1;

    if (NODE_IS_NULL_OR_OLEAF(n) || IS_LEAF(n))
      continue;
    Node *op = n->parent;
    n->parent = pn;
    if (op == NULL)
      UpdateParent(n);
  }
  // Collapse right away if there's too many children
  if (pn->empty_children + 1 >= i_end) {
    Collapse(pn->parent ? &pn->parent->child[GET_INDEX(pn->key, pn->parent)] : &root_);
    return;
  }

  if (mask_of_halves != 3) {
    // Only one half of the entries are actually used. Perform a halving operation.
    Halve(pn->parent ? &pn->parent->child[GET_INDEX(pn->key, pn->parent)] : &root_);
  }
}

RoutingTrie32::Node *RoutingTrie32::ConvertOleafToLeaf(Node *pn, uint32 i, Node *n) {
  return NewLeaf(pn->key + (i << pn->pos), pn->pos, VALUE_FROM_OLEAF(n));
}

// The |parent| and |leaf_next| pointers are repurposed to hold
// the next pointer in the free list.
class FreeableNodeCollector {
public:
  FreeableNodeCollector() : ptr_(NULL) {}

  void Add(RoutingTrie32::Node *n) {
    n->leaf_next = ptr_;
    ptr_ = n;
  }

  void Revert(RoutingTrie32::Node *reset_parent_to);
  void Free();
private:
  RoutingTrie32::Node *ptr_;
};

void FreeableNodeCollector::Revert(RoutingTrie32::Node *reset_parent_to) {
  for (RoutingTrie32::Node *p = ptr_, *pn; p != NULL; p = pn) {
    pn = p->parent;
    p->parent = reset_parent_to;
  }
}

void FreeableNodeCollector::Free() {
  for (RoutingTrie32::Node *p = ptr_, *pn; p != NULL; p = pn) {
    pn = p->parent;
    FreeNode(p);
  }
}

void RoutingTrie32::ReplaceChild(RoutingTrie32::Node **pnp, RoutingTrie32::Node *n) {
  RoutingTrie32::Node *pn = *pnp;
  if (pn->parent)
    RoutingTrie32::PutChild(pn->parent, (uint32)(pnp - pn->parent->child), n);
  else
    *pnp = n;
}

bool RoutingTrie32::Inflate(Node **pnp) {
  Node *pn = *pnp, *n0, *n1;
  FreeableNodeCollector free_on_failure, free_on_success, free_on_success_null;
  Node *tn = NewNode(pn->key, pn->pos - 1, pn->bits + 1);
  if (!tn)
    return false;
  tn->parent = pn->parent;


  uint8 oleaf_compare_value = tn->pos;

  for (uint32 i = 0, i_end = 1U << pn->bits; i != i_end; i++) {
    Node *n = pn->child[i];
    // An empty child
    if (n == NULL)
      continue;

    if (NODE_IS_OLEAF(n)) {
      // Convert oleaf to leaf as parent's |pos| changed
      if (!(n = ConvertOleafToLeaf(pn, i, n)))
        goto nomem;
      free_on_failure.Add(n);
      goto insert_child;
    }

    if (IS_LEAF(n)) {
      // Check whether the leaf can be converted to an oleaf.
      if (n->pos == oleaf_compare_value && n->leaf_next == NULL) {
        free_on_success_null.Add(n);
        PutChild(tn, GET_INDEX(n->key, tn), VALUE_TO_OLEAF(n->leaf_value));
        continue;
      }
      goto insert_child;
    }

    // A leaf or an internal node with skipped bits
    if ((n->pos + n->bits) != pn->pos) {
insert_child:
      PutChild(tn, GET_INDEX(n->key, tn), n);
      continue;
    }
    free_on_success.Add(n);
    // Copying oleaf from here is ok as pos is unchanged.
    if (n->bits == 1) {
      // An internal node with exactly two children
      n0 = n->child[0];
      n1 = n->child[1];
    } else {
      // An internal node with more than two children
      if (!(n1 = NewNode(n->key | (1 << tn->pos), n->pos, n->bits - 1)))
        goto nomem;
      free_on_failure.Add(n1);
      if (!(n0 = NewNode(n->key, n->pos, n->bits - 1)))
        goto nomem;
      free_on_failure.Add(n0);
      uint32 j_end = 1U << (n->bits - 1);
      for (uint32 j = 0; j != j_end; j++) {
        PutChild(n0, j, n->child[j]);
        PutChild(n1, j, n->child[j + j_end]);
      }
    }
    PutChild(tn, 2 * i + 0, n0);
    PutChild(tn, 2 * i + 1, n1);
  }

  free_on_success.Free();
  free_on_success_null.Free();
  free_on_failure.Revert(NULL);

  ReplaceChild(pnp, tn);
  UpdateParent(tn);
  FreeNode(pn);
  return true;

nomem:
  free_on_success.Revert(pn);
  free_on_success_null.Revert(NULL);
  free_on_failure.Free();
  FreeNode(tn);
  return false;
}

bool RoutingTrie32::Halve(Node **pnp) {
  Node *pn = *pnp, *n;
  Node *tn = NewNode(pn->key, pn->pos + 1, pn->bits - 1);
  FreeableNodeCollector free_on_failure, free_on_success_null;
  if (!tn)
    return false;
  tn->parent = pn->parent;

  uint8 oleaf_compare_value = tn->pos;

  for (uint32 i = 0, i_end = 1U << pn->bits; i != i_end; i += 2) {
    Node *n0 = pn->child[i + 0];
    Node *n1 = pn->child[i + 1];

    if (n0 == NULL || n1 == NULL) {
      // At least one of the children is empty.
      n = n0 ? n0 : n1;

      if (NODE_IS_OLEAF(n)) {
        // Convert oleaf to leaf as parent's |pos| changed
        if (!(n = ConvertOleafToLeaf(pn, i + (n0 == NULL), n)))
          goto nomem;
        free_on_failure.Add(n);
      } else if (n && IS_LEAF(n) && n->pos == oleaf_compare_value && n->leaf_next == NULL) {
        // The leaf can be converted to an oleaf.
        free_on_success_null.Add(n);
        n = VALUE_TO_OLEAF(n->leaf_value);
      }
    } else {
      // Two nonempty children
      if (!(n = NewNode(pn->key + (i << pn->pos), pn->pos, 1)))
        goto nomem;
      free_on_failure.Add(n);
      PutChild(n, 0, n0);
      PutChild(n, 1, n1);
    }
    PutChild(tn, i / 2, n);
  }
  free_on_failure.Revert(NULL);
  free_on_success_null.Free();
  ReplaceChild(pnp, tn);
  UpdateParent(tn);
  FreeNode(pn);
  return true;

nomem:
  free_on_failure.Free();
  free_on_success_null.Revert(NULL);
  FreeNode(tn);
  return false;
}

void RoutingTrie32::Collapse(Node **pnp) {
  Node *pn = *pnp, *n = NULL;

  if (pn->empty_children != (1U << pn->bits)) {
    for (uint32 i = 0; ; i++) {
      n = pn->child[i];
      if (n) {
        if (NODE_IS_OLEAF(n)) {
          if (!(n = ConvertOleafToLeaf(pn, i, n)))
            return;
        } else if (!IS_LEAF(n)) {
          n->parent = pn->parent;
        }
        break;
      }
    }
  }
  ReplaceChild(pnp, n);
  FreeNode(pn);
}
