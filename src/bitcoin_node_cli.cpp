#include "bitcoin_node_cli.hpp"
#include <json.hpp>


//this parametrs are static
std::unique_ptr<bitcoin_node_cli> bitcoin_node_cli::m_instance;
std::once_flag bitcoin_node_cli::m_once_flag;

bitcoin_node_cli& bitcoin_node_cli::get_instance()
{
	std::call_once(m_once_flag, []{
		m_instance.reset(new bitcoin_node_cli);
	});

	return *m_instance.get();
}

bitcoin_node_cli::bitcoin_node_cli(const std::string &ip_address, unsigned short port)
:
	m_rpc_http_address("http://" + ip_address + ':' + std::to_string(port)),
	m_ip(ip_address),
	m_port(port),
	m_http_json_rpc(std::make_unique<http_json_rpc<>>()) // TODO make factory (unit tests)
{
}

uint32_t bitcoin_node_cli::get_balance() const {
	std::lock_guard<std::mutex> lock(m_mutex);

	const std::string request (R"({"method":"getbalance","params":["*",0],"id":1})");
	const std::string receive_data = send_request_and_get_response(request);
	pfp_mark("Receive data " << receive_data);

	nlohmann::json json = nlohmann::json::parse(receive_data.c_str());
	double btc_amount = json.at("result").get<double>();
	_check(btc_amount<=21000000);
	_check(btc_amount>=0);
	return btc_amount * 100'000'000. ; // return balance in satoshi
}

std::string bitcoin_node_cli::get_new_address() const {
	std::lock_guard<std::mutex> lock(m_mutex);

	const std::string request = R"({"method":"getnewaddress","params":[],"id":1})";
	std::string receive_data = send_request_and_get_response(request);
	pfp_mark("Receive data " << receive_data);

	nlohmann::json json = nlohmann::json::parse(receive_data);
	return json.at("result").get<std::string>();
}

std::string bitcoin_node_cli::send_request_and_get_response(const std::string &request) const {
	const boost::asio::ip::address ip = boost::asio::ip::address::from_string(m_ip);
	const boost::asio::ip::tcp::endpoint endpoint(ip, m_port);
	// TODO read user and pass from conf
	return m_http_json_rpc->send_post_request(endpoint, request, "bitcoinrpcUSERNAME", "bitcoinrpcPASSWORD", std::chrono::seconds(5));
}
