#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include "geo.h"

namespace transport_catalogue {


    struct Stop {
        std::string_view name;
        earth_surface::Coordinates coordinates;
    };


    struct Bus {
        std::string_view name;
        std::vector<const Stop*> stops;
    };


    struct BusInfo {
        int stop_number;
        int unique_stop_number;
        int interval;
        double curvature;

        BusInfo();
    };

    struct HasherPair {
        size_t operator()(const std::pair<const Stop*, const Stop*>& p) const;
    };

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;
        void addStop(std::string_view name, earth_surface::Coordinates& coord);
        void addBus(std::string_view name, const std::vector<std::string>& stops);
        void addBusInfo(const Bus* bus, const BusInfo& info);
        const Stop* findStop(std::string_view name) const;
        const Bus* findBus(std::string_view name) const;
        const BusInfo getBusInfo(const Bus* bus);
        const BusInfo getBusInfo(const std::string_view name);
        int getBusesNumOnStop(const Stop* stop);
        std::vector<const Bus*> getBusesOnStop(const Stop* stop);
        void setDistance(const std::string& stop_from, const std::string& stop_to, int interval);
        int getDistance(const Stop* stop_from, const Stop* stop_to) const;

    private:
        std::deque<std::string> m_name_to_storage;
        std::deque<Stop> m_stops;
        std::unordered_map<std::string_view, const Stop*> m_name_to_stop;
        std::deque<Bus> m_buses;
        std::unordered_map<std::string_view, const Bus*> m_name_to_bus;
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, HasherPair> m_distance;
        std::unordered_map<const Stop*, std::unordered_set<Bus*>> m_stop_to_bus;
        std::unordered_map<const Bus*, BusInfo> m_bus_to_info;
        std::string_view getName(std::string_view str);
    };

}