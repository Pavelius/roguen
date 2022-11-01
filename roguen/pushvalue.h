#pragma once

template<typename T>
class pushvalue {
	T&		ref;
	T		value;
public:
	pushvalue(T& ref) : ref(ref), value(ref) {}
	pushvalue(T& ref, T nv) : ref(ref), value(ref) { ref = nv; }
	~pushvalue() { ref = value; }
	operator T() const { return value; }
};
