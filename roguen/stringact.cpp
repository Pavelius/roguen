#include "crt.h"
#include "stringact.h"

namespace {
struct change_string {
	const char*	id;
	const char*	female;
	const char*	male;
	const char*	multiply;
};
static change_string player_gender[] = {{"�", "�", "", "�"},
	{"��", "��", "", "��"},
	{"���", "���", "��", "���"},
	{"��", "��", "��", "��"},
	{"��", "��", "���", "��"},
	{"���", "���", "����", "���"},
	{"���", "���", "��", "���"},
	{"��", "��", "���", "��"},
	{"������", "������", "�������", "�������"},
	{"����", "����", "�����", "�����"},
	{"������", "������", "������", "�������"},
	{"�������", "�������", "��������", "�������"},
	{"�������", "�������", "�������", "�������"},
};
}

void stringact::addidentifier(const char* identifier) {
	if(name && equal(identifier, "�����") || equal(identifier, "name"))
		add(name);
	else if(name && strcmp(identifier, "�����") == 0)
		addof(name);
	else if(name && strcmp(identifier, "�����") == 0)
		addto(name);
	else {
		for(auto& e : player_gender) {
			if(strcmp(e.female, identifier) != 0)
				continue;
			if(female)
				add(e.female);
			else
				add(e.male);
			return;
		}
		stringbuilder::addidentifier(identifier);
	}
}