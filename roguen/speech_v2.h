#pragma once

void speech_initialize();
void speech_get(const char*& result, const char* id, const char* action, const char* middle, const char* postfix);
void speech_read(const char* url);

const char* speech_getid(int index);
const char* speech_get(const char* id);
