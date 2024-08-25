#pragma once

extern "C" void* memchr(const void* ptr, int value, long unsigned num);
extern "C" void* memcpy(void* destination, const void* source, long unsigned size) noexcept(true);
extern "C" int memcmp(const void* p1, const void* p2, size_t size) noexcept(true);
extern "C" void* memmove(void* destination, const void* source, size_t size) noexcept(true);
extern "C" void* memset(void* destination, int value, long unsigned size) noexcept(true);

template<class T> class slice {
	T* data;
	size_t count;
public:
	typedef T data_type;
	constexpr slice() : data(0), count(0) {}
	template<size_t N> constexpr slice(T(&v)[N]) : data(v), count(N) {}
	constexpr slice(T* data, unsigned count) : data(data), count(count) {}
	constexpr slice(T* p1, const T* p2) : data(p1), count(p2 - p1) {}
	explicit operator bool() const { return count != 0; }
	constexpr T* begin() const { return data; }
	constexpr T* end() const { return data + count; }
	constexpr unsigned size() const { return count; }
};
