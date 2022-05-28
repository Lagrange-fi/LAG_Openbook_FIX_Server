#pragma once

#include <string>
#include <sharedlib/include/ILogger.h>
#include "BrokerModels.h"

class IListener {
private:
	typedef std::string string;
public:

	IListener() {}
	IListener(const IListener&) = delete;
	IListener& operator = (const IListener&) = delete;

	virtual bool isEnabled() const = 0;
	virtual bool isConnected() const = 0;
	// virtual string getName() const = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void listen(const BrokerModels::Instrument&) = 0;
	virtual void unlisten(const BrokerModels::Instrument&) = 0;

	virtual ~IListener() = default;
};
