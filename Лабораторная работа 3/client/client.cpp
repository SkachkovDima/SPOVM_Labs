#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace std;

#ifdef _MSC_VER
#include <Windows.h>

#define TEXT_PIPE TEXT("\\\\.\\pipe\\pipe")

#else
#include <termios.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <wait.h>

char getch()
{
	struct termios renewed, old;
	tcgetattr(0, &old);
	renewed = old;
	renewed.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &renewed);
	char ch = getchar();
	tcsetattr(0, TCSANOW, &old);
	return ch;
}
#endif

int main(int argc, char* argv[]){

#ifdef _MSC_VER
	HANDLE mainPipe;
	HANDLE semaphores[3];

	semaphores[0] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 
		TRUE, TEXT("s[0]-ready"));
	semaphores[1] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 
		TRUE, TEXT("s[1]-end"));
	semaphores[2] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 
		TRUE, TEXT("s[2]-exit"));

	char text[100];

	mainPipe = CreateFile(
		TEXT_PIPE,
		GENERIC_READ,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	int successFlag;

#else
	struct sembuf sembuf[2];

	char* memory = (char*)shmat(atoi(argv[1]), 0, 0);
	int semid = atoi(argv[0]);
#endif

	while (true){
#ifdef _MSC_VER
		successFlag = 1;
		printf("Waiting for parent...\n");
		DWORD BytesOut;
		char s[] = "";
		strcpy(text, s);
		int index = WaitForMultipleObjects(3, semaphores, 
			FALSE, INFINITE) - WAIT_OBJECT_0;

		if (index == 2){
			printf("Closing child");
			break;
		}
		int size;
		if (!ReadFile(mainPipe, &size, sizeof(size), 
			&BytesOut, NULL)){
			break;
		}

		int successFlag = ReadFile(mainPipe, text, size + 1, 
			&BytesOut, NULL);

		if (!successFlag){
			break;
		}

		for (int i = 0; i < size; i++){
			printf("%c", text[i]);
		}
		printf("\n");

		ReleaseSemaphore(semaphores[1], 1, NULL);

#else
		sembuf[0].sem_num = 0;
		sembuf[0].sem_op = 1;
		sembuf[0].sem_flg = 0;

		sembuf[1].sem_num = 1;
		sembuf[1].sem_op = -1;
		sembuf[1].sem_flg = 0;

		semop(semid, &sembuf[1], 1);
		puts("I'm a Client and i'm ready...");
		semop(semid, &sembuf[0], 1);
		semop(semid, &sembuf[1], 1);

		printf("Client: ");
		puts(memory);

		if (!strcmp(memory, "quit")){
			kill(getppid(), SIGUSR1);
			break;
		}

		semop(semid, &sembuf[0], 1);
		semop(semid, &sembuf[1], 1);

		puts("Press button...");
		getch();
		system("clear");
#endif
	}

#ifdef _MSC_VER
	CloseHandle(mainPipe);
	CloseHandle(semaphores[0]);
	CloseHandle(semaphores[1]);

#else
	if (shmdt(memory) == -1){
		perror("Disconnect shared memory");
		exit(1);
	}
#endif
	return 0;
}