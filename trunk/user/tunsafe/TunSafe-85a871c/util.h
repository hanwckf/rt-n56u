// SPDX-License-Identifier: AGPL-1.0-only
// Copyright (C) 2018 Ludvig Strigeus <info@tunsafe.com>. All Rights Reserved.
#pragma once
#include "tunsafe_types.h"

char *base64_encode(const uint8 *input, size_t length, char *output, size_t output_size, size_t *actual_size);
bool base64_decode(const uint8 *in, size_t inLen, uint8 *out, size_t *outLen);
bool IsOnlyZeros(const uint8 *data, size_t data_size);

int RunCommand(const char *fmt, ...);
typedef void Logger(int type, const char *msg);
extern Logger *g_logger;


void *memdup(const void *p, size_t size);
char *my_strndup(const char *p, size_t size);

size_t my_strlcpy(char *dst, size_t dstsize, const char *src);

template<typename T, typename U> static inline T postinc(T&x, U v) {
  T t = x;
  x += v;
  return t;
}

template<typename T, typename U> static inline T exch(T&x, U v) {
  T t = x;
  x = v;
  return t;
}

template<typename T> static inline T exch_null(T&x) {
  T t = x;
  x = NULL;
  return t;
}

bool is_space(uint8_t c);
void OsGetRandomBytes(uint8 *dst, size_t dst_size);
bool ParseConfigKeyValue(char *m, std::vector<std::pair<char *, char*>> *result);
bool ParseHexString(const char *text, void *data, size_t data_size);
void PrintHexString(const void *data, size_t data_size, char *result);
void SplitString(char *s, int separator, std::vector<char*> *components);
bool ParseBase64Key(const char *s, uint8 key[32]);


uint64 OsGetMilliseconds();
void InitOsxGetMilliseconds();
void OsInterruptibleSleep(int millis);
void OsGetTimestampTAI64N(uint8 dst[12]);

struct CommandLineOutput {
  const char *filename_to_load;
  const char *interface_name;
  bool daemon;
};
int HandleCommandLine(int argc, char **argv, CommandLineOutput *output);
