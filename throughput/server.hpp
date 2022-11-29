#include "net.hpp"

class Server {
   private:
    virtual void handle(string msg, int fd) = 0;

   public:
    Server() {}

    void run(string ip, double time_period) {
        int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
        int sock_opt;
        assert(setsockopt(listen_fd, SOL_SOCKET,
                          TCP_NODELAY | SO_REUSEADDR | SO_REUSEPORT, &sock_opt,
                          sizeof(sock_opt)) >= 0);
        struct sockaddr_in servaddr = string_to_struct_addr(ip);
        assert(bind(listen_fd, (sockaddr *)&servaddr, sizeof(servaddr)) >= 0);
        assert(listen(listen_fd, 100) >= 0);

        system_clock::time_point total_start, total_end;
        bool started = false;
        int handled_task = 0;
        bool found = false;

        system_clock::time_point start, end;
        while (true) {
            // accept for one connection
            struct sockaddr_in clientaddr;
            socklen_t clientaddrlen = sizeof(clientaddr);
            int client_fd = accept(listen_fd, (struct sockaddr *)&clientaddr,
                                   &clientaddrlen);
            string recv_msg;

            start = system_clock::now();
            if (!recvMsg(client_fd, recv_msg)) {
                cout << "receive msg fail\n";
                continue;
            }
            if (!started) {
                total_start = system_clock::now();
                started = true;
            }

            // invoke handler function of server class
            handle(recv_msg, client_fd);
            end = system_clock::now();

            cout << "server handle time: "
                 << duration_cast<std::chrono::duration<double>>(end - start)
                        .count()
                 << endl;

            handled_task++;

            total_end = system_clock::now();
            cout << "elapsed time: "
                 << duration_cast<std::chrono::duration<double>>(total_end -
                                                                 total_start)
                        .count()
                 << ", handled task: " << handled_task << endl;
            if (!found && duration_cast<std::chrono::duration<double>>(
                              total_end - total_start)
                                  .count() > time_period) {
                cout << "FOUND throughput " << handled_task << " per "
                     << time_period << "s" << endl;
                found = true;
            }

            close(client_fd);
        }
    }
};