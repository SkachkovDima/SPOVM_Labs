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

int main(){
	setlocale(LC_ALL, "Russian");
	launch();
	return 0;
}

void launch(){

#ifdef _MSC_VER

	HANDLE fatherEvent = CreateEvent(NULL, false, true,
		"fatherEvent");
	HANDLE childEvent;

	char buffer[SIZE], string[SIZE];
	int number = 0;

	while (true){
		if (kbhit() != 0){
			switch (getch()){
			case '+':
				if (!addProcess(number)){
					exit(0);
				}
				number++;
				break;
			case '-':
				if (number > 0){
					closeProcess(number - 1);
					number--;
				}
				break;

			case 'q':
				closeAllProcess(number);
				exit(0);
			}
		}
		for (int counter = 0; counter < number; counter++){
			WaitForSingleObject(fatherEvent, INFINITE);
			childEvent = OpenEvent(EVENT_ALL_ACCESS, true,
				itoa(counter, buffer, DEC));
			SetEvent(childEvent);
		}
		Sleep(3);
		fflush(stdin);
	}

#else

	signal(SIGUSR1, fFather);

	list<int> pids;
	int key;

	while (true){
		cout << "next!" << endl;
		switch (getch()){
		case '+':
			pids.push_back(addProcess(pids.size()));
			break;
		case '-':
			closeProcess(pids.back());
			pids.pop_back();
			break;
		case 'q':
			closeAllProcess(pids);
			exit(0);
		}
		showProcessInformation(pids);
	}

#endif

}

int addProcess(int number){

#ifdef _MSC_VER 

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE childEvent;
	HANDLE closeEvent;
	char string[SIZE], buffer[SIZE];
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	strcpy(string, itoa(number, buffer, DEC));
	childEvent = CreateEvent(NULL, false, true, string);
	strcat(string, "close");
	closeEvent = CreateEvent(NULL, true, false, string);
	strcpy(string, "");
	for (int i = 0; i < 4; i++)
		strcat(string, itoa(number, buffer, DEC));
	strcat(string, " ");
	strcat(string, itoa(number, buffer, DEC));
	if (!CreateProcess(WINPATH, string,
		NULL, NULL, true, 0, NULL, NULL, &si, &pi)){
		cout << "CreateProcess error!" << endl;
		return 0;
	}
	return 1;

#else

	int pidChildren;
	char buffer[100];
	pidChildren = fork();
	switch (pidChildren){
	case -1:
		cout << "Create process failed!";
		break;
	case 0:
		execl(linuxPath,
			linuxPath,
			itoa(number), NULL);
		break;
	}
	return pidChildren;

#endif

}

void closeProcess(int number){

#ifdef _MSC_VER
	char string[SIZE], buffer[SIZE];
	HANDLE closeEvent;
	strcpy(string, itoa(number, buffer, DEC));
	strcat(string, "close");
	closeEvent = OpenEvent(EVENT_ALL_ACCESS, true, string);
	SetEvent(closeEvent);
	return;

#else
	if (kill(number, SIGUSR2) == -1){
		cout << "Send signal error!" << endl;
		exit(1);
	}

#endif

}

#ifdef _MSC_VER

void closeAllProcess(int number){
	char string[SIZE], buffer[SIZE];
	HANDLE closeEvent;
	for (int counter = 0; counter < number; counter++)
		closeProcess(counter);
	return;
}

#else

void closeAllProcess(list<int>& pids){
	for (list<int>::iterator it = pids.begin(); it != pids.end(); it++){
		if (kill(*it, SIGUSR2) == -1){
			cout << "Send signal error!" << endl;
			exit(1);
		}
	}
	pids.clear();
}

void showProcessInformation(list<int>& pids){
	for (list<int>::iterator it = pids.begin(); it != pids.end(); it++){
		if (kill(*it, SIGUSR1) == -1){
			cout << "Send signal error!" << endl;
			exit(1);
		}
		while (!flagFather);
		flagFather = false;
	}
}

char getch() {
	struct termios _new, old;
	tcgetattr(0, &old);
	_new = old;
	_new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &_new);
	char ch = getchar();
	tcsetattr(0, TCSANOW, &old);
	return ch;
}

char* itoa(int tmp){
	char* buffer;
	int size = 0, mas[SIZE], i;
	if (tmp == 0){
		buffer = new char[2];
		buffer[0] = 48;
		buffer[1] = '\0';
		return buffer;
	}
	while (tmp > 0){
		mas[size++] = tmp % 10;
		tmp /= 10;
	}
	buffer = new char[size + 1];
	for (i = 0; i < size; i++)
		buffer[i] = mas[size - i - 1] + 48;
	buffer[i] = '\0';
	return buffer;
}

#endif