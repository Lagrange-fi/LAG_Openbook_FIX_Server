#include <thread>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <memory>
#include <sharedlib/include/Logger.h>
#include <serum/SERUM_Data_session.hpp>
#include <serum/SERUM_Order_sandbox_session.hpp>

#include <SerumDEX/SerumMD.h>
#include <SerumDEX/PoolRequester/PoolsRequester.h>
#include <serum/ConsoleLogger.h>
#include <serum/SerumSettings.h>

bool go_exit = false;
class SERUM_Data_session;

// Define the function to be called when ctrl-c (SIGINT) is sent to process
void signal_callback_handler(int signum) {
    std::cout << "Caught ctrl+c signal " << signum << std::endl;
    // Terminate program
    go_exit = true;
}

int main(int argc, char **argv) {

    bool order_part = false;
    bool md_part = false;
    for (int i = 0; i < argc; i++){
        auto arg = std::string(argv[i]);
        if (arg == "-m")md_part = true;
        if (arg == "-t")order_part = true;
    }
    if(order_part)
        printf("Hello Trade FixServer\n");
    if(md_part)
        printf("Hello Stream FixServer\n");

    std::shared_ptr < ILogger > logger (new ConsoleLogger);
    std::shared_ptr < ISettings > settings (new SerumSettings);
    std::shared_ptr < IBrokerClient > serumClient( std::shared_ptr <IBrokerClient>(
            new SerumMD(logger,settings, std::make_shared< PoolsRequester >( logger, settings ),
            [](const std::string &exchangeName, marketlib::broker_event, const std::string &details) {}))
    );
    if(md_part)
    {
         try {
             logger->Info((boost::format("Session | Serum DEX start ")).str().c_str());
             serumClient->start();
        }
        catch(std::exception& ex)
        {
            logger->Error((boost::format("Session | Serum DEX start exception(%1%)")% ex.what()).str().c_str());
        }
    }


    std::string conf_file = "fix_server.xml";
    std::unique_ptr<FIX8::ServerSessionBase> ms_md(
            new FIX8::ServerSession<SERUM_Data_session>(FIX8::SERUM_Data::ctx(), conf_file, "SERUM_MD"));
    std::unique_ptr<FIX8::ServerSessionBase> ms_ord_sand(
            new FIX8::ServerSession<SERUM_Order_sandbox_session>(FIX8::SERUM_Data::ctx(), conf_file, "SERUM_ORD_SAND"));

    typedef std::shared_ptr<FIX8::SessionInstanceBase> ServerSession;
    std::vector<ServerSession> sessions;

    while(!go_exit)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if(!sessions.empty()) {
            int count = sessions.size();
            sessions.erase(std::remove_if(sessions.begin(), sessions.end(), [](ServerSession &sess) {
                    if (sess->session_ptr()->is_shutdown())
                        printf("Erase session: %s\n", sess->session_ptr()->get_sid().get_id().c_str());
                    return sess->session_ptr()->is_shutdown();
                }),sessions.end()
            );
            if(count != sessions.size()) {
                printf("Session removed, count= %d\n", (int) sessions.size());
            }
        }
        if(md_part)
        if (ms_md->poll())
        {
            ServerSession inst(ms_md->create_server_instance());
            ((SERUM_Data_session*)(inst->session_ptr()))->setupDataSubscriber(serumClient);
            sessions.push_back(inst);
            printf("Session added, count= %d\n", (int)sessions.size());
            //inst->session_ptr()->control() |= FIX8::Session::print;
            //FIX8::GlobalLogger::log("global_logger");
            //const FIX8::ProcessModel pm(ms->get_process_model(ms->_ses));
            inst->start(false);
        }
        // if(order_part)
        // if (ms_ord_sand->poll())
        // {
        //     std::shared_ptr<FIX8::SessionInstanceBase> inst(ms_ord_sand->create_server_instance());
        //     sessions.push_back(inst);
        //     printf("OSession added, count= %d\n", (int)sessions.size());
        //     //inst->session_ptr()->control() |= FIX8::Session::print;
        //     //FIX8::GlobalLogger::log("global_logger");
        //     //const FIX8::ProcessModel pm(ms->get_process_model(ms->_ses));
        //     inst->start(false);
        // }
    }

    std::for_each(sessions.begin(),sessions.end(),[](ServerSession& sess)
    {
        if(!sess->session_ptr()->is_shutdown())
        {
            sess->session_ptr()->stop();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if(md_part)
    {
        serumClient->stop();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

