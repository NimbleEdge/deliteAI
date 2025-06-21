/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nimble_exec_info.hpp"

#include <dlfcn.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <unwind.h>

#ifndef __APPLE__
#include <elf.h>
#include <link.h>
#include <linux/memfd.h>
#endif  // __APPLE__

#include <cstring>
#include <string>

#include "core_utils/fmt.hpp"
#include "native_interface.hpp"

namespace detail {

/**
 * [backtrace(3)](https://man7.org/linux/man-pages/man3/backtrace.3.html)
 * Saves a backtrace for the current call in the array pointed to by buffer.
 * "size" indicates the maximum number of void* pointers that can be set.
 *
 * Returns the number of addresses stored in "buffer", which is not greater
 * than "size". If the return value is equal to "size" then the number of
 * addresses may have been truncated.
 */
int backtrace(void* _Nonnull* _Nonnull buffer, int size);

/**
 * [backtrace_symbols(3)](https://man7.org/linux/man-pages/man3/backtrace_symbols.3.html)
 * Given an array of void* pointers, translate the addresses into an array
 * of strings that represent the backtrace.
 *
 * Returns a pointer to allocated memory, on error NULL is returned. It is
 * the responsibility of the caller to free the returned memory.
 */
char* _Nullable* _Nullable backtrace_symbols(void* _Nonnull const* _Nonnull buffer, int size);

/**
 * [backtrace_symbols_fd(3)](https://man7.org/linux/man-pages/man3/backtrace_symbols_fd.3.html)
 * Given an array of void* pointers, translate the addresses into an array
 * of strings that represent the backtrace and write to the file represented
 * by "fd". The file is written such that one line equals one void* address.
 */
void backtrace_symbols_fd(void* _Nonnull const* _Nonnull buffer, int size, int fd);

}  // namespace detail

namespace ne {

#ifndef __APPLE__

// Callback function for dl_iterate_phdr to find the base address of the main executable
static int callback(struct dl_phdr_info* info, size_t size, void* data) {
  if (info->dlpi_name && std::strstr(info->dlpi_name, LIBNIMBLENET_NAME) != nullptr) {
    for (int i = 0; i < info->dlpi_phnum; i++) {
      const ElfW(Phdr)* hdr = &info->dlpi_phdr[i];
      // printf("flags: 0x%08X \t addr: 0x%llx\n", hdr->p_flags, info->dlpi_addr + hdr->p_vaddr);
      if (hdr->p_type == PT_LOAD &&
          hdr->p_flags == 0x5) {  // TEXT section is type PT_LOAD with flags as 'rx'
        *((unsigned long*)data) = info->dlpi_addr + hdr->p_vaddr;
        return 1;  // stop iteration
      }
    }
  }
  return 0;  // Continue iteration
}

// Function to retrieve the base address of the executable
unsigned long get_base_address() {
  unsigned long base_address = 0;
  dl_iterate_phdr(callback, &base_address);
  return base_address;
}

#else  // __APPLE__

// stub for apple since we don't have link.h available, atleast till a suitable replacement is found
unsigned long get_base_address() { return 0; }

#endif  // __APPLE__

static bool receivedSignal = false;

void handle_crash_signal(int signum, siginfo_t* info, void* context) {
  if (receivedSignal) {
    return;
  }
  receivedSignal = true;

#ifdef __APPLE__
  std::string backtraceString = "NOT_SUPPORTED";
#else   // __APPLE__
  std::string delimiter = "$$$";

  void* buffer[128];  // Increase or decrease this based on your needs
  std::string backtraceString =
      ne::fmt("BASE_ADDR: [0x%lx]%s", get_base_address(), delimiter.c_str()).str;

  int frames = detail::backtrace(buffer, sizeof(buffer) / sizeof(buffer[0]));

  char** stackTrace = detail::backtrace_symbols(buffer, frames);
  ;
  for (int i = 0; i < frames; i++) {
    backtraceString += std::string(stackTrace[i]);
    backtraceString += delimiter;
  }
#endif  // __APPLE__

  auto errorMessage = ne::fmt(
      "METRICS::: 2024-04-08 16:32:53.472 ::: crash ::: "
      "{\"errorCode\":%d,\"backtrace\":\"%s\",\"signalCode\":%d}",
      signum, backtraceString.c_str(), info->si_code);
  nativeinterface::save_file_on_device_common(errorMessage.str, "segfault.log");
  exit(1);
}
}  // namespace ne

#ifndef __APPLE__

// IMPLEMENTATION DETAILS::
namespace detail {

class ErrnoRestorer {
 public:
  explicit ErrnoRestorer() : saved_errno_(errno) {}

  ~ErrnoRestorer() { errno = saved_errno_; }

  void override(int new_errno) { saved_errno_ = new_errno; }

 private:
  int saved_errno_;
};

class ScopedFd final {
 public:
  explicit ScopedFd(int fd) : fd_(fd) {}

  ScopedFd() : fd_(-1) {}

  ~ScopedFd() { reset(-1); }

  void reset(int fd = -1) {
    if (fd_ != -1) {
      ErrnoRestorer e;
      close(fd_);
    }
    fd_ = fd;
  }

  int get() const { return fd_; }

