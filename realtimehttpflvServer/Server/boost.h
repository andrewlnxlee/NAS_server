#ifndef BOOST_H
#define BOOST_H

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
//#include <boost/beast/version.hpp>
//#include <boost/asio.hpp>
// #include <boost/asio/steady_timer.hpp> 
#include <boost/asio.hpp>//add by lhx
#include <boost/bind/bind.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

#endif //BOOST_H