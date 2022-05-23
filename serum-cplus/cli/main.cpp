
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <memory>
#include <sharedlib/include/Logger.h>
#include <serum/SERUM_Data_session.hpp>

int main(int argc, char **argv) {
    printf("Hello FixServer");

    std::string conf_file = "server_stream_template.cfg";
    std::unique_ptr<FIX8::ServerSessionBase> ms(new FIX8::ServerSession<SERUM_Data_session>(FIX8::SERUM_Data::ctx(), conf_file, "Serum_Data"));

    XmlElement::XmlSet eset;

    //std::unique_ptr <FIX8::SessionInstanceBase> inst(ms->create_server_instance());
    //inst->session_ptr()->control() |= FIX8::Session::print;
    std::ostringstream sostr;
    //FIX8::GlobalLogger::log(sostr.str());
    sostr << "client connection established.";
    //const FIX8::ProcessModel pm(ms->get_process_model(ms->_ses));
    //inst->start(pm == FIX8::pm_thread);

    int i;
    std::cin >> i;
    return 0;
}