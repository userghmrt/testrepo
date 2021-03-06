// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef include_haship_hpp
#define include_haship_hpp 1


#include "strings_utils.hpp"
#include "libs1.hpp"

#include "formats_ip.hpp"

#include "crypto/crypto.hpp"

// https://tools.ietf.org/html/rfc2460
// https://tools.ietf.org/html/rfc4291
// also https://en.wikipedia.org/wiki/IPv6_packet#Fixed_header
namespace g_ipv6_rfc {
	constexpr unsigned char length_of_addr = 128/8 ;  // length of the address (not just header, but in any bin representation)

	constexpr unsigned char header_position_of_src = 8 ;  // rfc2460#section-3
	constexpr unsigned char header_length_of_src = 128/8 ;  // length of this field

	constexpr unsigned char header_position_of_dst = 24 ;  // rfc2460#section-3
	constexpr unsigned char header_length_of_dst = 128/8 ;  // length of this field
	constexpr unsigned char header_position_of_ip_protocol_type = 6 ;
}
// use: g_ipv6_rfc::header_position_of_src

namespace g_tuntap {
	constexpr unsigned char header_position_of_ipv6 = 4;
}
// use: g_tuntap::header_position_of_ipv6

// declare sizes; also forward declarations
constexpr int g_haship_addr_size = 16;
constexpr int g_haship_pubkey_size = 32;
struct c_haship_addr;
struct c_haship_pubkey;

/**
 * @class virtual hash-ip, e.g. ipv6, usable for ipv6-cjdns (fc00/8), and of course also for our ipv6-galaxy (fd42/16)
*/
struct c_haship_addr : public std::array<unsigned char, g_haship_addr_size> {
	struct tag_constr_by_hash_of_pubkey{}; // address is calculated from hash of given public key
	struct tag_constr_by_addr_dot{}; // address is in form of t_ipv6dot
	struct tag_constr_by_addr_bin{}; // address is in form of t_ipv6bin
	struct tag_constr_by_array_uchar{}; // address is in form of std::array of unsigned char of proper size

	c_haship_addr(); ///< address is zero filled

	/// create the IP address that matches given public key (e.g. hash of it)
	c_haship_addr(tag_constr_by_hash_of_pubkey x, const c_haship_pubkey & pubkey );
	/// create the IP address from a string (as dot/colon IP notation)
	c_haship_addr(tag_constr_by_addr_dot x, const t_ipv6dot & addr_string);
	/// create the IP address from binary serialization of the IP address
	c_haship_addr(tag_constr_by_addr_bin x, const t_ipv6bin & data );
	/// create the IP address from std::array of unsigned char
	c_haship_addr(tag_constr_by_array_uchar x, const std::array<unsigned char, g_haship_addr_size> & data);

	void print(ostream &ostr) const;
	std::string get_hip_as_string(bool with_dots) const;
	bool is_empty() const;
	static c_haship_addr make_empty(); ///< named constructor
};
ostream& operator<<(ostream &ostr, const c_haship_addr & v);

struct c_haship_pubkey : antinet_crypto::c_multikeys_pub {
	c_haship_pubkey();
	c_haship_pubkey( const string_as_bin & input ); ///< create the IP from a string with serialization of the key

	void print(ostream &ostr) const;
};
ostream& operator<<(ostream &ostr, const c_haship_pubkey & v);


namespace unittest {

	void ipv6conversions();

} // namespace


bool addr_is_galaxy(c_haship_addr addr);

#endif

