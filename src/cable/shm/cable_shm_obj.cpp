#include "cable_shm_obj.hpp"

c_cable_shm_obj::c_cable_shm_obj()
{

}

void c_cable_shm_obj::send_to(const c_cable_base_addr &dest, const unsigned char *data, size_t size) {
	pfp_UNUSED(dest);
	pfp_UNUSED(data);
	pfp_UNUSED(size);
}

