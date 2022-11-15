#include "tile.h"

#pragma once

struct featurei {
	const char*		id;
	framerange		features, overlay;
	unsigned char	priority;
	unsigned		flags;
	color			minimap;
	featurei		*leadto, *activateto;
	char			lead;
	char			chance_auto_activate;
	bool			is(tilef v) const { return (flags & (1 << v)) != 0; }
	bool			isvisible() const { return features.count != 0; }
	featurei*		getactivate() const { return activateto; }
	featurei*		gethidden() const;
	featurei*		getlead() const { return leadto; }
	void			paint(int random) const;
	bool			autoactivated() const { return activateto && chance_auto_activate; }
	bool			handactivated() const { return activateto && !chance_auto_activate; }
};