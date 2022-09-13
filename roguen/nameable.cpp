#include "crt.h"
#include "nameable.h"

const char* nameable::getname() const {
	return getnm(id);
}