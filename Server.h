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
    Server(boost::asio::io_context& io_context, unsigned short tcpPort, unsigned short udpPort)
        : tcpAcceptor(io_context, tcp::endpoint(tcp::v4(), tcpPort)),
        udpSocket(io_context, udp::endpoint(udp::v4(), udpPort)),
        tcpPort(tcpPort),
        udpPort(udpPort) {
        std::cout << "Server started on TCP port " << tcpPort << " and UDP port " << udpPort << std::endl;
    }

    void Start() {
        AcceptTCPConnection();
        ReceiveUDP();
        StartConsoleInput();
    }

private:
    class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    public:
        TcpConnection(tcp::socket socket, Server& server)
            : socket_(std::move(socket)),
            server_(server),
            client_id_(++nextID) {
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

        void send_shapes(const std::vector<BaseShape>& shapes) {
            std::string serializedShapes = server_.SerializeBaseShapeVector(shapes);
            send_message(serializedShapes);
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

                        std::cout << "Client " << client_id_ << ": " << message << std::endl;

                        // Check if the message is a serialized vector of BaseShape objects
                        if (message.length() > 2 && message[0] == 'B' && message[1] == 'S') {
                            std::vector<BaseShape> shapes = server_.DeserializeBaseShapeVector(message.substr(2));
                            std::cout << "Received " << shapes.size() << " shapes from client " << client_id_ << std::endl;
                            // Process the shapes as needed
                        }
                        else {
                            // Handle regular string message
                            server_.HandleClientMessage(client_id_, message);
                        }

                        read_message();
                    }
                    else {
                        server_.HandleClientDisconnect(client_id_);
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
                        server_.HandleClientDisconnect(client_id_);
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
        static int nextID;
    };

    tcp::acceptor tcpAcceptor;
    udp::socket udpSocket;
    udp::endpoint udpSenderEndpoint;
    enum { max_length = 1024 };
    char udpData[max_length];
    unsigned short tcpPort;
    unsigned short udpPort;
    std::map<int, std::shared_ptr<TcpConnection>> tcpConnections;

    std::string SerializeBaseShapeVector(const std::vector<BaseShape>& shapes) {
        std::ostringstream oss;
        boost::archive::binary_oarchive oa(oss);
        oa << shapes;
        return "BS" + oss.str(); // Prepend "BS" to indicate serialized shapes
    }

    std::vector<BaseShape> DeserializeBaseShapeVector(const std::string& serializedData) {
        std::istringstream iss(serializedData);
        boost::archive::binary_iarchive ia(iss);
        std::vector<BaseShape> shapes;
        ia >> shapes;
        return shapes;
    }

    void AcceptTCPConnection() {
        tcpAcceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    auto connection = std::make_shared<TcpConnection>(std::move(socket), *this);
                    tcpConnections[connection->ID()] = connection;
                    connection->start();
                }
                AcceptTCPConnection();
            });
    }

    void HandleClientDisconnect(int client_id) {
        auto it = tcpConnections.find(client_id);
        if (it != tcpConnections.end()) {
            std::cout << "Client " << client_id << " disconnected from "
                << it->second->Address() << ":" << it->second->Port() << std::endl;
            tcpConnections.erase(it);
        }
    }

    void HandleClientMessage(int client_id, const std::string& message) {
        // Handle regular string messages from clients
        std::cout << "Received message from client " << client_id << ": " << message << std::endl;
        // Process the message as needed
    }

    void BroadcastTcpMessage(const std::string& message) {
        for (auto& conn : tcpConnections) {
            conn.second->send_message(message);
        }
        std::cout << "Broadcasted to " << tcpConnections.size() << " clients: " << message << std::endl;
    }

    void BroadcastShapes(const std::vector<BaseShape>& shapes) {
        std::string serializedShapes = SerializeBaseShapeVector(shapes);
        for (auto& conn : tcpConnections) {
            conn.second->send_shapes(shapes);
        }
        std::cout << "Broadcasted " << shapes.size() << " shapes to " << tcpConnections.size() << " clients" << std::endl;
    }

    void SendToClient(int client_id, const std::string& message) {
        auto it = tcpConnections.find(client_id);
        if (it != tcpConnections.end()) {
            it->second->send_message(message);
            std::cout << "Sent to client " << client_id << ": " << message << std::endl;
        }
        else {
            std::cout << "Client " << client_id << " not found" << std::endl;
        }
    }

    void StartConsoleInput() {
        std::thread input_thread([this]() {
            std::string input;
            while (std::getline(std::cin, input)) {
                if (input.substr(0, 2) == "b:") {
                    // Broadcast message
                    BroadcastTcpMessage(input.substr(2));
                }
                else if (input.substr(0, 2) == "s:") {
                    // Broadcast shapes
                    std::vector<BaseShape> shapes = { /* Create some sample shapes */ };
                    BroadcastShapes(shapes);
                }
                else if (input.substr(0, 2) == "c:") {
                    // Send to specific client
                    size_t space_pos = input.find(' ', 2);
                    if (space_pos != std::string::npos) {
                        int client_id = std::stoi(input.substr(2, space_pos - 2));
                        SendToClient(client_id, input.substr(space_pos + 1));
                    }
                }
                else if (input == "list") {
                    // List all connected clients
                    std::cout << "Connected clients:" << std::endl;
                    for (const auto& conn : tcpConnections) {
                        std::cout << "Client " << conn.first << " - "
                            << conn.second->Address() << ":" << conn.second->Port() << std::endl;
                    }
                }
            }
            });
        input_thread.detach();
    }

    void ReceiveUDP() {
        udpSocket.async_receive_from(
            boost::asio::buffer(udpData, max_length), udpSenderEndpoint,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec) {
                    std::string message(udpData, bytes_recvd);
                    std::cout << "UDP received from "
                        << udpSenderEndpoint.address().to_string()
                        << ": " << message
                        << std::endl;

                    // Echo the message back to the sender
                    udpSocket.async_send_to(
                        boost::asio::buffer(udpData, bytes_recvd),
                        udpSenderEndpoint,
                        [](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {});
                }
                ReceiveUDP();
            });
    }
};

int Server::TcpConnection::nextID = 0;

int main() {
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 8080, 8081);
        server.Start();

        std::cout << "\nServer commands:" << std::endl;
        std::cout << "b:<message> - broadcast message to all clients" << std::endl;
        std::cout << "s: - broadcast shapes to all clients" << std::endl;
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
