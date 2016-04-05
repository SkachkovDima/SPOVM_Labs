#include "head.h"

int main(int argc, char* argv[]){
	setlocale(LC_ALL, "Russian");

#ifdef _MSC_VER
	char string[SIZE];
	strcpy(string, argv[1]);
	strcat(string, "close");
	HANDLE fatherEvent = OpenEvent(EVENT_ALL_ACCESS, true,
		"fatherEvent");
	DWORD key;
	HANDLE events[2];
	events[0] = OpenEvent(EVENT_ALL_ACCESS, false, argv[1]);
	events[1] = OpenEvent(EVENT_ALL_ACCESS, false, string);

	while (true){
		key = WaitForMultipleObjects(2, events, false, INFINITE);
		if (key == WAIT_OBJECT_0 + 0){
			cout << argv[0] << endl;
			SetEvent(fatherEvent);
		}
		if (key == WAIT_OBJECT_0 + 1){
			ResetEvent(events[1]);
			break;
		}
	}

#else

	signal(SIGUSR1, fActivate);
	signal(SIGUSR2, fClose);

	while (closeFlag){
		while (!activateFlag);
		cout << argv[1] << argv[1] << argv[1] << endl;
		activateFlag = false;
		if (kill(getppid(), SIGUSR1) == -1){
			cout << "Send signal error!" << endl;
			return 0;
		}
	}

#endif

	return 0;
}