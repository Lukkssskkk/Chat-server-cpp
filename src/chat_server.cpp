#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include<iostream>
#include<vector>
#include<map>
#include<thread>
#include<mutex>
#include<queue>
#include<cstring>
#include<chrono>

std::map<int, std::string> fd_user;
std::map<std::string, int> user_fd;
std::mutex mtx;
std::queue<std::pair<int,std::string>> fila;

void kill_prog_error(std::string error){
    std::cout<<error;
    exit(EXIT_FAILURE);
}

std::string extrair(const std::string &s, char a, char b) {
    size_t p1 = s.find(a), p2 = s.find(b);
    if (p1 == std::string::npos || p2 == std::string::npos) return "";
    return s.substr(p1+1, p2-p1-1);
}
void worker() {
    while (true) {
        mtx.lock();
        if (fila.empty()) {
            mtx.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        auto [fd, msg] = fila.front();
        fila.pop();
        mtx.unlock();

        // handshake
        if (msg[0] == '<') {
            size_t p1 = msg.find('"');
            size_t p2 = msg.rfind('"');

            if (p1 != std::string::npos && p2 != std::string::npos) {
                std::string user = msg.substr(p1+1, p2-p1-1);

                mtx.lock();
                fd_user[fd] = user;
                user_fd[user] = fd;
                mtx.unlock();

                std::cout << "Registrado: " << user << "\n";
            }
            continue;
        }

        std::string destino = extrair(msg, '{', '}');
        std::string origem = fd_user[fd];

        size_t sep = msg.find('|');
        std::string payload = (sep != std::string::npos) ? msg.substr(sep+1) : "";

        std::string resp = origem + ":|" + payload;

        mtx.lock();
        if (user_fd.count(destino)) {
            int dfd = user_fd[destino];
            send(dfd, resp.data(), resp.size(), 0);
        }
        mtx.unlock();
    }
}

int main(int argc, char **argv) {

    int threads = 2,port=9999;

    for (int i = 1; i < argc; i++) {
        std::string temp=argv[i];
        if (temp == "--threads"){
            if(i+1<argc){
                threads = atoi(argv[++i]);
                if(threads<=0){
                    kill_prog_error("Numero de threads invalidos\n");
                }
            }else{
                kill_prog_error("ERRO: Precisa de complemento.\n");
            }
        }
        else if(temp == "--port"){
            if(i+1<argc){
                port= std::atoi(argv[++i]);
                if(port<=0){
                    kill_prog_error("Numero da porta invalida\n");
                }
            }else{
                kill_prog_error("ERRO: Precisa de complemento.\n");
            }
        }
        else if (temp == "--help") {
            std::cout 
            <<"Servidor de aplicativo via terminal de comunicação\n"
            <<"\t\t\t\t\tFLAGS:\n"
            <<">> \t\t--port [PORTA]\t\t\t\t\tNumero da porta;\n"
            <<">> \t\t--threads [FILE]\t\t\t\tDefine quantos threads o servidor vai usar;\n" 
            <<"USO: \tchat-server-cpp --threads [NUMERO] --port [PORTA]\n";
            return 0;
        }else{
            kill_prog_error("Flag invalida\n");
        }
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (sockaddr*)&addr, sizeof(addr))<0){
        std::cout << "Erro no bind: " << strerror(errno) << "\n";
        kill_prog_error("ERRO: falhou na criacao do socket.\n");
    }
    if(listen(sockfd, 10)<0){
        kill_prog_error("ERRO: não foi possivel ouvir do socket.\n");
    }

    // threads
    std::vector<std::thread> pool;
    for (int i = 0; i < threads; i++){
        pool.emplace_back(worker);
    }

    std::cout<<"Porta do servidor: "<<port<<"\n";
    std::cout<<"Numero de threads: "<<threads<<"\n";
    std::cout<<"Servidor levantado com sucesso...\n";

    while (true) {
        int client = accept(sockfd, nullptr, nullptr);
        std::thread([client]() {
            char buf[512];

            while (true) {
                int n = recv(client, buf, 512, 0);
                if (n <= 0) break;

                std::string msg(buf, n);

                mtx.lock();
                fila.push({client, msg});
                mtx.unlock();
            }

            // remover usuário
            mtx.lock();
            if (fd_user.count(client)) {
                std::string u = fd_user[client];
                user_fd.erase(u);
                fd_user.erase(client);
            }
            mtx.unlock();

            close(client);
        }).detach();
    }
}