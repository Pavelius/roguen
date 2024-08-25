#include "bsdata.h"
#include "draw.h"
#include "io_stream.h"
#include "resource.h"

using namespace draw;

const sprite* resource::get() {
	if(notfound)
		return 0;
	if(!data) {
		char temp[260]; stringbuilder sb(temp);
		sb.add("art/%1.pma", name);
		data = (sprite*)loadb(temp);
	}
	if(!data)
		notfound = true;
	return data;
}

const sprite* draw::gres(res id) {
	return bsdata<resource>::elements[(int)id].get();
}