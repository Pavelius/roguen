#include "win.h"

extern "C" int time(void* p);

void waitcputime(unsigned v) {
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = (long long)v * -100000;
	auto hTimer = CreateWaitableTimerW(0, 1, 0);
	if(!hTimer)
		return;
	if(!SetWaitableTimer(hTimer, &liDueTime, 0, 0, 0, 0))
		return;
	WaitForSingleObject(hTimer, INFINITE);
}

unsigned long getcputime() {
    return GetTickCount();
}

unsigned int randomseed() {
	return time(0);
}