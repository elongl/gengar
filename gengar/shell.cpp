#include <windows.h>
#include <iostream>
#include "utils.h"

#define PIPE_BUFSIZE 8192

struct pipe_streams {
  HANDLE rd;
  HANDLE wr;
};

STARTUPINFOA get_pipe_startup_info(pipe_streams pipe) {
  STARTUPINFOA startup_info{};
  startup_info.hStdOutput = pipe.wr;
  startup_info.hStdError = pipe.wr;
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  return startup_info;
}

pipe_streams create_output_pipe() {
  pipe_streams pipe;
  SECURITY_ATTRIBUTES secattrs{};
  secattrs.nLength = sizeof(secattrs);
  secattrs.bInheritHandle = true;
  if (!CreatePipe(&pipe.rd, &pipe.wr, &secattrs, 0)) {
    std::cerr << "Failed to create pipe: " << get_last_error_msg() << std::endl;
  }
  return pipe;
}

std::string read_output_pipe(pipe_streams pipe) {
  unsigned long bytes_read;
  char buf[PIPE_BUFSIZE];
  std::string output;
  CloseHandle(pipe.wr);
  while (true) {
    if (ReadFile(pipe.rd, &buf, PIPE_BUFSIZE, &bytes_read, 0)) {
      output.append(buf, bytes_read);
    } else {
      if (bytes_read == 0) {
        break;
      } else {
        std::cerr << "Failed to read output pipe: " << get_last_error_msg()
                  << std::endl;
      }
    }
  }
  CloseHandle(pipe.rd);
  return output;
}

std::string shell(std::string cmd) {
  PROCESS_INFORMATION proc_info;
  pipe_streams pipe = create_output_pipe();
  auto startup_info = get_pipe_startup_info(pipe);
  std::cout << "Running: " << cmd << std::endl;
  if (!CreateProcessA(nullptr, ("cmd.exe /c " + cmd).data(), nullptr, nullptr,
                      true, CREATE_NO_WINDOW, nullptr, nullptr, &startup_info,
                      &proc_info)) {
    std::cerr << "Failed to create process: " << get_last_error_msg()
              << std::endl;
  };
  auto output = read_output_pipe(pipe);
  WaitForSingleObject(proc_info.hProcess, INFINITE);
  CloseHandle(proc_info.hProcess);
  CloseHandle(proc_info.hThread);
  return output;
}