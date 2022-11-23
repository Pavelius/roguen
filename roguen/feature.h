#include "tile.h"
#include "variant.h"

#pragma once

struct featurei {
	const char*		id;
	framerange		features, overlay;
	unsigned char	priority;
	unsigned		flags;
	int				movedifficult;
	color			minimap;
	featurei		*leadto, *activateto;
	variant			activate_item;
	variants		activate_script;
	char			random_count;
	char			lead;
	char			chance_auto_activate;
	bool			is(tilef v) const { return (flags & (1 << v)) != 0; }
	bool			islocked() const { return activate_item.value != 0; }
	bool			isvisible() const { return features.count != 0; }
	featurei*		getactivate() const { return activateto; }
	featurei*		gethidden() const;
	featurei*		getlead() const { return leadto; }
	featurei*		getlocked() const;
	void			paint(int random) const;
	bool			autoactivated() const { return activateto && chance_auto_activate; }
	bool			handactivated() const { return activateto && !chance_auto_activate; }
};