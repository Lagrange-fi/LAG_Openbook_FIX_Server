#pragma once

#include <string>

class ISettings {

private: 

	typedef std::string string;

public:

	enum class Property {
		ExchangeName,
		WebsocketEndpoint,
	};

	virtual string get(Property) const = 0;
};

