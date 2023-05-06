#include "transport_catalogue.h"
#include <iomanip>

namespace transport_directory {
namespace transport_catalogue {

StatBusRoute::StatBusRoute(std::string name, size_t count_stops,
                           size_t count_unique_stops, int route_length,
                           double curvature, const domain::BusRoute *bus_route)
    : name_(std::move(name)), count_stops_(count_stops),
      count_unique_stops_(count_unique_stops), route_length_(route_length),
      curvature_(curvature), bus_route_(bus_route) {}

StatForStop::StatForStop(std::string_view name_stop,
                         const std::set<std::string_view> *buses)
    : name_stop_(name_stop), buses_(buses) {}

void TransportCatalogue::AddStop(std::string name_stop,
                                 geo::Coordinates coordinates) {
  auto &stop = stops_.emplace_back(std::move(name_stop), coordinates);
  index_stops_.emplace(stop.name, &stop);
  stop_buses_[stop.name];
}

void TransportCatalogue::AddBusRoute(std::string name_bus,
                                     std::vector<std::string> stops,
                                     bool is_roundtrip) {
  std::vector<domain::Stop *> route(stops.size());
  auto &bus = bus_routes_.emplace_back(std::move(name_bus), std::move(route));
  for (size_t i = 0; i < bus.route.size(); ++i) {
    bus.route[i] = index_stops_[stops[i]];
    stop_buses_[stops[i]].insert(bus.name);
  }
  if (is_roundtrip) {
    bus.is_roundtrip = is_roundtrip;
  }
  index_buses_.emplace(bus.name, &bus);
}

void TransportCatalogue::SetDistance(const domain::Stop *from,
                                     const domain::Stop *to, int distance) {
  distances_.emplace(
      std::pair<const domain::Stop *, const domain::Stop *>{from, to},
      distance);
}

int TransportCatalogue::GetDistance(const domain::Stop *from,
                                    const domain::Stop *to) const {
  auto it = distances_.find({from, to});
  if (it == distances_.end()) {
    return distances_.at({to, from});
  }
  return it->second;
}

const domain::BusRoute *
TransportCatalogue::SearchRoute(std::string_view name_bus) const {
  auto it = index_buses_.find(name_bus);
  if (it == index_buses_.end()) {
    return nullptr;
  }
  return it->second;
}

const domain::Stop *
TransportCatalogue::SearchStop(std::string_view name_stop) const {
  auto it = index_stops_.find(name_stop);
  if (it == index_stops_.end()) {
    return nullptr;
  }
  return it->second;
}

const std::deque<domain::BusRoute> &TransportCatalogue::GetBusRoutes() const {
  return bus_routes_;
}

size_t TransportCatalogue::DistancesHasher::operator()(
    const std::pair<const domain::Stop *, const domain::Stop *> &p) const {
  return hasher((uintptr_t)p.first) + 47 * hasher((uintptr_t)p.second);
}

StatBusRoute
TransportCatalogue::RequestStatBusRoute(std::string_view name_bus) const {
  auto bus_route_ptr = SearchRoute(name_bus);
  if (!bus_route_ptr) {
    return StatBusRoute(std::string(name_bus));
  }
  const domain::BusRoute &bus_route = *bus_route_ptr;
  size_t count_stops = bus_route.route.size();
  std::unordered_set<domain::Stop *> unique_stops;
  unique_stops.insert(bus_route.route[0]);
  double distance = 0.0;
  int route_length = 0;
  for (size_t i = 1; i < bus_route.route.size(); ++i) {
    unique_stops.insert(bus_route.route[i]);
    distance += ComputeDistance(bus_route.route[i - 1]->coordinates,
                                bus_route.route[i]->coordinates);
    route_length += GetDistance(bus_route.route[i - 1], bus_route.route[i]);
  }
  size_t count_unique_stops = unique_stops.size();
  double curvature = static_cast<double>(route_length) / distance;
  return StatBusRoute(std::string(name_bus), count_stops, count_unique_stops,
                      route_length, curvature, bus_route_ptr);
}

StatForStop
TransportCatalogue::RequestStatForStop(std::string_view name_stop) const {
  if (!SearchStop(name_stop)) {
    return StatForStop(name_stop, nullptr);
  }
  auto it = stop_buses_.find(name_stop);
  return StatForStop(name_stop, &(it->second));
}

} // end namespace transport_catalogue
} // end namespace transport_directory