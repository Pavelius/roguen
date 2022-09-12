#include "crt.h"
#include "draw.h"
#include "resource.h"

using namespace draw;

const sprite* draw::gres(res id) {
	auto p = bsdata<resource>::elements + (int)id;
	if(p->notfound)
		return 0;
	if(!p->data) {
		char temp[260]; stringbuilder sb(temp);
		sb.add("art/%1.pma", p->name);
		p->data = (sprite*)loadb(temp);
	}
	if(!p->data)
		p->notfound = true;
	return p->data;
}