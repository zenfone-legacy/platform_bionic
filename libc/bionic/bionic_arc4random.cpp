/*
 * Copyright (C) 2016 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "private/bionic_arc4random.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/auxv.h>
#include <syscall.h>
#include <unistd.h>

#include "private/KernelArgumentBlock.h"
#include "private/libc_logging.h"

void __libc_safe_arc4random_buf(void* buf, size_t n, KernelArgumentBlock& args) {
  static bool have_getrandom = syscall(SYS_getrandom, nullptr, 0, 0) == -1 && errno != ENOSYS;
  static bool have_urandom = access("/dev/urandom", R_OK) == 0;
  static size_t at_random_bytes_consumed = 0;

  if (have_getrandom || have_urandom) {
    arc4random_buf(buf, n);
    return;
  }

  if (at_random_bytes_consumed + n > 16) {
    __libc_fatal("ran out of AT_RANDOM bytes, have %zu, requested %zu",
                 16 - at_random_bytes_consumed, n);
  }

  memcpy(buf, reinterpret_cast<char*>(args.getauxval(AT_RANDOM)) + at_random_bytes_consumed, n);
  at_random_bytes_consumed += n;
  return;
}