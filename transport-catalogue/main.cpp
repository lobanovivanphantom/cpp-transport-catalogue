#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include <iostream>

int main() {
  json::Document doc(json::Load(std::cin));
  transport_directory::transport_catalogue::TransportCatalogue guide;
  transport_directory::json_reader::LoadTransportGuide(doc, guide);
  transport_directory::json_reader::PrintAnswearsForRequests(doc, guide);
  return 0;
}