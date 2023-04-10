#pragma once

#include <sstream>

#include "transport_catalogue.h"

namespace query_input {

	std::istream& queryDataBaseUpdate(transport_catalogue::TransportCatalogue& catalogue, std::istream& input);

}
