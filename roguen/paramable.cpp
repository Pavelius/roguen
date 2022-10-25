#include "paramable.h"

const char* paramable::getname() const {
	static char temp[260];
	stringbuilder sb(temp);
	if(v1)
		sb.adds(v1.getname());
	if(v2)
		sb.adds(v2.getname());
	return temp;
}