#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace renderer {

struct RenderSettings {

  using OffSet = svg::Point;

  double width_ = 1.0;
  double height_ = 1.0;
  double padding_ = 1.0;
  double line_width_ = 1.0;
  double stop_radius_ = 1.0;
  int bus_label_font_size_ = 1;
  OffSet bus_label_offset_;
  int stop_label_font_size_ = 1;
  OffSet stop_label_offset_;
  svg::Color underlayer_color_;
  double underlayer_width_ = 1.0;
  std::vector<svg::Color> color_palette_;
};

class RenderRoute : public svg::Drawable {
public:
  RenderRoute(const std::vector<svg::Point> &stops, svg::Color stroke_color,
              double stroke_width);

  void Draw(svg::ObjectContainer &container) const override;

private:
  svg::Polyline route_;
};

class RenderNameOfRoutes : public svg::Drawable {
public:
  RenderNameOfRoutes(const svg::Point &pos, const svg::Point &offset,
                     int font_size, const std::string &name_bus,
                     svg::Color underlayer_color, double underlayer_width,
                     svg::Color text_color);

  void Draw(svg::ObjectContainer &container) const override;

private:
  svg::Text underlayer_;
  svg::Text text_;
};

class RenderCirclForStop : public svg::Drawable {
public:
  RenderCirclForStop(const svg::Point &center, double radius);

  void Draw(svg::ObjectContainer &container) const override;

private:
  svg::Circle circle_;
};

class RenderNameOfStops : public svg::Drawable {
public:
  RenderNameOfStops(const svg::Point &pos, const svg::Point &offset,
                    int font_size, const std::string &name_stop,
                    svg::Color underlayer_color, double underlayer_width);

  void Draw(svg::ObjectContainer &container) const override;

private:
  svg::Text underlayer_;
  svg::Text text_;
};

inline const double EPSILON = 1e-6;

class SphereProjector {
public:
  // points_begin è points_end çàäàþò íà÷àëî è êîíåö èíòåðâàëà ýëåìåíòîâ
  // geo::Coordinates
  template <typename PointInputIt>
  SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                  double max_width, double max_height, double padding);

  // Ïðîåöèðóåò øèðîòó è äîëãîòó â êîîðäèíàòû âíóòðè SVG-èçîáðàæåíèÿ
  svg::Point operator()(const geo::Coordinates &coords) const;

  static bool IsZero(double value) { return std::abs(value) < EPSILON; }

private:
  double padding_;
  double min_lon_ = 0;
  double max_lat_ = 0;
  double zoom_coeff_ = 0;
};

class MapRenderer {
public:
  MapRenderer(const RenderSettings &settings);

  std::vector<std::unique_ptr<svg::Drawable>> CreateArrayObjects(
      std::vector<const transport_directory::domain::BusRoute *> bus_routes)
      const;

private:
  RenderSettings settings_;
};

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin,
                                 PointInputIt points_end, double max_width,
                                 double max_height, double padding)
    : padding_(padding) {
  // Åñëè òî÷êè ïîâåðõíîñòè ñôåðû íå çàäàíû, âû÷èñëÿòü íå÷åãî
  if (points_begin == points_end) {
    return;
  }

  // Íàõîäèì òî÷êè ñ ìèíèìàëüíîé è ìàêñèìàëüíîé äîëãîòîé
  const auto [left_it, right_it] =
      std::minmax_element(points_begin, points_end,
                          [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
  min_lon_ = left_it->lng;
  const double max_lon = right_it->lng;

  // Íàõîäèì òî÷êè ñ ìèíèìàëüíîé è ìàêñèìàëüíîé øèðîòîé
  const auto [bottom_it, top_it] =
      std::minmax_element(points_begin, points_end,
                          [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
  const double min_lat = bottom_it->lat;
  max_lat_ = top_it->lat;

  // Âû÷èñëÿåì êîýôôèöèåíò ìàñøòàáèðîâàíèÿ âäîëü êîîðäèíàòû x
  std::optional<double> width_zoom;
  if (!IsZero(max_lon - min_lon_)) {
    width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
  }

  // Âû÷èñëÿåì êîýôôèöèåíò ìàñøòàáèðîâàíèÿ âäîëü êîîðäèíàòû y
  std::optional<double> height_zoom;
  if (!IsZero(max_lat_ - min_lat)) {
    height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
  }

  if (width_zoom && height_zoom) {
    // Êîýôôèöèåíòû ìàñøòàáèðîâàíèÿ ïî øèðèíå è âûñîòå íåíóëåâûå,
    // áåð¸ì ìèíèìàëüíûé èç íèõ
    zoom_coeff_ = std::min(*width_zoom, *height_zoom);
  } else if (width_zoom) {
    // Êîýôôèöèåíò ìàñøòàáèðîâàíèÿ ïî øèðèíå íåíóëåâîé, èñïîëüçóåì åãî
    zoom_coeff_ = *width_zoom;
  } else if (height_zoom) {
    // Êîýôôèöèåíò ìàñøòàáèðîâàíèÿ ïî âûñîòå íåíóëåâîé, èñïîëüçóåì åãî
    zoom_coeff_ = *height_zoom;
  }
}

} // namespace renderer
