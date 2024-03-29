#include "crt.h"
#include "io_stream.h"

extern "C" void* malloc(long unsigned size);
extern "C" void* realloc(void *ptr, long unsigned size);
extern "C" void	free(void* pointer);

unsigned rmoptimal(unsigned need_count) {
	const unsigned mc = 4 * 256 * 256;
	unsigned m = 8;
	while(m < mc) {
		m = m << 1;
		if(need_count < m)
			return m;
	}
	return need_count + mc;
}

float sqrt(const float x) {
	const float xhalf = 0.5f * x;
	// get bits for floating value
	union {
		float x;
		int i;
	} u;
	u.x = x;
	u.i = 0x5f3759df - (u.i >> 1);  // gives initial guess y0
	return x * u.x * (1.5f - xhalf * u.x * u.x);// Newton step, repeating increases accuracy
}

int isqrt(int num) {
	int res = 0;
	int bit = 1 << 30;
	// "bit" starts at the highest power of four <= the argument.
	while(bit > num)
		bit >>= 2;
	while(bit != 0) {
		if(num >= res + bit) {
			num -= res + bit;
			res = (res >> 1) + bit;
		} else
			res >>= 1;
		bit >>= 2;
	}
	return res;
}

int szcmpi(const char* p1, const char* p2) {
	while(*p2 && *p1) {
		unsigned s1 = szupper(szget(&p1));
		unsigned s2 = szupper(szget(&p2));
		if(s1 == s2)
			continue;
		return s1 - s2;
	}
	unsigned s1 = szupper(szget(&p1));
	unsigned s2 = szupper(szget(&p2));
	return s1 - s2;
}

int szcmpi(const char* p1, const char* p2, int max_count) {
	while(*p2 && *p1 && max_count-- > 0) {
		unsigned s1 = szupper(szget(&p1));
		unsigned s2 = szupper(szget(&p2));
		if(s1 == s2)
			continue;
		return s1 - s2;
	}
	if(!max_count)
		return 0;
	unsigned s1 = szupper(szget(&p1));
	unsigned s2 = szupper(szget(&p2));
	return s1 - s2;
}

bool equal(const char* p, const char* s) {
	if(!p || !s)
		return false;
	while(*s && *p)
		if(*p++ != *s++)
			return false;
	return *s == *p;
}

bool matchuc(const char* name, const char* filter) {
	if(!name || name[0] == 0)
		return false;
	auto sym = szupper(szget(&filter));
	auto pn = name;
	while(pn[0]) {
		auto sym1 = szupper(szget(&pn));
		if(sym1 == sym) {
			auto pf = filter;
			auto ps = pn;
			while(true) {
				if(pf[0] == 0)
					return true;
				auto sym2 = szupper(szget(&pf));
				auto sym1 = szupper(szget(&pn));
				if(sym1 != sym2)
					break;
			}
			pn = ps;
		}
	}
	return false;
}

bool ischa(unsigned u) {
	return (u >= 'A' && u <= 'Z')
		|| (u >= 'a' && u <= 'z')
		|| (u >= 0x410 && u <= 0x44F);
}

unsigned szupper(unsigned u) {
	if(u >= 0x61 && u <= 0x7A)
		return u - 0x61 + 0x41;
	else if(u >= 0x430 && u <= 0x44F)
		return u - 0x430 + 0x410;
	return u;
}

void szupper(char* p) {
	char* s1 = p;
	const char* p1 = p;
	unsigned sym;
	do {
		sym = szget(&p1);
		szput(&s1, szupper(sym));
	} while(sym);
}

void szlower(char* p) {
	char* s1 = p;
	const char* p1 = p;
	unsigned sym;
	do {
		sym = szget(&p1);
		szput(&s1, szlower(sym));
	} while(sym);
}

void szchange(char* p, char s1, char s2) {
	while(*p) {
		if(*p == s1)
			*p = s2;
		p++;
	}
}

