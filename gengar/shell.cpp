#include <windows.h>
#include <iostream>

#define PIPE_BUFSIZE 8192

HANDLE out_rd = NULL;
HANDLE out_wr = NULL;


std::string GetCmdPath()
{
	char path[1024];
	GetEnvironmentVariableA("comspec", path, sizeof(path));
	return path;
}

STARTUPINFOA GetPipeStartupInfo()
{
	STARTUPINFOA startup_info{};
	startup_info.hStdOutput = out_wr;
	startup_info.hStdError = out_wr;
	startup_info.dwFlags = STARTF_USESTDHANDLES;
	return startup_info;
}

void CreateOutputPipe()
{
	SECURITY_ATTRIBUTES secattrs{};
	secattrs.bInheritHandle = true;
	if (!CreatePipe(&out_rd, &out_wr, &secattrs, 0))
	{
		std::cerr << "Failed to create pipe: " << GetLastError() << std::endl;
	}
}

std::string ReadOutputPipe()
{
	unsigned long bytes_read;
	char buf[PIPE_BUFSIZE];
	std::string output;
	while (true)
	{
		if (ReadFile(out_rd, &buf, PIPE_BUFSIZE, &bytes_read, 0))
		{
			if (bytes_read == 0) { break; }
			else
			{
				output.append(buf, bytes_read);
				if (bytes_read < PIPE_BUFSIZE) { break; }
			}
		}
		else { std::cerr << "Failed to read output pipe: " << GetLastError() << std::endl; }
	}
	return output;
}

std::string RunShellCommand(std::string cmd)
{
	PROCESS_INFORMATION proc_info;
	CreateOutputPipe();
	auto startup_info = GetPipeStartupInfo();
	auto cmdline = "/c " + cmd;
	std::cout << "Running: " << cmd << std::endl;
	if (!CreateProcessA(
		GetCmdPath().data(),
		cmdline.data(),
		nullptr,
		nullptr,
		true,
		CREATE_NO_WINDOW,
		nullptr,
		nullptr,
		&startup_info,
		&proc_info
	))
	{
		std::cerr << "Failed to create process: " << GetLastError() << std::endl;
	};
	WaitForSingleObject(proc_info.hProcess, INFINITE);
	CloseHandle(proc_info.hProcess);
	CloseHandle(proc_info.hThread);
	CloseHandle(out_wr);
	auto output = ReadOutputPipe();
	CloseHandle(out_rd);
	return output;
}