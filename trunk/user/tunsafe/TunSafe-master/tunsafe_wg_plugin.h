#pragma once
#include "wireguard_proto.h"


enum {
  kTokenRequestType_Mask = 0xff,
  kTokenRequestType_Text = 1,
  kTokenRequestType_Password = 2,
  kTokenRequestType_6digits = 3,
  kTokenRequestType_7digits = 4,
  kTokenRequestType_8digits = 5,


  kTokenRequestStatus_Mask      = 0xff00,
  kTokenRequestStatus_None      = 0x0000,
  kTokenRequestStatus_NotAccepted = 0x0100,
  kTokenRequestStatus_Wrong     = 0x0200,
  kTokenRequestStatus_Locked    = 0x0300,
  kTokenRequestStatus_Ratelimit = 0x0400,
};



class PluginDelegate {
public:
  // Called when 2FA is requested for a peer.
  virtual void OnRequestToken(WgPeer *peer, uint32 type) = 0;
};

class TunsafePlugin : public WgPlugin {
public:
  enum {
    kMaxTokenLen = 128,
  };
  // Called after OnRequest2FA to supply the token.
  virtual void SubmitToken(const uint8 *text, size_t text_len) = 0;
};

TunsafePlugin *CreateTunsafePlugin(PluginDelegate *del, WireguardProcessor *wgp);
