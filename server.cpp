#include <iostream>
#include<boost/beast/core.hpp>
#include<boost/beast/http.hpp>
#include<boost/asio.hpp>
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;

namespace net = boost::asio;            // from <boost/asio.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
using tcp = net::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
																		
string mime_type(string path);
string read_file(const string& path);

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
					
					string target = string(req_.target());

					if(target == "/")
					{
						cout << "Entered the home route" << endl;
						target = "/index.html";
					}

					if(target == "POST /upload")
					{
						cout << "Coming soon" << endl;
						//handle_upload_request();
					}

					string full_path = "./www" + target;

					handle_regular_request(full_path);

				}catch(const exception& e){

					cerr << "Exception in handle_request: " << e.what() << endl;
					send_bad_response(http::status::internal_server_error, "Internal Server Error");
				}
		}


		void handle_regular_request(string full_path)
		{
			try{

				if (req_.find(http::field::range) != req_.end()) {
		    string range_header = string(req_[http::field::range]);
    
    if (range_header.compare(0,6 , "bytes=") == 0) {
        string range_spec = range_header.substr(6);
        size_t dash_pos = range_spec.find('-');
        
        if (dash_pos != string::npos) {
            string start_str = range_spec.substr(0, dash_pos);
            string end_str = range_spec.substr(dash_pos + 1);
            
            long long start_byte = -1;
            long long end_byte = -1;
            
            if (!start_str.empty()) {
                start_byte = stoll(start_str);
            }

            if (!end_str.empty()) {    
                end_byte = stoll(end_str);
            }
            
            cout << "Parsed range: start=" << start_byte << ", end=" << end_byte << endl;
            
            handle_range_request(full_path, start_byte, end_byte);
            return; 
        }
    }
}

				auto res = make_shared<http::response<http::string_body>>();				
				auto body = read_file(full_path);

				res->version(req_.version());
				res->result(http::status::ok);
				res->set(http::field::server, "Beast");
				res->set(http::field::content_type, mime_type(full_path));
				res->body() = move(body);
				res->prepare_payload();

				async_write(socket_ , *res , [self = shared_from_this() , res](beast:: error_code ec , size_t bytes_transferred){
						
					if(ec)
					{
						cerr << "Error sending response: " << ec.message() << endl;
					}

					self->socket_.shutdown(tcp::socket::shutdown_send , ec);
				});

			}catch(...){
				send_bad_response(http::status::not_found, "File Not Found");
				return;
			}
			
		}
		
		void handle_range_request(string path , long long start , long long end)
		{
			ifstream file(path , ios::binary | ios::ate);
			if(!file)
			{
				send_bad_response(http::status::not_found, "File Not Found");
				return;
			}

			streamsize file_size = file.tellg();
			file.seekg(0 , ios::beg);

			 if (start < 0 && end >= 0) {
        // Handle suffix range: bytes=-500 (last 500 bytes)
        start = max(0LL, file_size - end);
        end = file_size - 1;
			 } 
			 else if (start >= 0 && end < 0) {
        // Handle range from start to end: bytes=1000-
        end = file_size - 1;
			 } 
			 else if (start < 0 && end < 0) {
        // Invalid case
        send_bad_response(http::status::bad_request, "Invalid range format");
        return;
			}


			if(start >= file_size || end >= file_size || start > end)
			{
				send_bad_response(http::status::range_not_satisfiable, "Requested Range Not Satisfiable");
				return;
			}

			streamsize chunk_size = end - start + 1;
			
			file.seekg(start, ios::beg);
			vector<char> buffer(chunk_size);
			file.read(buffer.data(), chunk_size);
    
    // Check if read was successful
			if (file.gcount() != chunk_size) {
        send_bad_response(http::status::internal_server_error, 
                         "Error reading file chunk");
        return;
			}
    
    // Create 206 Partial Content response
    auto res = make_shared<http::response<http::string_body>>();
    res->version(req_.version());
    res->result(http::status::partial_content);
    res->set(http::field::server, "Beast");
    res->set(http::field::content_type, mime_type(path));
    res->set(http::field::accept_ranges, "bytes");
    res->set(http::field::content_range, 
             "bytes " + to_string(start) + "-" + 
             to_string(end) + "/" + to_string(file_size));
    res->body() = string(buffer.data(), chunk_size);
    res->prepare_payload();
    
    // Send the response
    http::async_write(socket_, *res, 
        [self = shared_from_this(), res](beast::error_code ec, size_t bytes_transferred) {
            if (!ec) {
                // Optionally keep connection alive for more requests
                self->do_read();
            } else {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            }
        });

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


string mime_type(string path)
{
	static const unordered_map<string, string> mime_map = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".gif", "image/gif"},
        {".txt", "text/plain"},
        {".mp4", "video/mp4"},
				{".m3u8" , "application/vnd.apple.mpegurl"},
				{".ts","video/mp2t"},
    };

		auto dot_position = path.find_last_of('.');
		if(dot_position == string::npos)
		{
			return "application/octet-stream";
		}
		string extension = path.substr(dot_position);
		auto it = mime_map.find(extension);
		if(it != mime_map.end())
		{
			return it->second;
		}
		return "application/octet-stream";
}


string read_file(const string& path)
{
	ifstream file(path , ios::binary);
	if(!file)
	{
		throw runtime_error("File not found");
	}
	 return string(
        (istreambuf_iterator<char>(file)), 
        istreambuf_iterator<char>()          
    );
}

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
