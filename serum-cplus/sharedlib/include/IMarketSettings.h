#pragma once

#include <string>

class IMarketSettings {

private: 

	typedef std::string string;

public:

	enum class Property {
		ClientId,
		PrivateKey,
		Endpoint
	};

	virtual string get(Property) const = 0;
};