#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <cstring>

namespace beast = boost::beast;
namespace websocket = boost::beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

using tcp = net::ip::tcp;
using json = nlohmann::json;

// =====================================
// BINARY MARKET DATA STRUCT
// =====================================

#pragma pack(push, 1)

struct MarketDataBinary
{
    double best_bid;
    double best_ask;

    double bid_qty;
    double ask_qty;

    double spread;
    double imbalance;
    double volatility;

    long long processing_latency_us;
};

#pragma pack(pop)

// =====================================
// GLOBAL BINARY CACHE
// =====================================

MarketDataBinary cache;

// =====================================
// MAIN
// =====================================

int main()
{
    try
    {
        // IO Context
        net::io_context ioc;

        // SSL Context
        ssl::context ctx(
            ssl::context::tlsv12_client
        );

        // Resolver
        tcp::resolver resolver(ioc);

        // WebSocket Stream
        websocket::stream<
            beast::ssl_stream<tcp::socket>
        > ws(ioc, ctx);

        // Resolve Binance
        auto const results =
            resolver.resolve(
                "stream.binance.com",
                "9443"
            );

        // Connect TCP
        net::connect(
            ws.next_layer().next_layer(),
            results.begin(),
            results.end()
        );

        // SSL Handshake
        ws.next_layer().handshake(
            ssl::stream_base::client
        );

        // WebSocket Handshake
        ws.handshake(
            "stream.binance.com",
            "/ws/btcusdt@depth"
        );

        std::cout
            << "Connected to Binance WebSocket\n";

        // Reusable Buffer
        beast::flat_buffer buffer;

        // Previous Mid Price
        double previous_mid = 0.0;

        // =====================================
        // MAIN LOOP
        // =====================================

        while(true)
        {
            // Start latency timer
            auto start =
                std::chrono::
                high_resolution_clock::now();

            // Reuse old buffer
            buffer.consume(buffer.size());

            // Read websocket data
            ws.read(buffer);

            // Convert buffer to string
            std::string data =
                beast::buffers_to_string(
                    buffer.data()
                );

            // Parse JSON ONCE
            json j = json::parse(data);

            // =====================================
            // STORE DIRECTLY INTO BINARY CACHE
            // =====================================

            cache.best_bid =
                std::stod(
                    std::string(
                        j["b"][0][0]
                    )
                );

            cache.bid_qty =
                std::stod(
                    std::string(
                        j["b"][0][1]
                    )
                );

            cache.best_ask =
                std::stod(
                    std::string(
                        j["a"][0][0]
                    )
                );

            cache.ask_qty =
                std::stod(
                    std::string(
                        j["a"][0][1]
                    )
                );

            // =====================================
            // SPREAD
            // =====================================

            cache.spread =
                cache.best_ask -
                cache.best_bid;

            // =====================================
            // IMBALANCE
            // =====================================

            cache.imbalance =
                (
                    cache.bid_qty -
                    cache.ask_qty
                )
                /
                (
                    cache.bid_qty +
                    cache.ask_qty
                );

            // =====================================
            // MID PRICE
            // =====================================

            double mid =
                (
                    cache.best_bid +
                    cache.best_ask
                ) / 2.0;

            // =====================================
            // VOLATILITY
            // =====================================

            cache.volatility =
                std::abs(
                    mid - previous_mid
                );

            previous_mid = mid;

            // =====================================
            // END LATENCY TIMER
            // =====================================

            auto end =
                std::chrono::
                high_resolution_clock::now();

            cache.processing_latency_us =
                std::chrono::duration_cast<
                    std::chrono::microseconds
                >(
                    end - start
                ).count();

            // =====================================
            // ACCESS RAW BINARY MEMORY
            // =====================================

            char* raw_memory =
                reinterpret_cast<char*>(
                    &cache
                );

            // Binary size
            size_t binary_size =
                sizeof(cache);

            // =====================================
            // LOW LATENCY OUTPUT
            // =====================================

            std::cout
                << "Bid: "
                << cache.best_bid

                << " | Ask: "
                << cache.best_ask

                << " | Spread: "
                << cache.spread

                << " | Imb: "
                << cache.imbalance

                << " | Vol: "
                << cache.volatility

                << " | ProcLatency(us): "
                << cache.processing_latency_us

                << " | BinarySize(bytes): "
                << binary_size

                << '\n';
        }
    }
    catch(std::exception const& e)
    {
        std::cerr
            << "Error: "
            << e.what()
            << '\n';
    }

    return 0;
}