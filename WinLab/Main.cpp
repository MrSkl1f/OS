#include <stdio.h>
#include <windows.h>
#include <iostream>

#define WRITERS 3
#define READERS 5
#define MAX_VAL 10

#define OK 0
#define ERROR 1

int value = 0;

bool activeWriter = false;
unsigned int activeReaders = 0;

unsigned int waitingReaders = 0;
unsigned int waitingWriters = 0;

HANDLE writers[WRITERS];
HANDLE readers[READERS];

HANDLE canRead;
HANDLE canWrite;
HANDLE mutex;

void StartRead()
{
	InterlockedIncrement(&waitingReaders);
	if (activeWriter || waitingWriters > 0)
	{
		WaitForSingleObject(canRead, INFINITE);
	}
	InterlockedDecrement(&waitingReaders);
	InterlockedIncrement(&activeReaders);
	SetEvent(canRead);
}

void StopRead()
{
	InterlockedDecrement(&activeReaders);
	if (activeReaders == 0)
	{
		SetEvent(canWrite);
	}
}

void StartWrite()
{
	InterlockedIncrement(&waitingWriters);
	if (activeWriter || activeReaders > 0)
	{
		WaitForSingleObject(canWrite, INFINITE);
	}
	InterlockedDecrement(&waitingWriters);
	activeWriter = true;
	ResetEvent(canWrite);
}

void StopWrite()
{
	activeWriter = false;
	if (waitingWriters)
	{
		SetEvent(canWrite);
	}
	else
	{
		SetEvent(canRead);
	}
}

DWORD WINAPI Reader(LPVOID lpParam)
{
	bool isEnd = FALSE;
	while (!isEnd)
	{
		StartRead();
		
		if (value >= MAX_VAL)
		{
			isEnd = TRUE;
		}
		else
		{
			printf("Reader %d (%d) read %d\n", (int)lpParam, GetCurrentThreadId(), value);
		}

		StopRead();
		Sleep(400);
	}
	return OK;
}

DWORD WINAPI Writer(LPVOID lpParam)
{
	bool isEnd = FALSE;
	while (!isEnd)
	{
		StartWrite();
		WaitForSingleObject(mutex, INFINITE);

		if (value >= MAX_VAL)
		{
			isEnd = TRUE;
		}
		else
		{
			value++;
			printf("Writer %d (%d) wrote %d\n", (int)lpParam, GetCurrentThreadId(), value);
		}

		ReleaseMutex(mutex);
		StopWrite();
		Sleep(400);
	}
	return OK;
}

bool CheckHandle(HANDLE cur, const char* msg)
{
	if (cur == NULL)
	{
		CloseHandle(mutex);
		CloseHandle(canRead);
		CloseHandle(canWrite);
		perror(msg);
		return false;
	}
	return true;
}

int CreateThreads() 
{
	for (int i = 0; i < WRITERS; i++)
	{
		writers[i] = CreateThread(NULL, 0, &Writer, (LPVOID)i, 0, NULL);
		if (!CheckHandle(writers[i], "Thread"))
		{
			return ERROR;
		}
	}

	for (int i = 0; i < READERS; i++)
	{
		readers[i] = CreateThread(NULL, 0, &Reader, (LPVOID)i, 0, NULL);
		if (!CheckHandle(readers[i], "Thread"))
		{
			return ERROR;
		}
	}

	return OK;
}

int InitHandles()
{
	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL)
	{
		perror("mutex");
		return ERROR;
	}

	canRead = CreateEvent(NULL, FALSE, FALSE, TEXT("ReadEvent"));
	if (canRead == NULL)
	{
		CloseHandle(mutex);
		perror("canRead");
		return ERROR;
	}

	canWrite = CreateEvent(NULL, TRUE, FALSE, TEXT("WriteEvent"));
	if (canWrite == NULL)
	{
		CloseHandle(mutex);
		CloseHandle(canRead);
		perror("canWrite");
		return ERROR;
	}
	
	return OK;
}

int main() 
{
	if (InitHandles() == ERROR || CreateThreads() == ERROR)
	{
		return ERROR;
	}

	WaitForMultipleObjects(WRITERS, writers, TRUE, INFINITE);
	WaitForMultipleObjects(READERS, readers, TRUE, INFINITE);

	CloseHandle(mutex);
	CloseHandle(canRead);
	CloseHandle(canWrite);

	return 0;
}