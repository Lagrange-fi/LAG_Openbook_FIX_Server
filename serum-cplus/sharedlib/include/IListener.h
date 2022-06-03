#pragma once

#include <string>
#include "ILogger.h"
#include <marketlib/include/BrokerModels.h>

class IListener {
private:
	typedef std::string string;
	typedef marketlib::instrument_descr_t Instrument;

public:

	IListener() {}
	IListener(const IListener&) = delete;
	IListener& operator = (const IListener&) = delete;

	virtual bool isEnabled() const = 0;
	virtual bool isConnected() const = 0;
	// virtual string getName() const = 0;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void listen(const Instrument&) = 0;
	virtual void unlisten(const Instrument&) = 0;

	virtual ~IListener() = default;
};
