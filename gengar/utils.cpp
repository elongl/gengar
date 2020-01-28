#include <Windows.h>
#include <system_error>

std::string get_last_error_msg() {
  return std::system_category().message(GetLastError());
}