#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace std;

#ifdef _MSC_VER
#include <Windows.h>
#include <conio.h>

#define TEXT_PIPE TEXT("\\\\.\\pipe\\pipe")
#define PATH TEXT("../Debug/client.exe")

HANDLE create_pipe();
void input(PROCESS_INFORMATION&, HANDLE&, HANDLE*);

#else
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>

#define CLIENT_PATH "/home/linux/University/Лабораторные работы/LR3/client"
#define NUMBER 2

bool work = true;

void setSemaphores(int);
void deleteSemaphores(int);
void handler(int);
void inputString(int, int);
#endif

int main(){
#ifdef _MSC_VER
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	HANDLE semaphores[3];
	semaphores[0] = CreateSemaphore(NULL, 0, 1, TEXT("s[0]-ready"));
	semaphores[1] = CreateSemaphore(NULL, 0, 1, TEXT("s[1]-end"));
	semaphores[2] = CreateSemaphore(NULL, 0, 1, TEXT("s[2]-exit"));

#else
	int semid;
	int shmid;

	pid_t pid;

	signal(SIGUSR1, handler);

	semid = semget(IPC_PRIVATE, NUMBER, IPC_CREAT | 0666);
	if (semid == -1){
		perror("semget()");
		exit(0);
	}
	setSemaphores(semid);

	shmid = shmget(IPC_PRIVATE, 55, IPC_CREAT | 0666);
	if (shmid == -1){
		perror("shmget()");
		exit(0);
	}
#endif

#ifdef _MSC_VER
	if (!CreateProcess(PATH, NULL, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)){
		printf("Creation failed\n");
	}

	HANDLE pipe = create_pipe();

#else
	pid = fork();
	switch (pid){
	case -1:
		perror("fork()");
		return 1;

	case 0:
		if (!execl(CLIENT_PATH, to_string(semid).c_str(),
			to_string(shmid).c_str(), nullptr)){
			perror("execl()");
			return 1;
		}
	}
#endif

#ifdef _MSC_VER
	input(pi, pipe, semaphores);

	CloseHandle(pipe);
	CloseHandle(semaphores[0]);
	CloseHandle(semaphores[1]);

#else
	inputString(semid, shmid);
	deleteSemaphores(semid);
#endif

	return 0;
}

#ifdef _MSC_VER
HANDLE create_pipe(){
	HANDLE pipe = CreateNamedPipe(TEXT_PIPE,
		PIPE_ACCESS_OUTBOUND,
		PIPE_TYPE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		0,
		0,
		INFINITE,
		(LPSECURITY_ATTRIBUTES)NULL
		);

	if (pipe == INVALID_HANDLE_VALUE){
		printf("Pipe Error\n");
		exit(0);
	}

	if (!ConnectNamedPipe(pipe, (LPOVERLAPPED)NULL)){
		printf("Connection failed\n");
	}

	return pipe;
}

void input(PROCESS_INFORMATION& pi, HANDLE& pipe, HANDLE *semaphores){
	char text[100];

	while (true){
		fflush(stdin);
		gets(text);
		DWORD BytesIn;

		if (!strcmp(text, "kill")){
			ReleaseSemaphore(semaphores[2], 1, NULL);
			WaitForSingleObject(pi.hProcess, INFINITE);
			break;
		}

		ReleaseSemaphore(semaphores[0], 1, NULL);

		int sizeOfText = strlen(text);
		WriteFile(pipe, &sizeOfText, sizeof(sizeOfText), 
			&BytesIn, (LPOVERLAPPED)NULL);

		if (!WriteFile(pipe, text, sizeOfText, 
			&BytesIn, (LPOVERLAPPED)NULL))
			printf("Write Error\n");

		WaitForSingleObject(semaphores[1], INFINITE);
	}
}

#else
void setSemaphores(int semid){
	for (int i = 0; i < NUMBER; i++){
		if (semctl(semid, i, SETVAL, 0) == -1){
			perror("Set semctl()");
			exit(1);
		}
	}
}

void deleteSemaphores(int semid){
	if (semctl(semid, 0, IPC_RMID, 0) == -1){
		perror("Delete semctl()");
		exit(1);
	}
}

void inputString(int semid, int shmid){
	char *memory = (char*)shmat(shmid, 0, 0);

	while (work){
		struct sembuf sembuf[2];

		sembuf[0].sem_num = 0;
		sembuf[0].sem_op = -1;
		sembuf[0].sem_flg = 0;

		sembuf[1].sem_num = 1;
		sembuf[1].sem_op = 1;
		sembuf[1].sem_flg = 0;

		semop(semid, &sembuf[1], 1);
		semop(semid, &sembuf[0], 1);

		char string[100];
		printf("Input: ");
		scanf("%s", string);
		strcpy(memory, string);

		semop(semid, &sembuf[1], 1);
		semop(semid, &sembuf[0], 1);

		puts("Server: I DID IT");
		semop(semid, &sembuf[1], 1);
	}

	if (shmdt(memory) == -1){
		perror("Disconnect shared memory");
		exit(1);
	}

	//    if(semctl(shmid, IPC_RMID, 0) == -1){
	//        perror("Delete shared memory");
	//        exit(1);
	//    }
}

void handler(int){
	signal(SIGUSR1, handler);
	work = false;
}
#endif