
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <memory>
#include <sharedlib/include/Logger.h>
#include <serum/SERUM_Data_session.hpp>

int main(int argc, char **argv) {

    printf("Hello FixServer\n");

    std::string conf_file = "hf_server.xml";
    std::unique_ptr<FIX8::ServerSessionBase> ms(
            new FIX8::ServerSession<SERUM_Data_session>(FIX8::SERUM_Data::ctx(), conf_file, "SERUM_MD"));

    XmlElement::XmlSet eset;
    std::list<std::shared_ptr<FIX8::SessionInstanceBase>> sessions;

    while(true) {
        if (!ms->poll())
            continue;
        std::shared_ptr<FIX8::SessionInstanceBase> inst(ms->create_server_instance());
        sessions.push_back(inst);
        inst->session_ptr()->control() |= FIX8::Session::print;
        FIX8::GlobalLogger::log("global_logger");
        std::cout << "client connection established - " << ms->_session_name.c_str() << std::endl;
        const FIX8::ProcessModel pm(ms->get_process_model(ms->_ses));
        inst->start(true);
    }

    int i;
    std::cin >> i;
    return 0;
}