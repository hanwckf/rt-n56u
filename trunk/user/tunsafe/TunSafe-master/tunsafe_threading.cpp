// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#include "stdafx.h"
#include "tunsafe_threading.h"
#include <stdlib.h>
#include <assert.h>

#if defined(OS_POSIX)
Thread::Thread() {
  thread_ = 0;
}

Thread::~Thread() {
  assert(thread_ == 0);
}

static void *ThreadMainStatic(void *x) {
  Thread::Runner *t = (Thread::Runner*)x;
  t->ThreadMain();
  return 0;
}

void Thread::StartThread(Runner *runner) {
  assert(thread_ == 0);
  if (pthread_create(&thread_, NULL, &ThreadMainStatic, runner) != 0)
    tunsafe_die("pthread_create failed");
}

void Thread::StopThread() {
  if (thread_) {
    void *x;
    pthread_join(thread_, &x);
    thread_ = 0;
  }
}

void Thread::DetachThread() {
  if (thread_) {
    pthread_detach(thread_);
    thread_ = 0;
  }
}

bool Thread::is_started() {
  return thread_ != 0;
}

void ConditionVariable::WaitTimed(Mutex *mutex, int millis) {
#if !defined(OS_MACOSX)
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  ts.tv_sec += (millis / 1000);
  ts.tv_nsec += (millis % 1000) * 1000000;
  if (ts.tv_nsec >= 1000000000) {
    ts.tv_nsec -= 1000000000;
    ts.tv_sec++;
  }
  pthread_cond_timedwait(&condvar_, &mutex->lock_, &ts);
#else
  struct timespec ts;
  ts.tv_sec = millis / 1000;
  ts.tv_nsec = (millis % 1000) * 1000000;

  pthread_cond_timedwait_relative_np(&condvar_, &mutex->lock_, &ts);
#endif
}
#endif  // defined(OS_POSIX)

#if defined(OS_WIN)
Thread::Thread() {
  thread_ = 0;
}

Thread::~Thread() {
  assert(thread_ == 0);
}

static DWORD WINAPI ThreadMainStatic(void *x) {
  Thread::Runner *t = (Thread::Runner*)x;
  t->ThreadMain();
  return 0;
}

void Thread::StartThread(Runner *runner) {
  assert(thread_ == 0);
  DWORD thread_id;
  thread_ = CreateThread(NULL, 0, &ThreadMainStatic, (LPVOID)runner, 0, &thread_id);
}

void Thread::StopThread() {
  if (thread_) {
    WaitForSingleObject(thread_, INFINITE);
    CloseHandle(thread_);
    thread_ = 0;
  }
}

void Thread::DetachThread() {
  if (thread_) {
    CloseHandle(thread_);
    thread_ = 0;
  }
}

bool Thread::is_started() {
  return thread_ != 0;
}
#endif

MultithreadedDelayedDelete::MultithreadedDelayedDelete() {
  table_ = NULL;
  num_threads_ = 0;
}

MultithreadedDelayedDelete::~MultithreadedDelayedDelete() {
  assert(curr_.size() == 0);
  assert(next_.size() == 0);
  assert(to_delete_.size() == 0);
  free(table_);
}

void MultithreadedDelayedDelete::Configure(uint32 num_threads) {
  assert(table_ == NULL);
  num_threads_ = num_threads;
  table_ = (CheckpointData*)calloc(sizeof(CheckpointData), num_threads);
}

void MultithreadedDelayedDelete::Add(DoDeleteFunc *func, void *param) {
  if (num_threads_ == 0) {
    func(param);
    return;
  }
  lock_.Acquire();
  Entry e = {func, param};
  curr_.push_back(e);
  lock_.Release();
}

void MultithreadedDelayedDelete::Checkpoint(uint32 thread_id) {
  table_[thread_id].value.store(1);
}

void MultithreadedDelayedDelete::MainCheckpoint() {
  // Wait for all threads to signal that they reached the checkpoint
  for (size_t i = 0; i < num_threads_; i++) {
    if (table_[i].value.load() == 0)
      return;
  }

  // All threads reached the checkpoint, clear the values
  for (size_t i = 0; i < num_threads_; i++)
    table_[i].value.store(0);

  // Swap curr and next, and delete all nexts.
  lock_.Acquire();
  std::swap(curr_, next_);
  std::swap(curr_, to_delete_);
  lock_.Release();

  for (auto it = to_delete_.begin(); it != to_delete_.end(); ++it) {
    it->func(it->param);
  }
  to_delete_.clear();
}
