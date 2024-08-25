#pragma once

extern "C" void exit(int exit_code);

#ifdef _DEBUG
#define assert(e) if(!(e)) { exit(255); }
#else
#define assert(e)
#endif
