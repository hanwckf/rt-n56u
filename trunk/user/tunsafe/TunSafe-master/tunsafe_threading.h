// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once
#include "tunsafe_types.h"
#include <atomic>
#include <vector>
#include <assert.h>
#if !defined(OS_WIN)
#include <pthread.h>
#endif  // !defined(OS_WIN)

#if defined(OS_WIN)

class ReaderWriterLock {
public:
  ReaderWriterLock() : lock_(SRWLOCK_INIT) {}
  void AcquireExclusive() { AcquireSRWLockExclusive(&lock_); }
  void AcquireShared()    { AcquireSRWLockShared(&lock_);  }
  void ReleaseExclusive() { ReleaseSRWLockExclusive(&lock_); }
  void ReleaseShared()    { ReleaseSRWLockShared(&lock_); }
private:
  SRWLOCK lock_;
};

class Mutex {
  friend class ConditionVariable;
public:
#if defined(_DEBUG)
  bool locked_;
  bool IsLocked() { return locked_; }
#define Mutex_SETLOCKED(x) locked_ = x;
#else
  bool IsLocked() { return false; }
#define Mutex_SETLOCKED(x) 
#endif
  Mutex() : lock_(SRWLOCK_INIT) { Mutex_SETLOCKED(false); } 
  ~Mutex() {  }
  void Acquire() {
    AcquireSRWLockExclusive(&lock_);
    Mutex_SETLOCKED(true);
  }
  void Release() {
    Mutex_SETLOCKED(false);
    ReleaseSRWLockExclusive(&lock_);
  }
private:
  SRWLOCK lock_;
};

class ConditionVariable {
public:
  ConditionVariable() { InitializeConditionVariable(&condvar_); }
  void Wait(Mutex *mutex) { SleepConditionVariableSRW(&condvar_, &mutex->lock_, INFINITE, 0); }
  void WaitTimed(Mutex *mutex, int millis) { SleepConditionVariableSRW(&condvar_, &mutex->lock_, millis, 0); }
  void Wake() { WakeConditionVariable(&condvar_); }

private:
  CONDITION_VARIABLE condvar_;
};

typedef uint32 ThreadId;

static inline bool CurrentThreadIdEquals(ThreadId thread_id) {
  return thread_id == GetCurrentThreadId();
}

#else  // defined(OS_WIN)

class ReaderWriterLock {
public:
  ReaderWriterLock() {
    if (pthread_rwlock_init(&lock_, NULL) != 0)
      tunsafe_die("pthread_rwlock_init failed");
  }
  ~ReaderWriterLock() {
    pthread_rwlock_destroy(&lock_);
  }
  void AcquireExclusive() { int rv = pthread_rwlock_wrlock(&lock_); assert(rv == 0); }
  void AcquireShared()    { int rv = pthread_rwlock_rdlock(&lock_); assert(rv == 0); }
  void ReleaseExclusive() { int rv = pthread_rwlock_unlock(&lock_); assert(rv == 0); }
  void ReleaseShared()    { int rv = pthread_rwlock_unlock(&lock_); assert(rv == 0); }
private:
  pthread_rwlock_t lock_;
};

class Mutex {
  friend class ConditionVariable;
public:
#if defined(_DEBUG)
  bool locked_;
  bool IsLocked() { return locked_; }
#define Mutex_SETLOCKED(x) locked_ = x;
#else
  bool IsLocked() { return false; }
#define Mutex_SETLOCKED(x) 
#endif
  Mutex() { 
    if (pthread_mutex_init(&lock_, NULL) != 0)
      tunsafe_die("pthread_mutex_init failed");
    Mutex_SETLOCKED(false);
  } 
  ~Mutex() { 
    pthread_mutex_destroy(&lock_);
  }
  void Acquire() {
    int rv = pthread_mutex_lock(&lock_);
    assert(rv == 0);
    Mutex_SETLOCKED(true);
  }
  void Release() {
    Mutex_SETLOCKED(false);
    int rv = pthread_mutex_unlock(&lock_);
    assert(rv == 0);
  }
  pthread_mutex_t *impl() { return &lock_; }
private:
  pthread_mutex_t lock_;
};

class ConditionVariable {
public:
  ConditionVariable() { pthread_cond_init(&condvar_, NULL); }
  void Wait(Mutex *mutex) { pthread_cond_wait(&condvar_, &mutex->lock_); }
  void WaitTimed(Mutex *mutex, int millis);
  void Wake() { pthread_cond_signal(&condvar_); }
private:
  pthread_cond_t condvar_;
};


typedef pthread_t ThreadId;

static inline bool CurrentThreadIdEquals(ThreadId thread_id) {
  return pthread_equal(thread_id, pthread_self()) != 0;
}

static inline ThreadId GetCurrentThreadId() {
  return pthread_self();
}

#endif  // !defined(OS_WIN)

class ScopedLockShared {
public:
  ScopedLockShared(ReaderWriterLock *lock) : lock_(lock) { lock->AcquireShared(); }
  ~ScopedLockShared() { lock_->ReleaseShared(); }
private:
  ReaderWriterLock *lock_;
};

class ScopedLockExclusive {
public:
  ScopedLockExclusive(ReaderWriterLock *lock) : lock_(lock) { lock->AcquireExclusive(); }
  ~ScopedLockExclusive() { lock_->ReleaseExclusive(); }
private:
  ReaderWriterLock *lock_;
};

class ScopedLock {
public:
  ScopedLock(Mutex *lock) : lock_(lock) { lock->Acquire(); }
  ~ScopedLock() { lock_->Release(); }
private:
  Mutex *lock_;
};

class Thread {
public:
  class Runner {
  public:
    virtual void ThreadMain() = 0;
  };
  Thread();
  ~Thread();
  void StartThread(Runner *runner);
  void StopThread();
  void DetachThread();
  bool is_started();

private:
#if defined(OS_WIN)
  HANDLE thread_;
#else  // defined(OS_WIN)
  pthread_t thread_;
#endif  // !defined(OS_WIN)
};


// This class deletes objects delayed. All participating threads will call a function,
// and then once all threads did, all registered objects will get deleted.
class MultithreadedDelayedDelete {
public:
  MultithreadedDelayedDelete();
  ~MultithreadedDelayedDelete();

  // Call this with the # of worker threads that will call the wireguard code
  void Configure(uint32 num_threads);

  // Add something that will be deleted.
  typedef void DoDeleteFunc(void *x);
  void Add(DoDeleteFunc *func, void *param);


  // Call this periodically from worker thread with id |thread_id|, ids start from zero up to num_threads-1
  void Checkpoint(uint32 thread_id);

  // Call this periodically from the main thread to do the actual deletes once the worker threads
  // have reached the checkpoint.
  void MainCheckpoint();

  bool enabled() const { return num_threads_ != 0; }

private:
  struct Entry {
    DoDeleteFunc *func;
    void *param;
  };

  struct CheckpointData {
    std::atomic<uint32> value;
    uint8 align[60];
  };

  uint32 num_threads_;

  std::vector<Entry> curr_, next_, to_delete_;
  CheckpointData *table_;
  Mutex lock_;
};
