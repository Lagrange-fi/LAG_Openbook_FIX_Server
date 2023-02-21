//
// Created by Zay on 2/19/2023.
//
#pragma once

class SerumSettings : public ISettings {

private:

    typedef std::string string;

public:

    string get(Property property) const override {
        switch (property) {
            case Property::ExchangeName:
                return "Serum";
            case Property::WebsocketEndpoint:
                return "wss://vial.mngo.cloud/v1/ws";
            default:
                return "";
        }
    }
};

