#include "crt.h"
#include "io_memory.h"

extern "C" void* malloc(long unsigned size);
extern "C" void* realloc(void *ptr, long unsigned size);
extern "C" void	free(void* pointer);

extern unsigned rmoptimal(unsigned need_count);

void io::writer::clear() {
	if(data)
		delete data;
	writed = allocated = 0;
}

int io::writer::write(const void* result, int count) {
	if(count <= 0)
		return 0;
	auto total = writed + count;
	if(total > allocated) {
		if(!data)
			data = malloc(total);
		else {
			auto p = realloc(data, total);
			if(!p)
				return 0;
			data = p;
		}
		allocated = total;
	}
	memcpy((char*)data + writed, result, count);
	writed += count;
	return count;
}

int	io::writer::seek(int count, int rel) {
	switch(rel) {
	case SeekCur: return writed;
	case SeekEnd: return writed + count;
	default: writed = ((unsigned)count > allocated) ? allocated : count; return writed;
	}
}

int io::reader::read(void* result, int count) {
	if(readed >= maximum || count <= 0)
		return 0;
	if(readed + count > maximum)
		count = maximum - readed;
	memcpy(result, (char*)data + readed, count);
	readed += count;
	return count;
}

int	io::reader::seek(int count, int rel) {
	switch(rel) {
	case SeekCur: return readed;
	case SeekEnd: return maximum + count;
	default: readed = ((unsigned)count > maximum) ? maximum : count; return readed;
	}
}

void io::reader::clear() {
	readed = 0;
}