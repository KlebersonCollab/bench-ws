#include "crow.h"
#include <tbb/concurrent_unordered_set.h>
#include <iostream>
#include <string>

// N�mero de clientes esperados para iniciar a comunica��o
const int CLIENTS_TO_WAIT_FOR = 32;
// Conjunto de conex�es ativas
tbb::concurrent_unordered_set<crow::websocket::connection*> clients;

// Fun��o para enviar mensagem de "pronto" para todos os clientes conectados
void sendReadyMessage() {
    std::cout << "Todos os clientes conectados" << std::endl;

    for (auto& client : clients) {
        client->send_text("ready");
    }
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
        .websocket()
        .onopen([](crow::websocket::connection& conn) {
        clients.insert(&conn);
        std::string name = "Client" + std::to_string(clients.size());
        std::cout << name << " conectado (" << CLIENTS_TO_WAIT_FOR - clients.size() << " restantes)" << std::endl;

        if (clients.size() == CLIENTS_TO_WAIT_FOR) {
            sendReadyMessage();
        }
            })
        .onclose([](crow::websocket::connection& conn, const std::string& reason) {
                auto it = clients.find(&conn);
                if (it != clients.end()) {
                    clients.unsafe_erase(it);
                }
                std::cout << "Conex�o WebSocket fechada: " << reason << std::endl;
            })
                .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
                for (auto& client : clients) {
                    if (client != &conn) {
                        client->send_text(data);
                    }
                }
                    });

            // Definindo o n�mero de threads para 6
            app.port(3010).concurrency(4).run();
}
