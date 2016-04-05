#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <Windows.h>

#else
#include <termios.h>
#include <time.h>
#include <sys/wait.h>
#include <list>
#include <sys/types.h>	
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <string>
#include <signal.h>

bool closeFlag = true;
bool activateFlag = false;

void fActivate(int){
	signal(SIGUSR1, fActivate);
	activateFlag = true;
}

void fClose(int){
	signal(SIGUSR2, fClose);
	closeFlag = false;
}

#endif

using namespace std;

#define SIZE 100