 private:
  int fd_;
};

struct StackState {
  void** frames;
  int frame_count;
  int cur_frame = 0;

  StackState(void** frames, int frame_count) : frames(frames), frame_count(frame_count) {}
};

static _Unwind_Reason_Code TraceFunction(_Unwind_Context* context, void* arg) {
  // The instruction pointer is pointing at the instruction after the return
  // call on all architectures.
  // Modify the pc to point at the real function.
  uintptr_t ip = _Unwind_GetIP(context);
  if (ip != 0) {
#if defined(__arm__)
    // If the ip is suspiciously low, do nothing to avoid a segfault trying
    // to access this memory.
    if (ip >= 4096) {
      // Check bits [15:11] of the first halfword assuming the instruction
      // is 32 bits long. If the bits are any of these values, then our
      // assumption was correct:
      //  b11101
      //  b11110
      //  b11111
      // Otherwise, this is a 16 bit instruction.
      uint16_t value = (*reinterpret_cast<uint16_t*>(ip - 2)) >> 11;
      if (value == 0x1f || value == 0x1e || value == 0x1d) {
        ip -= 4;
      } else {
        ip -= 2;
      }
    }
#elif defined(__aarch64__)
    // All instructions are 4 bytes long, skip back one instruction.
    ip -= 4;
#elif defined(__i386__) || defined(__x86_64__)
    // It's difficult to decode exactly where the previous instruction is,
    // so subtract 1 to estimate where the instruction lives.
    ip--;
#endif
  }

  StackState* state = static_cast<StackState*>(arg);
  state->frames[state->cur_frame++] = reinterpret_cast<void*>(ip);
  return (state->cur_frame >= state->frame_count) ? _URC_END_OF_STACK : _URC_NO_REASON;
}

int backtrace(void** buffer, int size) {
  if (size <= 0) {
    return 0;
  }

  StackState state(buffer, size);
  _Unwind_Backtrace(TraceFunction, &state);
  return state.cur_frame;
}

char** backtrace_symbols(void* const* buffer, int size) {
  if (size <= 0) {
    return nullptr;
  }
  // Do this calculation first in case the user passes in a bad value.
  size_t ptr_size;
  if (__builtin_mul_overflow(sizeof(char*), size, &ptr_size)) {
    return nullptr;
  }

  ScopedFd fd(syscall(SYS_memfd_create, "backtrace_symbols_fd", MFD_CLOEXEC));
  if (fd.get() == -1) {
    return nullptr;
  }
  backtrace_symbols_fd(buffer, size, fd.get());

  // Get the size of the file.
  off_t file_size = lseek(fd.get(), 0, SEEK_END);
  if (file_size <= 0) {
    return nullptr;
  }

  // The interface for backtrace_symbols indicates that only the single
  // returned pointer must be freed by the caller. Therefore, allocate a
  // buffer that includes the memory for the strings and all of the pointers.
  // Add one byte at the end just in case the file didn't end with a '\n'.
  size_t symbol_data_size;
  if (__builtin_add_overflow(ptr_size, file_size, &symbol_data_size) ||
      __builtin_add_overflow(symbol_data_size, 1, &symbol_data_size)) {
    return nullptr;
  }

  uint8_t* symbol_data = reinterpret_cast<uint8_t*>(malloc(symbol_data_size));
  if (symbol_data == nullptr) {
    return nullptr;
  }

  // Copy the string data into the buffer.
  char* cur_string = reinterpret_cast<char*>(&symbol_data[ptr_size]);
  // If this fails, the read won't read back the correct number of bytes.
  lseek(fd.get(), 0, SEEK_SET);
  ssize_t num_read = read(fd.get(), cur_string, file_size);
  fd.reset(-1);
  if (num_read != file_size) {
    free(symbol_data);
    return nullptr;
  }

  // Make sure the last character in the file is '\n'.
  if (cur_string[file_size] != '\n') {
    cur_string[file_size++] = '\n';
  }

  for (int i = 0; i < size; i++) {
    (reinterpret_cast<char**>(symbol_data))[i] = cur_string;
    cur_string = strchr(cur_string, '\n');
    if (cur_string == nullptr) {
      free(symbol_data);
      return nullptr;
    }
    cur_string[0] = '\0';
    cur_string++;
  }
  return reinterpret_cast<char**>(symbol_data);
}

// This function should do no allocations if possible.
void backtrace_symbols_fd(void* const* buffer, int size, int fd) {
  if (size <= 0 || fd < 0) {
    return;
  }

  for (int frame_num = 0; frame_num < size; frame_num++) {
    void* address = buffer[frame_num];
    Dl_info info;
    if (dladdr(address, &info) != 0) {
      if (info.dli_fname != nullptr) {
        write(fd, info.dli_fname, strlen(info.dli_fname));
      }
      if (info.dli_sname != nullptr) {
        dprintf(fd, "(%s+0x%" PRIxPTR ") ", info.dli_sname,
                reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(info.dli_saddr));
      } else {
        dprintf(fd, "(+%p) ", info.dli_saddr);
      }
    }

    dprintf(fd, "[%p]\n", address);
  }
}

}  // namespace detail

#endif  // __APPLE__
