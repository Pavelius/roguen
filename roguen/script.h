#pragma once

struct script {
	typedef void(*fnscript)(int bonus);
	const char*		id;
	fnscript		proc;
	static bool		stop;
};
