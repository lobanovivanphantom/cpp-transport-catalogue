#pragma once
 
#include <algorithm>
#include <vector>
#include <string>
 
#include "geo.h"
 
namespace domain {
 
struct StatRequest { 
    int id;
    std::string type;
    std::string name;    
};
    
struct Bus;
    
struct Stop {    
    std::string name;
    double latitude;
    double longitude;
    
    std::vector<Bus*> buses;
};
 
struct Bus { 
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
    size_t route_length;
};
 
struct Distance {    
    const Stop* start;
    const Stop* end;
    int distance;
};  
 
struct BusQueryResult {
    std::string_view name;
    bool not_found;
    int stops_on_route;
    int unique_stops;
    int route_length;
    double curvature;
};    
   
struct StopQueryResult {
    std::string_view name;
    bool not_found;
    std::vector <std::string> buses_name;
};

struct StopEdge {
    std::string_view name;
    double time = 0;
};
 
struct BusEdge {
    std::string_view bus_name;
    size_t span_count = 0;
    double time = 0;
};

struct RoutingSettings {
    double bus_wait_time = 0;
    double bus_velocity = 0;
};

 
}//end namespace domain