#include <windows.h>
#include <iostream>

#define PIPE_BUFSIZE 8192


struct pipe
{
	HANDLE rd;
	HANDLE wr;
};


STARTUPINFOA GetPipeStartupInfo(struct pipe pipe)
{
	STARTUPINFOA startup_info{};
	startup_info.hStdOutput = pipe.wr;
	startup_info.hStdError = pipe.wr;
	startup_info.dwFlags = STARTF_USESTDHANDLES;
	return startup_info;
}

struct pipe CreateOutputPipe()
{
	struct pipe pipe;
	SECURITY_ATTRIBUTES secattrs{};
	secattrs.nLength = sizeof(secattrs);
	secattrs.bInheritHandle = true;
	if (!CreatePipe(&pipe.rd, &pipe.wr, &secattrs, 0))
	{
		std::cerr << "Failed to create pipe: " << GetLastError() << std::endl;
	}
	return pipe;
}

std::string ReadOutputPipe(struct pipe pipe)
{
	unsigned long bytes_read;
	char buf[PIPE_BUFSIZE];
	std::string output;
	CloseHandle(pipe.wr);
	while (true)
	{
		if (ReadFile(pipe.rd, &buf, PIPE_BUFSIZE, &bytes_read, 0))
		{
			output.append(buf, bytes_read);
		}
		else
		{
			if (bytes_read == 0) { break; }
			else { std::cerr << "Failed to read output pipe: " << GetLastError() << std::endl; }
		}
	}
	CloseHandle(pipe.rd);
	return output;
}

std::string RunShellCommand(std::string cmd)
{
	PROCESS_INFORMATION proc_info;
	struct pipe pipe = CreateOutputPipe();
	auto startup_info = GetPipeStartupInfo(pipe);
	std::cout << "Running: " << cmd << std::endl;
	if (!CreateProcessA(
		nullptr,
		("cmd.exe /c " + cmd).data(),
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
	return ReadOutputPipe(pipe);
}