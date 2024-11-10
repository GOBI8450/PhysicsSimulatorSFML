#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <deque>
#include <mutex>
// message_types.hpp
#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <sstream>
#include <set>
#include "../PhysicSSimulator/BaseShape.h"
#include "../PhysicSSimulator/SFMLSerialization.hpp"
#include "../PhysicSSimulator/PhysicsSimulatotion.h"


using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Server {
public:
    Server(boost::asio::io_context& io_context, unsigned short tcp_port, unsigned short udp_port)
        : tcp_acceptor_(io_context, tcp::endpoint(tcp::v4(), tcp_port)),
        udp_socket_(io_context, udp::endpoint(udp::v4(), udp_port)),
        tcp_port_(tcp_port),
        udp_port_(udp_port) {
        std::cout << "Server started on TCP port " << tcp_port_ << " and UDP port " << udp_port_ << std::endl;
    }

    void start() {
        accept_tcp_connection();
        receive_udp();
        start_console_input();
    }

private:
    class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    public:
        TcpConnection(tcp::socket socket, Server& server)
            : socket_(std::move(socket)),
            server_(server),
            client_id_(++next_id_) {
            address_ = socket_.remote_endpoint().address().to_string();
            port_ = socket_.remote_endpoint().port();
        }

        void start() {
            std::cout << "Client " << client_id_ << " connected from " << address_ << ":" << port_ << std::endl;
            read_message();
        }

        void send_message(const std::string& message) {
            bool write_in_progress = !message_queue_.empty();
            message_queue_.push_back(message + "\n");
            if (!write_in_progress) {
                do_write();
            }
        }

        tcp::socket& Socket() { return socket_; }
        const std::string& Address() const { return address_; }
        unsigned short Port() const { return port_; }
        int ID() const { return client_id_; }

    private:
        void read_message() {
            auto self(shared_from_this());
            boost::asio::async_read_until(
                socket_,
                buffer_,
                '\n',
                [this, self](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::string message(boost::asio::buffers_begin(buffer_.data()),
                            boost::asio::buffers_begin(buffer_.data()) + length);
                        buffer_.consume(length);

                        std::cout << "Client " << client_id_ << ": " << message;

                        read_message();
                    }
                    else {
                        server_.handle_client_disconnect(client_id_);
                    }
                });
        }

        void do_write() {
            auto self(shared_from_this());
            boost::asio::async_write(
                socket_,
                boost::asio::buffer(message_queue_.front()),
                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        message_queue_.pop_front();
                        if (!message_queue_.empty()) {
                            do_write();
                        }
                    }
                    else {
                        server_.handle_client_disconnect(client_id_);
                    }
                });
        }

        tcp::socket socket_;
        Server& server_;
        boost::asio::streambuf buffer_;
        std::deque<std::string> message_queue_;
        std::string address_;
        unsigned short port_;
        int client_id_;
        static int next_id_;
    };

    std::string serializeBaseShapeVector(const std::vector<BaseShape>& shapes) {
        std::ostringstream oss;                    // Output string stream to hold binary data
        boost::archive::binary_oarchive oa(oss);   // Binary output archive for serialization
        oa << shapes;                              // Serialize the vector of BaseShape
        return oss.str();                          // Return serialized data as a string
    }

    std::vector<BaseShape> deserializeBaseShapeVector(const std::string& serializedData) {
        std::istringstream iss(serializedData);
        boost::archive::binary_iarchive ia(iss);
        std::vector<BaseShape> shapes;
        ia >> shapes;  // Deserialize the vector of BaseShape
        return shapes;
    }

    void accept_tcp_connection() {
        tcp_acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    auto connection = std::make_shared<TcpConnection>(std::move(socket), *this);
                    tcp_connections_[connection->ID()] = connection;
                    connection->start();
                }
                accept_tcp_connection();
            });
    }

    void handle_client_disconnect(int client_id) {
        auto it = tcp_connections_.find(client_id);
        if (it != tcp_connections_.end()) {
            std::cout << "Client " << client_id << " disconnected from "
                << it->second->Address() << ":" << it->second->Port() << std::endl;
            tcp_connections_.erase(it);
        }
    }

    void broadcast_tcp_message(const std::string& message) {
        for (auto& conn : tcp_connections_) {
            conn.second->send_message("Broadcast: " + message);
        }
        std::cout << "Broadcasted to " << tcp_connections_.size() << " clients: " << message << std::endl;
    }

    void send_to_client(int client_id, const std::string& message) {
        auto it = tcp_connections_.find(client_id);
        if (it != tcp_connections_.end()) {
            it->second->send_message("Direct message: " + message);
            std::cout << "Sent to client " << client_id << ": " << message << std::endl;
        }
        else {
            std::cout << "Client " << client_id << " not found" << std::endl;
        }
    }

    void start_console_input() {
        std::thread input_thread([this]() {
            std::string input;
            while (std::getline(std::cin, input)) {
                if (input.substr(0, 2) == "b:") {
                    // Broadcast message
                    broadcast_tcp_message(input.substr(2));
                }
                else if (input.substr(0, 2) == "c:") {
                    // Send to specific client
                    size_t space_pos = input.find(' ', 2);
                    if (space_pos != std::string::npos) {
                        int client_id = std::stoi(input.substr(2, space_pos - 2));
                        send_to_client(client_id, input.substr(space_pos + 1));
                    }
                }
                else if (input == "list") {
                    // List all connected clients
                    std::cout << "Connected clients:" << std::endl;
                    for (const auto& conn : tcp_connections_) {
                        std::cout << "Client " << conn.first << " - "
                            << conn.second->Address() << ":" << conn.second->Port() << std::endl;
                    }
                }
            }
            });
        input_thread.detach();
    }

    void receive_udp() {
        udp_socket_.async_receive_from(
            boost::asio::buffer(udp_data_, max_length), udp_sender_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec) {
                    std::cout << "UDP received from "
                        << udp_sender_endpoint_.address().to_string()
                        << ": " << std::string(udp_data_, bytes_recvd)
                        << std::endl;

                    udp_socket_.async_send_to(
                        boost::asio::buffer(udp_data_, bytes_recvd),
                        udp_sender_endpoint_,
                        [](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {});
                }
                receive_udp();
            });
    }

    tcp::acceptor tcp_acceptor_;
    udp::socket udp_socket_;
    udp::endpoint udp_sender_endpoint_;
    enum { max_length = 1024 };
    char udp_data_[max_length];
    unsigned short tcp_port_;
    unsigned short udp_port_;
    std::map<int, std::shared_ptr<TcpConnection>> tcp_connections_;
};

int Server::TcpConnection::next_id_ = 0;

int main() {
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 8080, 8081);
        server.start();

        std::cout << "\nServer commands:" << std::endl;
        std::cout << "b:<message> - broadcast message to all clients" << std::endl;
        std::cout << "c:<client_id> <message> - send message to specific client" << std::endl;
        std::cout << "list - list all connected clients" << std::endl;

        std::vector<std::thread> threads;
        const int thread_count = std::thread::hardware_concurrency();

        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&io_context]() {
                io_context.run();
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }
    catch (std::exception& error) {
        std::cerr << "Exception: " << error.what() << std::endl;
    }

    return 0;
}
