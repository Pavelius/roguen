#include "io_stream.h"

#pragma once

namespace io {
class writer : stream {
	unsigned		writed, allocated;
	void*			data;
public:
	writer() : stream(), writed(0), allocated(0), data(0) {}
	~writer() { clear(); }
	char*			begin() const { return (char*)data; }
	void			clear();
	char*			end() const { return (char*)data + writed; }
	int				read(void* result, int count) override { return 0; }
	int				seek(int count, int rel = SeekCur) override;
	int				write(const void* result, int count) override;
};
class reader : stream {
	unsigned		readed, maximum;
	void*			data;
public:
	reader(void* data, unsigned maximum) : stream(), readed(0), maximum(maximum), data(data) {}
	template<class T> reader(T& source) : reader(&source, sizeof(T)) {}
	~reader() { clear(); }
	char*			begin() const { return (char*)data; }
	void			clear();
	char*			end() const { return (char*)data + maximum; }
	int				read(void* result, int count) override;
	int				seek(int count, int rel = SeekCur) override;
	int				write(const void* result, int count) override { return 0; }
};
}