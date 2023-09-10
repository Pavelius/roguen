#include "io_stream.h"

#pragma once

namespace io {
class memory : stream {
	unsigned		readed, writed, allocated;
	void*			data;
public:
	memory() : stream(), readed(0), writed(0), allocated(0) {}
	~memory() { clear(); }
	void			clear();
	int				read(void* result, int count) override;
	int				seek(int count, int rel = SeekCur) override;
	int				write(const void* result, int count) override;
};
}