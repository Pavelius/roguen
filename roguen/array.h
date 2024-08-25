#pragma once

#include "slice.h"

class array {
	size_t count_maximum;
	void grow(unsigned offset, size_t delta);
	void shrink(unsigned offset, size_t delta);
	void zero(unsigned offset, size_t delta);
public:
	void* data;
	size_t count, size;
	constexpr array(size_t size = 0) : count_maximum(0), data(0), count(0), size(size) {}
	constexpr array(void* data, size_t size, size_t count) : count_maximum(count | 0x80000000), data(data), count(count), size(size) {}
	constexpr array(void* data, size_t size, size_t count, unsigned count_maximum) : count_maximum(count_maximum | 0x80000000), data(data), count(count), size(size) {}
	constexpr explicit operator bool() const { return count != 0; }
	~array();
	void* add();
	void* addz() { auto p = add(); memset(p, 0, size); return p; }
	void* add(const void* element);
	void* addfind(const char* id);
	void* addu(const void* element, unsigned count);
	const char* addus(const char* element, unsigned count);
	char* begin() const { return (char*)data; }
	void change(unsigned offset, int size);
	void clear();
	char* end() const { return (char*)data + size * count; }
	int find(int i1, int i2, void* value, unsigned offset, size_t size) const;
	int find(void* value, unsigned offset, size_t size) const { return find(0, -1, value, offset, size); }
	int findps(const char* value, unsigned offset, size_t size) const;
	int find(const char* value, unsigned offset) const;
	const void* findu(const void* value, size_t size) const;
	const char* findus(const char* value, size_t size) const;
	void* findv(const char* value, unsigned offset) const;
	size_t getmaximum() const { return count_maximum & 0x7FFFFFFF; }
	size_t getcount() const { return count; }
	size_t getsize() const { return size; }
	constexpr bool have(const void* element) const { return element >= data && element < (char*)data + size * count; }
	int indexof(const void* element) const;
	void* insert(int index, const void* element);
	bool isgrowable() const { return (count_maximum & 0x80000000) == 0; }
	void* ptr(int index) const { return (char*)data + size * index; }
	void* ptrs(int index) const { return (((unsigned)index) < count) ? (char*)data + size * index : 0; }
	template<class T> slice<T> records() const { return slice<T>((T*)data, count); }
	void remove(int index, int elements_count = 1);
	void shift(int i1, int i2, size_t c1, size_t c2);
	void setcount(unsigned value) { count = value; }
	void setup(size_t size);
	void swap(int i1, int i2);
	void reserve(unsigned count);
};