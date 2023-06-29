#pragma once
#include "json.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport_catalogue {
namespace detail {
namespace json {

class JSONReader {
public:
  JSONReader() = default;
  JSONReader(Document doc);
  JSONReader(std::istream &input);

  void parse_node_base(const Node &root, TransportCatalogue &catalogue);
  void parse_node_stat(const Node &root,
                       std::vector<StatRequest> &stat_request);
  void parse_node_render(const Node &node,
                         map_renderer::RenderSettings &render_settings);
  void parse_node_routing(const Node &node, router::RoutingSettings &route_set);
  void parse_node_serialization(
      const Node &node,
      serialization::SerializationSettings &serialization_set);

  void parse_node_make_base(
      TransportCatalogue &catalogue,
      map_renderer::RenderSettings &render_settings,
      router::RoutingSettings &routing_settings,
      serialization::SerializationSettings &serialization_settings);

  void parse_node_process_requests(
      std::vector<StatRequest> &stat_request,
      serialization::SerializationSettings &serialization_settings);

  Stop parse_node_stop(Node &node);
  Bus parse_node_bus(Node &node, TransportCatalogue &catalogue);
  std::vector<Distance> parse_node_distances(Node &node,
                                             TransportCatalogue &catalogue);

  const Document &get_document() const;

private:
  Document document_;
};

} // end namespace json
} // end namespace detail
} // end namespace transport_catalogue