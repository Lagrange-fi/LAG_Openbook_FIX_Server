#pragma once

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

#include <functional>
#include <chrono>
#include <atomic>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/platforms.hpp>

#include "ILogger.h"

//	#define WEBSOCKET_DEBUG

template < class Handler >
class ConnectionWrapper {

private:
	
	friend Handler;

	typedef std::string string;
	typedef boost::asio::ssl::context context;
	typedef std::shared_ptr < context > context_ptr;
	typedef websocketpp::client < websocketpp::config::asio_tls_client > client;
	typedef websocketpp::connection_hdl connection_hdl;
	typedef client::message_ptr message_ptr;
	typedef client::connection_ptr connection_ptr;
	typedef std::shared_ptr < std::thread > thread_ptr;
	typedef std::error_code error_code;
	typedef std::chrono::seconds duration;
	typedef std::mutex mutex;
	typedef std::lock_guard < mutex > lock_guard;
	typedef std::shared_ptr < ILogger > logger_ptr;
	typedef std::exception exception;
	typedef std::atomic_bool atomic_bool;

	mutex sendLock;
	mutex connectionLock;
	mutex toggleLock;
	thread_ptr thread;
	client endpoint;
	Handler* handler;
	string uri;
	connection_hdl hdl;
	duration timeout;
	atomic_bool enabled;
	atomic_bool connected;
	logger_ptr logger;

	context_ptr onTlsInit() {
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::onTlsInit");
	#endif
		context_ptr ctx = std::make_shared < context > (context::sslv23);
		try {
			ctx->set_options(
				context::default_workarounds |
				context::no_sslv2 |
				context::no_sslv3 |
				context::single_dh_use
			);
		} catch (exception &ex) {
			// logger->Error("Tls initialization error => {}"_format(ex.what()).c_str());
		}
		return ctx;
	}

	void async_connect() {
		lock_guard guard(connectionLock);
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::connect");
	#endif
		if (connected || !enabled) {
			return;
		}
		error_code ec;
		connection_ptr connection = endpoint.get_connection(uri, ec);
		if (ec) {
			// logger->Error("Socket connection error => {}"_format(ec.message()).c_str());
			return;
		}
		hdl = connection->get_handle();
		connection->set_open_handler(std::bind(&ConnectionWrapper::onOpen, this, std::placeholders::_1));
		connection->set_close_handler(std::bind(&ConnectionWrapper::onClose, this, std::placeholders::_1));
		connection->set_fail_handler(std::bind(&ConnectionWrapper::onFail, this, std::placeholders::_1));
		connection->set_message_handler(std::bind(&ConnectionWrapper::onMessage, this, std::placeholders::_1, std::placeholders::_2));
		endpoint.connect(connection);
	}
	void async_close() {
		lock_guard guard(connectionLock);
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::disconnect");
	#endif
		if (enabled || !connected) {
			return;
		}
		error_code ec;
		endpoint.close(hdl, websocketpp::close::status::normal, "", ec);
		if (ec) {
			// logger->Error("Socket disconnection error => {}"_format(ec.message()).c_str());
		}
	}

	ConnectionWrapper(Handler *_handler, const string &_uri, logger_ptr _logger) :
		handler(_handler), uri(_uri), timeout(std::chrono::seconds(1)), enabled(false), connected(false), logger(_logger)
	{
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::ConnectionWrapper");
	#endif
		endpoint.clear_access_channels(websocketpp::log::alevel::all);
		endpoint.clear_error_channels(websocketpp::log::elevel::all);
		endpoint.init_asio();
		endpoint.start_perpetual();
		endpoint.set_tls_init_handler(bind(&ConnectionWrapper::onTlsInit, this));
		thread.reset(new std::thread(&client::run, &endpoint));
	}

	void onOpen(connection_hdl) {
		connected = true;
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::onOpen");
	#endif
		handler->onOpen();
		if (!enabled) {
			async_close();
		}
	}
	void onClose(connection_hdl) {
		connected = false;
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::onClose");
	#endif
		handler->onClose();
		if (enabled) {
			std::this_thread::sleep_for(timeout);
			async_connect();
		}
	}
	void onFail(connection_hdl) {
		connected = false;
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::onFail");
	#endif
		handler->onFail();
		if (enabled) {
			std::this_thread::sleep_for(timeout);
			async_connect();
		}
	}
	void onMessage(connection_hdl, message_ptr message) {
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::onMessage => {}"_format(message->get_payload()));
	#endif
		handler->onMessage(message->get_payload());
	}

	void async_start() {
		lock_guard guard(toggleLock);
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::start");
	#endif
		if (enabled) {
			return;
		}
		enabled = true;
		async_connect();
	}
	void async_stop() {
		lock_guard guard(toggleLock);
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::stop");
	#endif
		if (!enabled) {
			return;
		}
		enabled = false;
		async_close();
	}

	void async_send(const string &message) {
		lock_guard guard(sendLock);
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::send => {}"_format(message));
	#endif
		if (!enabled || !connected) {
			return;
		}
		error_code ec;
		endpoint.send(hdl, message, websocketpp::frame::opcode::text, ec);
		if (ec) {
			// logger->Error("Error sending message => {}"_format(ec.message()).c_str());
		}
	}

	~ConnectionWrapper() {
	#ifdef WEBSOCKET_DEBUG
		logger->debug("> ConnectionWrapper::~ConnectionWrapper");
	#endif
		async_stop();
		while (connected) continue;
		endpoint.stop_perpetual();
		thread->join();
	}

};

