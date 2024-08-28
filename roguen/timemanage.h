#pragma once

struct timemanage {
	unsigned	minutes;
	unsigned	restore_half_turn, restore_turn, restore_hour, restore_day_part, restore_day, restore_several_days;
	unsigned	getminutes() const { return minutes; }
};
