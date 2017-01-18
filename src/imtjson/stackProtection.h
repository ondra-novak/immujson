#pragma once

#include <algorithm>
#include <cstdint>

namespace json {


	class StackProtected {
	public:
		StackProtected();
		~StackProtected();
		void checkInstance() const;


		static const std::uintptr_t cookieValue;
protected:
		std::uintptr_t cookie;


	};

}