unsigned szlower(unsigned u) {
	if(u >= 0x41 && u <= 0x5A)
		return u - 0x41 + 0x61;
	else if(u >= 0x410 && u <= 0x42F)
		return u - 0x410 + 0x430;
	return u;
}

int getdigitscount(unsigned number) {
	if(number < 10)
		return 1;
	if(number < 100)
		return 2;
	if(number < 1000)
		return 3;
	if(number < 10000)
		return 4;
	if(number < 100000)
		return 5;
	if(number < 1000000)
		return 6;
	if(number < 10000000)
		return 7;
	if(number < 100000000)
		return 8;
	return 9;
}

unsigned szget(const char** input, codepage code) {
	const unsigned char* p;
	unsigned result;
	switch(code) {
	case codepage::UTF8:
		p = (unsigned char*)*input;
		result = *p++;
		if(result >= 192 && result <= 223)
			result = (result - 192) * 64 + (*p++ - 128);
		else if(result >= 224 && result <= 239) {
			result = (result - 224) * 4096 + (p[0] - 128) * 64 + (p[1] - 128);
			p += 2;
		}
		*input = (const char*)p;
		return result;
	case codepage::U16LE:
		p = (unsigned char*)*input;
		result = p[0] | (p[1] << 8);
		*input = (const char*)(p + 2);
		return result;
	case codepage::W1251:
		result = (unsigned char)*(*input)++;
		if(((unsigned char)result >= 0xC0))
			return result - 0xC0 + 0x410;
		else switch(result) {
		case 0xB2: return 0x406;
		case 0xAF: return 0x407;
		case 0xB3: return 0x456;
		case 0xBF: return 0x457;
		}
		return result;
	default:
		return *(*input)++;
	}
}

void szput(char** output, unsigned value, codepage code) {
	char* p;
	switch(code) {
	case codepage::UTF8:
		p = *output;
		if(((unsigned short)value) < 128)
			*p++ = (unsigned char)value;
		else if(((unsigned short)value) < 2047) {
			*p++ = (unsigned char)(192 + (((unsigned short)value) / 64));
			*p++ = (unsigned char)(128 + (((unsigned short)value) % 64));
		} else {
			*p++ = (unsigned char)(224 + (((unsigned short)value) / 4096));
			*p++ = (unsigned char)(128 + ((((unsigned short)value) / 64) % 64));
			*p++ = (unsigned char)(224 + (((unsigned short)value) % 64));
		}
		*output = p;
		break;
	case codepage::W1251:
		if(value >= 0x410 && value <= 0x44F)
			value = value - 0x410 + 0xC0;
		else switch(value) {
		case 0x406: value = 0xB2; break; // I
		case 0x407: value = 0xAF; break; // �
		case 0x456: value = 0xB3; break;
		case 0x457: value = 0xBF; break;
		}
		*(*output)++ = (unsigned char)value;
		break;
	case codepage::U16LE:
		*(*output)++ = (unsigned char)(value & 0xFF);
		*(*output)++ = (unsigned char)(((unsigned)value >> 8));
		break;
	case codepage::U16BE:
		*(*output)++ = (unsigned char)(((unsigned)value >> 8));
		*(*output)++ = (unsigned char)(value & 0xFF);
		break;
	default:
		*(*output)++ = (unsigned char)value;
		break;
	}
}

char* szput(char* result, unsigned sym, codepage page) {
	char* p = result;
	szput(&p, sym, page);
	*p = 0;
	return result;
}

void szencode(char* output, int output_count, codepage output_code, const char* input, int input_count, codepage input_code) {
	char* s1 = output;
	char* s2 = s1 + output_count;
	const char* p1 = input;
	const char* p2 = p1 + input_count;
	while(p1 < p2 && s1 < s2)
		szput(&s1, szget(&p1, input_code), output_code);
	if(s1 < s2) {
		s1[0] = 0;
		if((output_code == codepage::U16BE || output_code == codepage::U16LE) && (s1 + 1) < s2)
			s1[1] = 0;
	}
}

void* array::add() {
	if(count >= getmaximum()) {
		if(isgrowable())
			reserve(count + 1);
		else
			return data;
	}
	return (char*)data + size * (count++);
}

