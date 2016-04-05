#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <list>

#ifdef _MSC_VER

#include <conio.h>
#include <Windows.h> 

#else

#include <termios.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>

char* itoa(int);
char getch();
bool flagFather = false;

void fFather(int){
	flagFather = true;
}

#endif

using namespace std;

#define WINPATH "..//Debug//Child.exe"
#define LINUXPATH "/home/linux/Рабочий стол/child"
#define SIZE 100
#define DEC 10

int addProcess(int);
void closeProcess(int);
void showProcessInformation(list<int>&);
void closeAllProcess(list<int>&);
void closeAllProcess(int);
void launch();