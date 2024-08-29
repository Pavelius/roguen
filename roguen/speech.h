#pragma once

struct speechi;

extern int speech_random;

void speech_get(const char*& result, const char* id, const char* action, const char* middle);
void speech_read(const char* url);

const char* speech_get_id(int index);
const char* speech_get(const char* id);
const char* speech_get(int index);