#include "direction.h"
#include "pointm.h"

pointm pointm::to(direction_s v, unsigned char mps) const {
	if(!(*this))
		return pointm();
	switch(v) {
	case North:
		if(y == 0)
			return pointm();
		return pointm(x, y - 1);
	case NorthWest:
		if(y == 0 || x == 0)
			return pointm();
		return pointm(x - 1, y - 1);
	case NorthEast:
		if(y == 0 || x >= mps - 1)
			return pointm();
		return pointm(x + 1, y - 1);
	case South:
		if(y >= mps - 1)
			return pointm();
		return pointm(x, y + 1);
	case SouthWest:
		if(y >= mps - 1 || x == 0)
			return pointm();
		return pointm(x - 1, y + 1);
	case SouthEast:
		if(y >= mps - 1 || x >= mps - 1)
			return pointm();
		return pointm(x + 1, y + 1);
	case West:
		if(x == 0)
			return pointm();
		return pointm(x - 1, y);
	case East:
		if(x >= mps - 1)
			return pointm();
		return pointm(x + 1, y);
	default:
		return *this;
	}
}