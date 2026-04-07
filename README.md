# Chat-server-cpp
Cliente e servidor escrito em C++ de um aplicativo de comunicação via HTTP com criptografia AES feito para linux
# Instalação:
Necessita de ter no seu Linux o OpenSSL e CMake instalado.Para instalar usa-se os comandos:
```
mkdir -pv build
cd build
cmake ..
```
* Para instalação do cliente e servidor:
```
make
```
* Para apenas do cliente:
```
make chat-client-cpp
```
* Para apenas o servidor:
```
make chat-server-cpp
```
* Para instalação central:
```
sudo make install
```
# Uso
O uso do programa, caso não seja instalado de forma central é usando ```./chat-client-cpp``` para o cliente e ```./chat-server-cpp``` para o servidor, caso de forma central, usa-se apenas ```chat-client-cpp``` para o cliente e ```chat-server-cpp``` para o servidor.

De que forma executar:
* cliente:
```
chat-client-cpp --user [USUARIO] --to [USUARIO_REQUISITADO] --endereco [IP ou DNS] --port [PORTA]
```
* servidor:
```
chat-server-cpp --port [PORTA] --threads [NUMERO]
```
