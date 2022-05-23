#pragma once

#include <string>
#include <type_traits>
#include <boost/format.hpp>
#include <boost/json/src.hpp>
#include <boost/json.hpp>

#include "BrokerLib/IBrokerClient.h"
#include "BrokerLib/BrokerModels.h"

using namespace std;

namespace SerumAdapter {
	static std::string subscriptionModelToString(IBrokerClient::SubscriptionModel model) {
		static const std::string values[]{
			"level1", "level2"
		};
		return values[static_cast < int > (model)];
	}

	static std::string getMarket(const BrokerModels::Instrument& instr) {
		return instr.first + "/" + instr.second;
	}
};