void* array::add(const void* element) {
	auto p = add();
	memcpy(p, element, getsize());
	return p;
}

void* array::addfind(const char* id) {
	auto i = find(id, 0);
	if(i!=-1)
		return ptr(i);
	auto p = add();
	*((const char**)p) = id;
	return p;
}

array::~array() {
	clear();
}

void array::clear() {
	count = 0;
	if(!isgrowable())
		return;
	count_maximum = 0;
	if(data)
		free(data);
	data = 0;
}

void array::setup(size_t size) {
	if(!isgrowable())
		return;
	clear();
	this->size = size;
}

void array::reserve(unsigned count) {
	if(!isgrowable())
		return;
	if(!size)
		return;
	if(data && count < getmaximum())
		return;
	count_maximum = rmoptimal(count);
	if(data) {
		auto p = realloc(data, count_maximum * size);
		if(!p)
			exit(0);
		data = p;
	} else
		data = malloc(count_maximum * size);
}

static bool matchstring(const char* v1, const char* v2, size_t size) {
	for(size_t n = 0; n < size; n++) {
		if(v1[n] != v2[n])
			return false;
	}
	return v1[size] == 0;
}

int array::findps(const char* value, unsigned offset, size_t size) const {
	auto m = getcount();
	for(unsigned i = 0; i < m; i++) {
		auto p = (const char**)((char*)ptr(i) + offset);
		if(!(*p))
			continue;
		if(matchstring(*p, value, size))
			return i;
	}
	return -1;
}

void* array::findv(const char* id, unsigned offset) const {
	if(!id)
		return 0;
	auto i = findps(id, offset, zlen(id));
	if(i == -1)
		return 0;
	return ptr(i);
}

int array::find(int i1, int i2, void* value, unsigned offset, size_t size) const {
	if(i2 == -1)
		i2 = getcount() - 1;
	switch(size) {
	case 4:
		for(auto i = i1; i <= i2; i++) {
			if(*((int*)value) == *((int*)((char*)ptr(i) + offset)))
				return i;
		}
		break;
	case 2:
		for(auto i = i1; i <= i2; i++) {
			if(*((int*)value) == *((int*)((char*)ptr(i) + offset)))
				return i;
		}
		break;
	case 1:
		for(auto i = i1; i <= i2; i++) {
			if(*((int*)value) == *((int*)((char*)ptr(i) + offset)))
				return i;
		}
		break;
	default:
		for(auto i = i1; i <= i2; i++) {
			if(memcmp(value, (char*)ptr(i) + offset, size) == 0)
				return i;
		}
		break;
	}
	return -1;
}

void array::sort(int i1, int i2, pcompare compare, void* param) {
	if(i2 == -1)
		i2 = count;
	if(i1 == -1)
		i1 = 0;
	for(int i = i2; i > i1; i--) {
		for(int j = i1; j < i; j++) {
			auto t1 = ptr(j);
			auto t2 = ptr(j + 1);
			if(compare(t1, t2, param) > 0)
				swap(j, j + 1);
		}
	}
}

void array::remove(int index, int elements_count) {
	if(((unsigned)index) >= count)
		return;
	if((unsigned)index < count - elements_count)
		memcpy(ptr(index), ptr(index + elements_count), (count - (index + elements_count)) * getsize());
	count -= elements_count;
}

int	array::indexof(const void* element) const {
	if(element >= data && element < ((char*)data + size * count))
		return ((char*)element - (char*)data) / size;
	return -1;
}

void* array::insert(int index, const void* element) {
	auto count_before = getcount(); add();
	memmove((char*)data + (index + 1) * size, (char*)data + index * size, (count_before - index) * size);
	void* p = ptr(index);
	if(element)
		memcpy(p, element, size);
	else
		memset(p, 0, size);
	return p;
}

void array::swap(int i1, int i2) {
	unsigned char* a1 = (unsigned char*)ptr(i1);
	unsigned char* a2 = (unsigned char*)ptr(i2);
	for(unsigned i = 0; i < size; i++) {
		char a = a1[i];
		a1[i] = a2[i];
		a2[i] = a;
	}
}

