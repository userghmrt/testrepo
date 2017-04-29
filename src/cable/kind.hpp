
#pragma once

#include <ostream>

enum class t_cable_kind {
	kind_simul=1,
	kind_shm=2,
	kind_udp=3,
};

std::string to_string(t_cable_kind obj);
std::ostream & operator<<(std::ostream & ostr, t_cable_kind obj);


