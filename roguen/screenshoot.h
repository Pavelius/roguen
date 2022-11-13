#include "point.h"
#include "draw.h"

#pragma once

namespace draw {
struct screenshoot : public point, public surface {
	screenshoot(bool fade = false);
	screenshoot(rect rc, bool fade = false);
	~screenshoot();
	void				blend(const surface& destination, unsigned milliseconds = 1000) const;
	static void			fade(fnevent proc, unsigned milliseconds = 1000);
	void				restore() const;
};
}