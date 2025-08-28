#include <iostream>
#include<boost/beast/core.hpp>
#include<boost/beast/http.hpp>
#include<boost/asio.hpp>
#include<iostream>
#include<fstream>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;

namespace net = boost::asio;            // from <boost/asio.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
using tcp = net::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
																		
class Session : public enable_shared_from_this<Session> {
	
	tcp::socket socket_;
	beast::flat_buffer buffer_;
	http::request<http::string_body> req_;
	
	public:
	// Transfering the ownership of the socket to socket_
		explicit Session(tcp::socket socket) : socket_(move(socket)){};
		void start(){
			do_read();
		}
	private:

		void do_read()
		{
			// captureing shared pointer to the current object
			auto self = shared_from_this();
			http::async_read(socket_ , buffer_, req_ , [self](beast::error_code ec , size_t bytes_transferred){
				if(ec){
								
					if(ec != http::error::end_of_stream){
						cerr << "Error reading headers " << ec.message() << endl;
					}
// Gracefully close the socket ensuring that all data is sent and received
					self->socket_.shutdown(tcp::socket::shutdown_send , ec);
					return;
				}

				cout << "successfully read the request header" << self->req_.method_string() <<" " << self->req_.target() << endl;
				self->handle_request();
			});
		}


		void handle_request()
		{
				try{
					
					string target = req_.target().to_string();

					if(target == "/")
					{
						target = "/index.html";
					}

					if(target == "POST /upload")
					{
						handle_upload_request();
					}

					full_path = "./www" + target;

					handle_regular_request(full_path);

				}catch(const exception& e){

					cerr << "Exception in handle_request: " << e.what() << endl;
					send_bad_response(http::status::internal_server_error, "Internal Server Error");
				}
		}


		void handle_regular_request()
		{
			
		}

		void send_bad_response(http::status status , string_view error_message )
		{
			//  capturing the response as a shared pointer to extend its lifetime
			auto res = make_shared<http::response<http::string_body>>();

			res->version(req_.version());
			res->result(status);
			res->set(http::field::server , "Beast");
			res->set(http::field::content_type , "text/html");
			res->body() = string(error_message);
			res->prepare_payload();

			http::async_write(socket_ , *res , [self = shared_from_this() , res](beast:: error_code ec , size_t bytes_transferred){
				if(ec){
					cerr << "Error sending response: " << ec.message() << endl;
				}

				self->socket_.shutdown(tcp::socket::shutdown_send , ec);
			});

		}
};


int main()
{
	const auto address = net::ip::make_address("127.0.0.1");
	const unsigned short port = 8080;

	net::io_context ioc; // the task scheduler for asynchronous operations
	tcp::acceptor acceptor(ioc, {address, port});

	cout << "Server listening on " << address.to_string() << ":" << port << endl;
	std::function<void()> do_accept;
	do_accept = [&] {
		acceptor.async_accept([&]( const boost::system::error_code ec,tcp::socket socket) {
			if (!ec) {
				cout << "Client connected successfully" << endl;
	// Transfering the ownership of the socket to the sesion constructor
	// through a shared pointer that will manage the session object
				make_shared<Session>(move(socket))->start();
			}
			do_accept();
		});
	};


	do_accept();

	ioc.run();

	return 0 ;

}
