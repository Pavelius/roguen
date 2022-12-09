#include "dice.h"

#pragma once

struct diceprogress : dice {
 char multiplier, divider, bound;
 int roll(int level) const;
};