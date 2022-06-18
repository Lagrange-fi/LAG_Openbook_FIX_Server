#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <thread>


#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <memory>
#include <sharedlib/include/Logger.h>
#include <serum/SERUM_Data_session.hpp>
#include <serum/SERUM_Order_sandbox_session.hpp>

/*
    to stop the daemon, find its PID using the command 
        ps -ef
    
    then send him the SIGINT command
        kill -2 <PID>
*/

volatile bool go_exit = false;

void sigint_handler(int sig)
{
    go_exit = true;
}

static void daemon(std::function<void()>);

int main(int argc, char **argv) {
    daemon([argc, argv](){
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

        std::string conf_file = "fix_server.xml";
        std::unique_ptr<FIX8::ServerSessionBase> ms_md(
                new FIX8::ServerSession<SERUM_Data_session>(FIX8::SERUM_Data::ctx(), conf_file, "SERUM_MD"));
        std::unique_ptr<FIX8::ServerSessionBase> ms_ord_sand(
                new FIX8::ServerSession<SERUM_Order_sandbox_session>(FIX8::SERUM_Data::ctx(), conf_file, "SERUM_ORD_SAND"));

        typedef std::shared_ptr<FIX8::SessionInstanceBase> ClientSession;
        std::vector<ClientSession> sessions;

        while(!go_exit)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if(!sessions.empty()) {
                int count = sessions.size();
                sessions.erase(std::remove_if(sessions.begin(), sessions.end(), [](ClientSession &sess) {
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
                    std::shared_ptr<FIX8::SessionInstanceBase> inst(ms_md->create_server_instance());
                    sessions.push_back(inst);
                    printf("Session added, count= %d\n", (int)sessions.size());
                    //inst->session_ptr()->control() |= FIX8::Session::print;
                    //FIX8::GlobalLogger::log("global_logger");
                    //const FIX8::ProcessModel pm(ms->get_process_model(ms->_ses));
                    inst->start(false);
                }
            if(order_part)
                if (ms_ord_sand->poll())
                {
                    std::shared_ptr<FIX8::SessionInstanceBase> inst(ms_ord_sand->create_server_instance());
                    sessions.push_back(inst);
                    printf("OSession added, count= %d\n", (int)sessions.size());
                    //inst->session_ptr()->control() |= FIX8::Session::print;
                    //FIX8::GlobalLogger::log("global_logger");
                    //const FIX8::ProcessModel pm(ms->get_process_model(ms->_ses));
                    inst->start(false);
                }
        }

        std::for_each(sessions.begin(),sessions.end(),[](ClientSession& sess)
        {
            if(!sess->session_ptr()->is_shutdown())
            {
                sess->session_ptr()->stop();
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });

}
   
static void daemon(std::function<void()> payload)
{
    pid_t pid;
    
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    if (setsid() < 0) {
        printf("Could not generate session ID for child process");
        exit(EXIT_FAILURE);
    }

    umask(0);    
    chdir("./");

    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    openlog ("FixServerLog", LOG_PID, LOG_DAEMON);
    signal(SIGINT, sigint_handler);
    payload();
    closelog();
}