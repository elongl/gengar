#include <windows.h>
#include <iostream>

#define PIPE_BUFSIZE 4096

const std::string CMD_PATH = "C:\\Windows\\System32\\cmd.exe";

HANDLE out_rd = NULL;
HANDLE out_wr = NULL;

STARTUPINFOA GetStartupInfo()
{
	STARTUPINFOA startup_info;
	startup_info.hStdInput = out_wr;
	startup_info.hStdOutput = out_rd;
	startup_info.hStdError = out_rd;
	startup_info.dwFlags = STARTF_USESTDHANDLES;
	return startup_info;
}

void CreateOutputPipe()
{
	SECURITY_ATTRIBUTES secattrs;
	secattrs.bInheritHandle = true;
	CreatePipe(&out_rd, &out_wr, &secattrs, 0);
}

std::string ReadOutputPipe()
{
	unsigned long bytes_read;
	char buf[PIPE_BUFSIZE];
	std::string output;
	while (true)
	{
		bool succeed = ReadFile(out_rd, &buf, PIPE_BUFSIZE, &bytes_read, 0);
		if (succeed)
		{
			if (bytes_read == 0) { break; }
			else { output.append(buf, bytes_read); }
		}
		else
		{
			std::cerr << "Failed to read output pipe: " << GetLastError() << std::endl;
		}
	}
	return output;
}

std::string RunShellCommand(std::string cmd)
{
	std::cout << "Running: " << cmd << std::endl;
	PROCESS_INFORMATION proc_info;
	CreateProcessA(
		CMD_PATH.data(),
		("/c " + cmd).data(),
		nullptr,
		nullptr,
		true,
		CREATE_NO_WINDOW,
		nullptr,
		nullptr,
		&GetStartupInfo(),
		&proc_info
	);
	CloseHandle(proc_info.hThread);
	CloseHandle(proc_info.hProcess);
	CloseHandle(out_wr);
	std::string output = ReadOutputPipe();
	CloseHandle(out_rd);
	return output;
}