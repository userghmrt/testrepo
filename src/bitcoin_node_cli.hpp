#ifndef BITCOIN_NODE_CLI_HPP
#define BITCOIN_NODE_CLI_HPP

#include <libs0.hpp>
#include <boost/asio.hpp>
#include <boost/asio/use_future.hpp>
#include <mutex>
#include <cppcodec/base64_default_rfc4648.hpp>
#include <future>

class i_http_json_rpc_provider {
	public:
		/**
		 * @brief send_post_request
		 * @param endpoint bitcoin node endpoint (ip + port)
		 * @param json_request json data for send
		 * @param username bitcoin rpc username
		 * @param password bitcoin rpc password
		 * @param timeout
		 * @return resmonse from bitcoin node
		 */
		virtual std::string send_post_request(
		                              const boost::asio::ip::tcp::endpoint &endpoint,
		                              const std::string &json_request,
		                              const std::string &username,
		                              const std::string &password,
		                              const std::chrono::seconds &timeout) = 0;
		virtual ~i_http_json_rpc_provider() = default;
};

template <class TSOCKET = boost::asio::ip::tcp::socket>
class http_json_rpc : public i_http_json_rpc_provider {
	public:
		/**
		 * @brief http_json_rpc this c-tor will be needed for unit tests
		 * @param m_io_service io_servide used to create socket
		 * @param socket not opened socket
		 */
		http_json_rpc(
		              std::shared_ptr<boost::asio::io_service> &m_io_service,
		              std::unique_ptr<TSOCKET> &&socket
		              );
		http_json_rpc();
		std::string send_post_request(
		                              const boost::asio::ip::tcp::endpoint &endpoint,
		                              const std::string &json_request,
		                              const std::string &username,
		                              const std::string &password,
		                              const std::chrono::seconds &timeout
		) override;
	private:
		std::shared_ptr<boost::asio::io_service> m_io_service;
		std::unique_ptr<TSOCKET> m_socket;
		std::string generate_post_data(
		                               const std::string &json,
		                               const boost::asio::ip::tcp::endpoint &endpoint,
		                               const std::string &user,
				const std::string &pass);
};

template<class TSOCKET>
http_json_rpc<TSOCKET>::http_json_rpc()
	:
	m_io_service(std::make_shared<boost::asio::io_service>()),
	m_socket(std::make_unique<TSOCKET>(*m_io_service))
{
}

template<class TSOCKET>
std::string http_json_rpc<TSOCKET>::send_post_request(
                          const boost::asio::ip::tcp::endpoint &endpoint,
                          const std::string &json_request,
                          const std::string &username,
                          const std::string &password,
                          const std::chrono::seconds &timeout) {
	auto timeout_point = std::chrono::steady_clock::now() + timeout;
	boost::asio::io_service::work idle_work(*m_io_service);
	std::thread run_thread([this]{m_io_service->run();});
	
	// connect
	std::future<void> connect_future = m_socket->async_connect(endpoint, boost::asio::use_future);
	std::future_status status;
	status = connect_future.wait_until(timeout_point);
	if (status != std::future_status::ready) throw std::runtime_error("send request timeout (connect)");
	
	// write request
	const std::string post_data = generate_post_data(json_request, endpoint, username, password);
	std::future<size_t> write_future = boost::asio::async_write(
						*m_socket,
						boost::asio::buffer(post_data.data(), post_data.size()),
						boost::asio::use_future);
	status = write_future.wait_until(timeout_point);
	if (status != std::future_status::ready) throw std::runtime_error("send request timeout (write request)");
	const size_t write_bytes = write_future.get();
	pfp_dbg1("write " << write_bytes << " to bitcoin rpc");
	
	// read response
	const size_t max_response_size = 1000;
	std::string response('\0', max_response_size);
	std::future<size_t> read_future = boost::asio::async_read(
					*m_socket,
					boost::asio::buffer(&response[0], response.size()),
					boost::asio::use_future
				);
	status = read_future.wait_until(timeout_point);
	if (status != std::future_status::ready) throw std::runtime_error("send request timeout (read response)");
	const size_t readed_bytes = read_future.get();
	pfp_dbg1("readed " << readed_bytes << "from bitcoin rpc");
	while (response.back() == '\0') {
		response.pop_back();
	}
	m_io_service->stop(); // stop idle work
	run_thread.join();
	return response;
}

template<class TSOCKET>
std::string http_json_rpc<TSOCKET>::generate_post_data(const std::string &json, const boost::asio::ip::tcp::endpoint &endpoint, const std::string &user, const std::string &pass) {
	const std::string ip = endpoint.address().to_string();
	const unsigned short port = endpoint.port();
	std::string post_data;
	post_data += "POST / HTTP/1.1 \r\n";
	post_data += "Host: " + ip + ":" + std::to_string(port) + "\r\n";
	post_data += "Authorization: Basic " + base64::encode(user + ":" + pass) + "\r\n";
	post_data += "User-Agent: ConnHttpJsonAuth\r\n";
	post_data += "Accept: */*\r\n";
	post_data += "Accept-Encoding: identity\r\n"; // rfc7231 Section 5.3.4 no compression
	post_data += "Content-Type: application/json\r\n";
	post_data += "Content-Length: " + std::to_string(json.size()) + "\r\n";
	post_data += "\r\n"; // new line
	post_data += json;
	return post_data;
}
//////////////////////////////////////////////////////

class bitcoin_node_cli final {
	public:
		static bitcoin_node_cli& get_instance();

		bitcoin_node_cli(const bitcoin_node_cli& src) = delete;

		/**
		 * @brief get_balance this funstion is thread safe
		 * @return amount of BTC (in satoshi)
		 */
		uint32_t get_balance() const;

		/**
		 * @brief get_new_address
		 * @return new bitcoin address
		 */
		std::string get_new_address() const;

	private:
		static std::unique_ptr<bitcoin_node_cli> m_instance;
		static std::once_flag m_once_flag;
		mutable std::mutex m_mutex;
		const std::string m_rpc_http_address; // i.e. http://127.0.0.1:18332 for localhost testnet
		const std::string m_ip;
		const unsigned short m_port;
		std::unique_ptr<i_http_json_rpc_provider> m_http_json_rpc;

		std::string send_request_and_get_response(const std::string &request) const;

		bitcoin_node_cli(const std::string &ip_address="127.0.0.1", unsigned short port=18332);
};

#endif // BITCOIN_NODE_CLI_HPP
