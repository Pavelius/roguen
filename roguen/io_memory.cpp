#include "crt.h"
#include "io_memory.h"

extern "C" void* malloc(long unsigned size);
extern "C" void* realloc(void *ptr, long unsigned size);
extern "C" void	free(void* pointer);

extern unsigned rmoptimal(unsigned need_count);

void io::memory::clear() {
	if(data)
		delete data;
	writed = readed = allocated = 0;
}

int io::memory::read(void* result, int count) {
	if(readed >= writed || count <= 0)
		return 0;
	if(readed + count > writed)
		count = writed - readed;
	memcpy(result, (char*)data + readed, count);
	readed += count;
	return count;
}

int io::memory::write(const void* result, int count) {
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