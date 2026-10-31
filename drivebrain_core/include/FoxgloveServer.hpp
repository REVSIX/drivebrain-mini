#pragma once

#include <foxglove/websocket/base64.hpp>
#include <foxglove/websocket/websocket_notls.hpp>
#include <foxglove/websocket/websocket_server.hpp>
#include <foxglove/websocket/server_factory.hpp>
#include <google/protobuf/descriptor.pb.h>
#include <MCAPLogger.hpp>
#include <string>
#include <atomic>
#include <cassert>
#include <spdlog/spdlog.h>

namespace core {

    class FoxgloveServer {
        public:
            /**
             *  Initialzes a new Foxglove server singleton instance
             */
            static void create();

            /**
             * Fetches Foxglove server singleton instance
             *
             * @retun FoxgloveServer instance
             */
            static FoxgloveServer& instance();

            /**
             * Destroys the Foxglove server singleton instance, stopping the server and freeing resources
             */
            static void destroy();

            /**
             * Destructs the foxglove server instance by stopping the server.
             */
            ~FoxgloveServer() {
                _server->stop();
                spdlog::info("Destructed and stopped foxglove websocket server");
            }

            /**
             * Sends a protobuf to be viewed in Foxglove.
             *
             * @param msg the message to be sent
             * @param topic topic to send on, defaults to the message type name. A new topic gets its channel on first use.
             */
            void send_live_telem_msg(std::shared_ptr<google::protobuf::Message> msg, const std::string &topic = "");

        private:
            FoxgloveServer();

            /* Singleton move semantics */
            FoxgloveServer(const FoxgloveServer&) = delete;
            FoxgloveServer& operator=(const FoxgloveServer&) = delete;

            /* Singleton instance */
            inline static std::atomic<FoxgloveServer*> _s_instance;

            std::unordered_map<std::string, uint32_t> _name_to_id_map;

            std::unique_ptr<foxglove::ServerInterface<websocketpp::connection_hdl>> _server;
            foxglove::ServerOptions _server_options;

            std::mutex _channel_mutex;
    };
}
