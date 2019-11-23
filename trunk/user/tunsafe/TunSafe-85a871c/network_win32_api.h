// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once

#include "stdafx.h"
#include "tunsafe_types.h"
#include "wireguard.h"
#include <functional>

struct StatsCollector {
public:
  enum {
    CHANNELS = 2,
    TIMEVALS = 4,
  };
  StatsCollector() { Init(); }
  void AddSamples(float data[CHANNELS]);
  struct TimeSeries {
    float *data;
    int size;
    int shift;
  };
  const TimeSeries *GetTimeSeries(int channel, int timeval) { return &accum_[channel][timeval].data; }
private:
  struct Accumulator {
    float acc;
    int acc_count;
    int acc_max;
    bool dirty;
    TimeSeries data;
  };
  void Init();
  static void AddToGraphDataSource(StatsCollector::TimeSeries *ts, float value);
  static void AddToAccumulators(StatsCollector::Accumulator *acc, float rval);
  Accumulator accum_[CHANNELS][TIMEVALS];
};

struct LinearizedGraph {
  uint32 total_size;
  uint32 graph_type;
  uint8 num_charts;
  uint8 reserved[7];
};

class TunsafeBackend {
public:
  // All codes < 0 are permanent errors
  enum StatusCode {
    kStatusStopped = 0,
    kStatusInitializing = 1,
    kStatusConnecting = 2,
    kStatusConnected = 3,
    kStatusReconnecting = 4,
    kStatusTunRetrying = 10,

    kErrorInitialize = -1,
    kErrorServiceLost = -3,
  };

  static bool IsPermanentError(StatusCode status) {
    return (int32)status < 0;
  }

  class Delegate {
  public:
    virtual ~Delegate();
    virtual void OnGetStats(const WgProcessorStats &stats) = 0;
    virtual void OnGraphAvailable() = 0;
    virtual void OnStateChanged() = 0;
    virtual void OnClearLog() = 0;
    virtual void OnLogLine(const char **s) = 0;
    virtual void OnStatusCode(TunsafeBackend::StatusCode status) = 0;
    virtual void OnConfigurationProtocolReply(uint32 ident, const std::string &&reply) = 0;
    // This function is needed for CreateTunsafeBackendDelegateThreaded,
    // It's expected to be called on the main thread and then all callbacks will arrive
    // on the right thread.
    virtual void DoWork();
  };

  TunsafeBackend();
  virtual ~TunsafeBackend();

  // Setup/teardown the connection to the local service (if any)
  virtual bool Configure() = 0;
  virtual void Teardown() = 0;

  // Set the name of the tun adapter that we want to use.
  // On Windows this is the guid of the adapter.
  // After having called this, this tun name cannot be used by any other instances.
  // Returns false if the name can't be exclusively reserved to this adapter.
  virtual bool SetTunAdapterName(const char *name) = 0;

  virtual void Start(const char *config_file) = 0;
  virtual void Stop() = 0;
  virtual void RequestStats(bool enable) = 0;
  virtual void ResetStats() = 0;

  virtual InternetBlockState GetInternetBlockState() = 0;
  virtual void SetInternetBlockState(InternetBlockState s) = 0;

  virtual void SetServiceStartupFlags(uint32 flags) = 0;
  virtual std::string GetConfigFileName() = 0;
  virtual LinearizedGraph *GetGraph(int type) = 0;
  virtual void SendConfigurationProtocolPacket(uint32 identifier, const std::string &&message) = 0;

  // Returns a nonzero value whenever a token is requested,
  // as a reply to OnStateChanged
  virtual uint32 GetTokenRequest() = 0;
  // Called when the UI answers the token request
  virtual void SubmitToken(const std::string &&message) = 0;

  bool is_started() { return is_started_; }
  bool is_remote() { return is_remote_; }
  const uint8 *public_key() { return public_key_; }

  StatusCode status() { return status_; }
  uint32 GetIP() { return ipv4_ip_; }

  static TunsafeBackend *FindBackendByTunGuid(const char *guid);
  static char *GetAllGuid();

protected:
  bool is_started_;
  bool is_remote_;
  StatusCode status_;
  uint32 ipv4_ip_;
  uint8 public_key_[32];
};

TunsafeBackend *CreateNativeTunsafeBackend(TunsafeBackend::Delegate *delegate);
TunsafeBackend::Delegate *CreateTunsafeBackendDelegateThreaded(TunsafeBackend::Delegate *delegate, const std::function<void(void)> &callback);
