#include "main.h"

static talki* current_talk;

static const phrasei* ask_answer(const phrasei* p) {
	answers an;
	for(auto pa = p->nextanswer(); pa; pa = pa->nextanswer())
		an.add(pa, pa->text);
	if(!an)
		an.add(0, getnm("Continue"));
	return (phrasei*)an.choose();
}

static const phrasei* apply_answer(const phrasei* p) {
	if(!p)
		return 0;
	auto pt = talki::owner(p);
	return pt->find(p->next);
}

void talk_entry(const phrasei* p) {
	while(p) {
		console.clear();
		opponent->say(p->text);
		p = ask_answer(p);
		p = apply_answer(p);
	}
}

bool creature::talk(const char* id) {
	pushvalue push(current_talk);
	current_talk = bsdata<talki>::find(id);
	if(!current_talk)
		return false;
	talk_entry(current_talk->find(1));
	return true;
}