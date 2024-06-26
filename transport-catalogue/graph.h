#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight> struct Edge {
  VertexId from;
  VertexId to;
  Weight weight;
};

template <typename Weight> class DirectedWeightedGraph {
private:
  using IncidenceList = std::vector<EdgeId>;
  using IncidentEdgesRange =
      ranges::Range<typename IncidenceList::const_iterator>;

public:
  DirectedWeightedGraph() = default;
  explicit DirectedWeightedGraph(size_t vertex_count);
  EdgeId add_edge(const Edge<Weight> &edge);

  size_t get_vertex_count() const;
  size_t get_edge_count() const;
  const Edge<Weight> &get_edge(EdgeId edge_id) const;
  IncidentEdgesRange get_incident_edges(VertexId vertex) const;

private:
  std::vector<Edge<Weight>> edges_;
  std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::add_edge(const Edge<Weight> &edge) {
  edges_.push_back(edge);

  const EdgeId id = edges_.size() - 1;

  incidence_lists_.at(edge.from).push_back(id);

  return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::get_vertex_count() const {
  return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::get_edge_count() const {
  return edges_.size();
}

template <typename Weight>
const Edge<Weight> &
DirectedWeightedGraph<Weight>::get_edge(EdgeId edge_id) const {
  return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::get_incident_edges(VertexId vertex) const {
  return ranges::as_range(incidence_lists_.at(vertex));
}

} // end namespace graph