void array::shift(int i1, int i2, size_t c1, size_t c2) {
	if(i2 < i1) {
		iswap(i2, i1);
		iswap(c1, c2);
	}
	auto a1 = (char*)ptr(i1);
	auto a2 = (char*)ptr(i2);
	auto s1 = c1 * size;
	auto s2 = c2 * size;
	unsigned s = (a2 - a1) + s2 - 1;
	for(unsigned i = 0; i < s1; i++) {
		auto a = a1[0];
		memcpy(a1, a1 + 1, s);
		a1[s] = a;
	}
}

void array::shrink(unsigned offset, size_t delta) {
	if(offset + delta > size)
		return;
	auto p1 = (char*)data;
	auto p2 = (char*)data;
	const auto s2 = size - delta - offset;
	auto pe = p1 + count * size;
	while(p1 < pe) {
		if(offset) {
			memcpy(p2, p1, offset);
			p2 += offset; p1 += offset;
		}
		p1 += delta;
		if(s2) {
			memcpy(p2, p1, s2);
			p2 += s2; p1 += s2;
		}
	}
	auto new_size = size - delta;
	count_maximum = getmaximum() * size / new_size;
	size = new_size;
}

void array::grow(unsigned offset, size_t delta) {
	if(!delta)
		return;
	if(!isgrowable())
		return;
	auto new_size = size + delta;
	auto new_size_bytes = count_maximum * new_size;
	if(data)
		data = realloc(data, new_size_bytes);
	else
		data = malloc(new_size_bytes);
	auto p1 = (char*)data + new_size * count;
	auto p2 = (char*)data + size * count;
	auto s2 = size - offset;
	while(p1 > data) {
		if(s2) {
			memcpy(p1 - s2, p1 - s2, s2);
			p2 -= s2; p1 -= s2;
		}
		p1 -= delta;
		if(offset) {
			memcpy(p1, p2, offset);
			p2 -= offset; p1 -= offset;
		}
	}
	size = new_size;
}

void array::zero(unsigned offset, size_t delta) {
	if(!delta)
		return;
	auto pe = (char*)data + size * count;
	for(auto p = (char*)data + offset; p < pe; p += size)
		memset(p, 0, delta);
}

void array::change(unsigned offset, int size) {
	if(!size)
		return;
	if(size > 0) {
		grow(offset, size);
		zero(offset, size);
	} else
		shrink(offset, -size);
}

const void* array::findu(const void* value, size_t size) const {
	auto p = (char*)data;
	auto pe = (char*)data + count*(this->size);
	auto s = *((char*)value);
	while(p < pe) {
		p = (char*)memchr(p, s, pe - p);
		if(!p || (unsigned(pe - p) < size))
			break;
		if(memcmp(p, value, size) == 0)
			return p;
		p++;
	}
	return 0;
}

void* array::addu(const void* element, unsigned count) {
	auto s = count * size;
	auto p = findu(element, s);
	if(p)
		return (void*)p;
	auto ci = this->count;
	reserve(this->count + count);
	auto pr = ptr(ci);
	memcpy(pr, element, s);
	this->count += count;
	return pr;
}

const char* array::findus(const char* value, size_t size) const {
	auto p = (char*)data;
	auto pe = (char*)data + count * (this->size);
	auto s = *value;
	while(p < pe) {
		p = (char*)memchr(p, s, pe - p);
		if(!p || (unsigned(pe - p) < size))
			break;
		if(memcmp(p, value, size) == 0 && p[size]==0)
			return p;
		p++;
	}
	return 0;
}

const char* array::addus(const char* element, unsigned count) {
	auto p = findus(element, count);
	if(p)
		return p;
	auto ci = this->count;
	reserve(this->count + count + 1);
	auto pr = (char*)ptr(ci);
	memcpy(pr, element, count);
	pr[count] = 0;
	this->count += count + 1;
	return pr;
}