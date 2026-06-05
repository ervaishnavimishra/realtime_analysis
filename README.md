# realtime_analysis
A real-time market analysis engine for XAU/USD markets that processes Binance order book data and computes market microstructure features such as:

- Order Book Imbalance
- Spread
- Mid Price
- Microprice
- Rolling Volatility
- Liquidity Metrics

Built with Modern C++ for low-latency processing.

## C++ Dependencies

- Boost.Beast
- Boost.Asio
- OpenSSL
- nlohmann/json