//Net
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
//Std
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <cstring>
//OpenSSL
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define INVALIDO temp=="--pin"||temp=="--crypt"||temp=="--endereco"||temp=="--user"||temp=="--help"||temp=="--user"

void kill_prog_error(std::string error){
    std::cout<<error;
    exit(EXIT_FAILURE);
}

std::string sha256(const std::string &input) {
    unsigned char hash[32];
    SHA256((unsigned char*)input.data(), input.size(), hash);
    return std::string((char*)hash, 32);
}

std::string aes_encrypt(const std::string &msg, const std::string &key) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";

    unsigned char iv[16];
    if (!RAND_bytes(iv, sizeof(iv))) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    std::vector<unsigned char> out(msg.size() + 16);

    int len = 0, total = 0;

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,(unsigned char*)key.data(), iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    if (!EVP_EncryptUpdate(ctx, out.data(), &len,(unsigned char*)msg.data(), msg.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    total += len;

    if (!EVP_EncryptFinal_ex(ctx, out.data() + total, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    total += len;

    EVP_CIPHER_CTX_free(ctx);

    return std::string((char*)iv, 16) + std::string((char*)out.data(), total);
}

std::string aes_decrypt(const std::string &msg, const std::string &key) {
    if (msg.size() < 16) return "";

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";

    unsigned char iv[16];
    std::memcpy(iv, msg.data(), 16);

    std::string real = msg.substr(16);
    std::vector<unsigned char> out(real.size());

    int len = 0, total = 0;

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
        (unsigned char*)key.data(), iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    if (!EVP_DecryptUpdate(ctx, out.data(), &len,
        (unsigned char*)real.data(), real.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    total += len;

    if (!EVP_DecryptFinal_ex(ctx, out.data() + total, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    total += len;

    EVP_CIPHER_CTX_free(ctx);

    return std::string((char*)out.data(), total);
}

void redraw(const std::vector<std::string> &hist) {
    std::cout << "\033[H\n\n";
    for (auto &m : hist) std::cout << m << "\n";
    std::cout << ">> " << std::flush;
}

int main(int argc, char **argv) {
    std::string user, other, pin, addr;
    bool use_crypto = false;
    int port=9999;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];

        if (a=="--user"){
            if(i+1<argc){
                std::string temp=argv[++i];
                if(INVALIDO){
                    kill_prog_error("ERRO: Nome de usuário invalido.\n");
                }
                user = temp;
            }
            else{
                kill_prog_error("ERRO: necessita de complemento.\n");
            }
        }
        else if (a=="--to"){
            if(i+1<argc){
                std::string temp=argv[++i];
                if(INVALIDO){
                    kill_prog_error("ERRO: Nome invalido.\n");
                }
                other = temp;
            }
            else{
                kill_prog_error("ERRO: necessita de complemento.\n");
            }
        }
        else if (a=="--pin") {
            if(use_crypto==false){
                kill_prog_error("ERRO: não habilitado criptografia primeiro\n");
            }
            if(i+1<argc){
                std::string temp=argv[++i];
                if(INVALIDO){
                    kill_prog_error("ERRO: Nome invalido.\n");
                }
                std::ifstream f(temp);
                if(!f.is_open()){
                    kill_prog_error("ERRO: não foi possível abrir o pin");
                }
                std::string line;
                while (getline(f, line)) pin += line;
                if(pin.empty()){
                    kill_prog_error("ERRO: não foi possivel pegar o pin.\n");
                }
            }
            else{
                kill_prog_error("ERRO: necessita de complemento.\n");
            }

        }
        else if (a=="--crypt") {
            if(i+1<argc){
                std::string temp=argv[++i];
                if(temp=="s"){
                    use_crypto=true;
                }else if(temp=="n"){
                    use_crypto=false;
                }
                else{
                    kill_prog_error("ERRO: Invalido a opção de negar ou aceitar criptografia\n");
                }
            }
            else{
                kill_prog_error("ERRO: necessita de complemento.\n");
            }
        }
        else if (a == "--endereco"){
             if(i+1<argc){
                std::string temp=argv[++i];
                if(INVALIDO){
                    kill_prog_error("ERRO: Nome invalido.\n");
                }
                addr=temp;
            }
            else{
                kill_prog_error("ERRO: necessita de complemento.\n");
            }
        }
        else if(a=="--port"){
            if(i+1<argc){
                std::string temp=argv[++i];
                if(INVALIDO){
                    kill_prog_error("ERRO: valor para porta invalida.\n");
                }
                port=std::atoi(temp.c_str());
            }
            else{
                kill_prog_error("ERRO: necessita de complemento.\n");
            }
        }
        else if (a=="--help") {
            std::cout
            <<"App de comunicação via http feito em C++ com capacidade de criptografia AES com libssl\n"
            <<"\t\t\t\t\tFLAGS:\n"
            <<">> \t\t--user [USUARIO]\t\t\tDefine seu nome no servidor;\n"
            <<">> \t\t--to [OUTRO]\t\t\t\tDefine para quem vai a mensagem;\n"
            <<">> \t\t--endereco [IP]\t\t\t\tEndereco do server que estara em execucao;\n"
            <<">> \t\t--crypt [s/n]\t\t\t\tDefine se quer criptografar as mensagens;\n"
            <<">> \t\t--pin [FILE]\t\t\t\tDefine onde fica o arquivo do pin;\n" 
            <<">> \t\t--port [PORTA]\t\t\t\tNumero da porta do servidor;\n"
            <<"{UTILIZACAO_SEM_CRIPT}:\tchat-client-cpp --user [USUARIO] --to [OUTRO] --endereco [IP] --port [PORTA]\n"
            <<"{UTILIZACAO_COM_CRIPT}:\tchat-client-cpp --user [USUARIO] --to [OUTRO] --endereco [IP] --port [PORTA] --crypt [s/n] --pin [FILE]\n";
            return 0;
        }
        else{
            std::string error="ERRO: incorreto a sintaxe do programa, "+a+" inexistente\n";
            kill_prog_error(error);
        }
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in s{};
    s.sin_family = AF_INET;
    s.sin_port = htons(port);
    inet_pton(AF_INET, addr.c_str(), &s.sin_addr);

    if(connect(sockfd, (sockaddr*)&s, sizeof(s))<0){
        kill_prog_error("ERRO: servidor inexistente neste ip\n");
    }

    std::string metodo = use_crypto ? "AES" : "NONE";
    std::string hello = "<" + metodo + ">\"" + user + "\"";
    send(sockfd, hello.data(), hello.size(), 0);

    struct pollfd fds[2] = {
        {0, POLLIN, 0},
        {sockfd, POLLIN, 0}
    };

    std::vector<std::string> hist;
    redraw(hist);

    std::string key = sha256(pin);
    std::system("clear");

    while (true) {
        int ret = poll(fds, 2, 3000);
        if (ret < 0){
            kill_prog_error("ERRO:Pool falhou\n");
        }
        if (fds[0].revents & POLLIN) {
            std::string line;
            std::getline(std::cin, line);

            if(line=="quit()"){
                std::cout<<"\nSaindo do programa...\n";
                return 0;
            }

            hist.push_back("Voce: "+line);
            redraw(hist);

            std::string payload = line;

            if (use_crypto)
                payload = aes_encrypt(payload, key);

            std::string msg =
                "{" + other + "}|" + payload;

            send(sockfd, msg.data(), msg.size(), 0);
        }
        if (fds[1].revents & POLLIN) {
            char buf[512];
            int n = recv(sockfd, buf, 512, 0);

            if (n <= 0) {
                kill_prog_error("ERRO:Servidor desconectado\n");
            }

            std::string msg(buf, n);
            size_t sep = msg.find(":|");

            if (sep != std::string::npos) {
                std::string origem = msg.substr(0, sep);
                std::string payload = msg.substr(sep + 2);

                if (use_crypto)
                    payload = aes_decrypt(payload, key);

                hist.push_back(origem + ": " + payload);
                redraw(hist);
            }
        }
    }
}