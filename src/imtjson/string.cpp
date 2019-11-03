/*
 * string.cpp
 *
 *  Created on: 14. 10. 2016
 *      Author: ondra
 */


#include "basicValues.h"
#include "string.h"

#include <cstring>
#include <sstream>
#include "stringValue.h"

#include "array.h"
#include "streams.h"
#include "utf8.h"


namespace json {



String::String():impl(AbstractStringValue::getEmptyString())
{
}

String::String(Value v):impl(v.getHandle()->unproxy()) {
}

String::String(std::size_t sz, std::function<std::size_t(char *)> fillFn):impl(new(sz) StringValue(nullptr, sz, fillFn)) {
}


StringView<char> String::substr(std::size_t pos, std::size_t length) const {
	return impl->getString().substr(pos,length);
}


StringView<char> String::substr(std::size_t pos) const {
	return impl->getString().substr(pos);
}

StringView<char> String::left(std::size_t length) const {
	return impl->getString().substr(0,length);
}

StringView<char> String::right(std::size_t length) const {
	std::size_t l = impl->getString().length;
	if (length > l) length = l;
	return impl->getString().substr(l - length, length);
}

std::size_t String::length() const {
	return impl->getString().length;
}

bool String::empty() const {
	return length() == 0;
}

char String::operator [](std::size_t pos) const {
	StringView<char> s = impl->getString();
	if (pos >= s.length) return 0;
	else return s[pos];
}

String::String(const std::initializer_list<StringView<char> >& strlist) {
	std::size_t cnt = 0;
	for (auto &&item : strlist) {
		cnt += item.length;
	}

	impl = new(cnt) StringValue(nullptr, cnt, [&](char *buff) {
		for (auto &&item : strlist) {
			memcpy(buff, item.data, item.length);
			buff+=item.length;
		}
		return cnt;
	});
}


String operator+(StringView<char> a, StringView<char> b) {
	if (a.empty() && b.empty()) return String();

	std::string v;
	v.reserve(a.length+b.length);
	v.append(a.data,a.length);
	v.append(b.data,b.length);

	return String(std::move(v));
}

/*String String::operator +(const StringView<char>& other) const {
	if (other.empty()) return *this;

	StringView<char> a = impl->getString();
	StringView<char> b = other;
	std::string v;
	v.reserve(a.length+b.length);
	v.append(a.data,a.length);
	v.append(b.data,b.length);



	return String(new StringValue(std::move(v)));

}*/



String::operator StringView<char>() const {
	return impl->getString();
}

StringView<char> String::str() const {
	return impl->getString();
}

const char* String::c_str() const {
	return impl->getString().data;
}


String String::insert(std::size_t pos, const StringView<char>& what) {
	using namespace std;

	if (what.empty()) return *this;



	StringView<char> a = impl->getString();
	if (pos > a.length) pos = a.length;
	std::size_t sz = a.length+what.length;

	return String(new(sz) StringValue(nullptr, sz,[&](char *trg) {
		std::memcpy(trg, a.data, pos);
		std::memcpy(trg+pos,what.data,what.length);
		std::memcpy(trg+pos+what.length,a.data+pos,a.length-pos);
		return sz;
	}));


}

String String::replace(std::size_t pos, std::size_t size,
		const StringView<char>& what) {
	StringView<char> a = impl->getString();
	if (pos > a.length) pos = a.length;
	if (pos+size > a.length) size = a.length - pos;

	std::size_t needsz = a.length+what.length - size;
	return String(new(needsz) StringValue(nullptr, needsz,[&](char *buff){
		std::memcpy(buff, a.data, pos);
		std::memcpy(buff+pos, what.data, what.length);
		std::memcpy(buff+pos+what.length, a.data+pos+size, a.length-pos-size);
		return needsz;
	}));

}

class SubStrString: public AbstractStringValue {
public:
	SubStrString(const PValue  &s, std::size_t pos, std::size_t length)
		:parent(s),pos(pos),length(length) {}

	virtual StringView<char> getString() const override {
		return parent->getString().substr(pos,length);
	}
	virtual bool getBool() const override {return true;}


	PValue parent;
	std::size_t pos;
	std::size_t length;
};


Value String::split(const StringView<char> separator, std::size_t maxCount) const {
	Array res;
	std::size_t curPos = 0;
	StringView<char> a = impl->getString();
	std::size_t nextPos = a.indexOf(separator, curPos);
	while (nextPos != a.npos && maxCount) {
		res.add(new SubStrString(impl,curPos, nextPos-curPos));
		curPos = nextPos+separator.length;
		nextPos = a.indexOf(separator, curPos);
		maxCount --;
	}
	if (curPos == 0) {
		res.add(impl);
	} else {
		res.add(new SubStrString(impl, curPos, a.length - curPos));
	}
	return res;


}

Value::TwoValues String::splitAt(Int pos) const
{
	std::size_t sz = length();
	if (pos > 0) {
		if ((unsigned)pos < sz) {
			return Value::TwoValues(new SubStrString(impl, 0, pos), new SubStrString(impl, pos, sz - pos));
		}
		else {
			return Value::TwoValues(*this, {});
		}
	}
	else if (pos < 0) {
		if ((int)sz + pos > 0) return splitAt((int)sz + pos);
		else return Value::TwoValues({}, *this);
	}
	else {
		return Value::TwoValues(*this, {});
	}	

}

String& String::operator +=(const StringView<char> other) {
	impl = ((*this) + other).getHandle();
	return *this;
}

std::size_t String::indexOf(const StringView<char> other, std::size_t start) const {
	StringView<char> a = impl->getString();
	return a.indexOf(other, start);
}
std::size_t String::lastIndexOf(const StringView<char> other, std::size_t start) const {
	StringView<char> a = impl->getString();
	return a.lastIndexOf(other, start);
}


PValue String::getHandle() const {
	return impl;
}

std::ostream &operator<<(std::ostream &out, const String &x) {
	StringView<char> a = x;
	out.write(a.data, a.length);
	return out;
}

String::String(const StringView<char>& str):impl(Value(str).getHandle()) {

}

String::String(const char* str):impl(Value(str).getHandle()) {
}

String::String(const std::basic_string<char>& str):impl(Value(str).getHandle()) {

}


String::String(const StringView<wchar_t> &wstr) {

	WideToUtf8 conv;
	std::vector<char> buff;
	buff.reserve(wstr.length*3);
	std::size_t pos = 0;
	conv([&]() -> int {
		if (pos < wstr.length) return wstr[pos++];
		else return eof;
	}, [&](int c) {
		buff.push_back((char)c);
	});

	StrViewA str(buff.data(), buff.size());
	impl = StringValue::create(str);
}

std::wstring json::String::wstr() const {
	Utf8ToWide conv;
	std::wostringstream buff;
	conv(fromString(str()), [&](wchar_t c){
		buff.put(c);
	});
	return buff.str();
}

}

