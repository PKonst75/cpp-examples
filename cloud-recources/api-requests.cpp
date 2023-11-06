#include<boost/beast.hpp>
#include<boost/asio.hpp>
#include<boost/asio/connect.hpp>
#include<boost/asio/ip/tcp.hpp>
#include<boost/asio/ssl.hpp>
#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/json_parser.hpp>
#include<string>
#include<iostream>

using namespace std::literals;
namespace http = boost::beast::http;

const std::string MAIN_CITE = "mts-olimp-cloud.codenrock.com"s;
const std::string TOKEN = "?token=5uEucQKvpONPv1ehjTh99E14hTs8s3dw"s;
const std::string REQUEST_RESOURCE = "/api/resource"s;
const std::string REQUEST_STATISTIC = "/api/statistic"s;
const std::string REQUEST_PRICE = "/api/price"s;

std::string GetResponseSSL(const std::string& request_type, const std::string& token = ""s) {
	const std::string host = MAIN_CITE;
	const std::string path = request_type + token;
	const std::string port = "443";

	boost::asio::io_service svc;

	boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23_client);
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssocket = { svc, ctx };
	boost::asio::ip::tcp::resolver resolver(svc);
	auto it = resolver.resolve(host, port);
	connect(ssocket.lowest_layer(), it);
	ssocket.handshake(boost::asio::ssl::stream_base::handshake_type::client);
	http::request<http::string_body> req{ http::verb::get, path, 11 };
	req.set(http::field::host, host);

	http::write(ssocket, req);

	http::response<http::string_body> res;
	boost::beast::flat_buffer buffer;
	http::read(ssocket, buffer, res);

	if (res.base().result() != boost::beast::http::status::ok) {
		return "";
	}

	return res.body().data();
}

std::string PostResponseSSL(const std::string& request_type, const std::string& token = ""s, const std::string& data = ""s) {
	const std::string host = MAIN_CITE;
	const std::string path = request_type + token;
	const std::string port = "443";

	boost::asio::io_service svc;
	boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23_client);
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssocket = { svc, ctx };

	boost::asio::ip::tcp::resolver resolver(svc);
	auto it = resolver.resolve(host, port);
	connect(ssocket.lowest_layer(), it);
	ssocket.handshake(boost::asio::ssl::stream_base::handshake_type::client);


	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << "POST " << path << " HTTP/1.0\r\n";
	request_stream << "Host: " << host << ":" << port << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Content-Length: " << data.length() << "\r\n";
	request_stream << "Content-Type: application/json\r\n";
	request_stream << "Connection: close\r\n\r\n";
	request_stream << data;

	// Send the request.
	boost::asio::write(ssocket, request);

	http::response<http::string_body> res;
	boost::beast::flat_buffer buffer;
	http::read(ssocket, buffer, res);

	if (res.base().result() != boost::beast::http::status::ok) {
		return "";
	}

	return res.body().data();
}

std::string DeleteResponseSSL(const std::string& request_type, const std::string& token = ""s, int id = 0) {
	const std::string host = MAIN_CITE;
	const std::string path = request_type + "/" + std::to_string(id) + token;
	const std::string port = "443";

	boost::asio::io_service svc;
	boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23_client);
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssocket = { svc, ctx };

	boost::asio::ip::tcp::resolver resolver(svc);
	auto it = resolver.resolve(host, port);
	connect(ssocket.lowest_layer(), it);
	ssocket.handshake(boost::asio::ssl::stream_base::handshake_type::client);


	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << "DELETE " << path << " HTTP/1.0\r\n";
	request_stream << "Host: " << host << ":" << port << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	// Send the request.
	boost::asio::write(ssocket, request);

	http::response<http::string_body> res;
	boost::beast::flat_buffer buffer;
	http::read(ssocket, buffer, res);

	if (res.base().result() != boost::beast::http::status::ok) {
		return "";
	}

	return res.body().data();
}


std::string PostResponseSSLCreateNewVm(const std::string& json_data) {
	return  PostResponseSSL(REQUEST_RESOURCE, TOKEN, json_data);
}

std::string GetStatistic() {
	return GetResponseSSL(REQUEST_STATISTIC, TOKEN);
}

std::string GetResources() {
	return GetResponseSSL(REQUEST_RESOURCE, TOKEN);
}

std::string DeleteResponseSSL(int id) {
	return  DeleteResponseSSL(REQUEST_RESOURCE, TOKEN, id);
}

std::string GetPrice() {
	return GetResponseSSL(REQUEST_PRICE);
}

