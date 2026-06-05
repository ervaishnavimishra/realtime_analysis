#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

#include <iostream>

namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace beast = boost::beast;
namespace websocket = beast::websocket;

using tcp = asio::ip::tcp;

void startWebSocket()
{
    try
    {
        asio::io_context ioc;

        ssl::context ctx(ssl::context::tls_client);

        websocket::stream<
            beast::ssl_stream<tcp::socket>
        > ws(ioc, ctx);

        tcp::resolver resolver(ioc);

        auto const results =
            resolver.resolve(
                "stream.binance.com",
                "9443"
            );

        asio::connect(
            beast::get_lowest_layer(ws),
            results
        );

        ws.next_layer().handshake(
            ssl::stream_base::client
        );

        ws.handshake(
            "stream.binance.com",
            "/ws/btcusdt@depth20@100ms"
        );

        std::cout
            << "Connected to Binance"
            << std::endl;

        for (;;)
        {
            beast::flat_buffer buffer;

            ws.read(buffer);

            std::cout
                << beast::make_printable(
                       buffer.data()
                   )
                << std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr
            << e.what()
            << std::endl;
    }
}