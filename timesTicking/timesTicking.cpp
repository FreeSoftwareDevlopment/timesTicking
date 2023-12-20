#include <iostream>
#include "toLowerCase.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <filesystem>
#include <psapi.h>
#include <string>
#include <chrono>
#include <comdef.h>

bool compare(const wchar_t* x, const wchar_t* y) {
	int l = lstrlenW(x);
	if (l != lstrlenW(y)) return 0;
	for (int xr = 0; xr < l; xr++) {
		if (toLowerChar(x[xr]) != toLowerChar(y[xr])) return 0;
	}
	return 1;
}

std::vector<DWORD> getProcId(const wchar_t* target) {
	std::vector<DWORD> pId;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hsnap, &pe32) == TRUE)
	{
		do {
			if (compare(pe32.szExeFile, target)) {
				if (pe32.th32ProcessID != NULL)
					pId.push_back(pe32.th32ProcessID);
			}
		} while (Process32Next(hsnap, &pe32));
	}
	CloseHandle(hsnap);
	return pId;
}

std::filesystem::path getRelativePath(const char* p) {
	char x[MAX_PATH]{ 0 };
	GetModuleFileNameExA(GetCurrentProcess(), NULL, x, MAX_PATH);
	std::filesystem::path cur = x;
	return cur.parent_path() / p;
}


int wmain(int argc, wchar_t* argv[])
{
	if (argc > 2) {
		auto tiSec = std::stoi(argv[1]);
		std::chrono::high_resolution_clock::time_point c = std::chrono::high_resolution_clock::now();
		std::cout << "Setting Seconds: " << tiSec << "\n";
		while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - c).count() < tiSec)
			Sleep(600);

		std::cout << "Time expired ... Stopping Processes" << std::endl;
		bool killedSomething = false;

		for (int i = 2; i < argc; i++) {
			std::wcout << L"Trying to kill all Instances of \"" << argv[i] << L"\"\n";
			auto pids = getProcId(argv[i]);

			for (size_t _i = 0; _i < pids.size(); _i++) {
				std::wcout << L"\tTry to Kill PID " << pids[_i] << "\n";
				auto process = OpenProcess(PROCESS_TERMINATE, NULL, pids[_i]);
				if (process == NULL) {
					std::cerr << "\t Can't Access " << pids[_i] << "\n";
					continue;
				}
				std::cout << "\t Trying to Brutally Murder the Process " << pids[_i] << "\n";
				if (SUCCEEDED(TerminateProcess(process, NULL))) {
					std::cout << "\t Brutally Murdered the Process " << pids[_i] << "\n";
					killedSomething = true;
				}
				else
					std::cerr << "\t Failed to Brutally Murder the Process " << pids[_i] << "\n";
				CloseHandle(process);
			}
		}

		if (killedSomething)
			MessageBoxW(NULL, L"ByeBye Process!!", L"Process Killer", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
	}
	else {
		_bstr_t b(argv[0]);// Convert to char
		std::cout
			<< "OOOOPS! This Application needs some Arguments:\n\t"
			<< getRelativePath(b)
			<< " <timeInSeconds> <...targetApplicationName>\n";
	}
}
