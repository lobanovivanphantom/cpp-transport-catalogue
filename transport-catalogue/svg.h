#pragma once
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

struct Rgb {
  Rgb() = default;
  Rgb(unsigned int r, unsigned int g, unsigned int b);
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
};

struct Rgba {
  Rgba() = default;
  Rgba(unsigned int r, unsigned int g, unsigned int b, double o);
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

struct PrintColor {
  PrintColor(std::ostream &stream);

  std::ostream &os;

  void operator()(std::monostate) const;

  void operator()(const std::string &color) const;

  void operator()(const Rgb &rgb) const;

  void operator()(const Rgba &rgba) const;
};

} // namespace svg

std::ostream &operator<<(std::ostream &os, const svg::Color &color);

namespace svg {

struct Point {
  Point() = default;
  Point(double x, double y) : x(x), y(y) {}
  double x = 0;
  double y = 0;
};

/*
 * Âñïîìîãàòåëüíàÿ ñòðóêòóðà, õðàíÿùàÿ êîíòåêñò äëÿ âûâîäà SVG-äîêóìåíòà ñ
 * îòñòóïàìè. Õðàíèò ññûëêó íà ïîòîê âûâîäà, òåêóùåå çíà÷åíèå è øàã îòñòóïà ïðè
 * âûâîäå ýëåìåíòà
 */
struct RenderContext {
  RenderContext(std::ostream &out) : out(out) {}

  RenderContext(std::ostream &out, int indent_step, int indent = 0)
      : out(out), indent_step(indent_step), indent(indent) {}

  RenderContext Indented() const {
    return {out, indent_step, indent + indent_step};
  }

  void RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
      out.put(' ');
    }
  }

  std::ostream &out;
  int indent_step = 0;
  int indent = 0;
};

enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};

enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};

} // namespace svg

std::ostream &operator<<(std::ostream &os, const svg::StrokeLineCap &line_cap);

std::ostream &operator<<(std::ostream &os,
                         const svg::StrokeLineJoin &line_join);

namespace svg {

template <typename Owner> class PathProps {
public:
  Owner &SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return AsOwner();
  }
  Owner &SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
  }

  Owner &SetStrokeWidth(double width) {
    stroke_width_ = width;
    return AsOwner();
  }

  Owner &SetStrokeLineCap(StrokeLineCap line_cap) {
    line_cap_ = line_cap;
    return AsOwner();
  }

  Owner &SetStrokeLineJoin(StrokeLineJoin line_join) {
    line_join_ = line_join;
    return AsOwner();
  }

protected:
  ~PathProps() = default;

  void RenderAttrs(std::ostream &out) const {
    using namespace std::literals;

    if (fill_color_) {
      out << " fill=\""sv << *fill_color_ << "\""sv;
    }
    if (stroke_color_) {
      out << " stroke=\""sv << *stroke_color_ << "\""sv;
    }
    if (stroke_width_) {
      out << " stroke-width=\"" << *stroke_width_ << "\"";
    }
    if (line_cap_) {
      out << " stroke-linecap=\"" << *line_cap_ << "\"";
    }
    if (line_join_) {
      out << " stroke-linejoin=\"" << *line_join_ << "\"";
    }
  }

private:
  Owner &AsOwner() {
    // static_cast áåçîïàñíî ïðåîáðàçóåò *this ê Owner&,
    // åñëè êëàññ Owner — íàñëåäíèê PathProps
    return static_cast<Owner &>(*this);
  }

  std::optional<Color> fill_color_;
  std::optional<Color> stroke_color_;
  std::optional<double> stroke_width_;
  std::optional<StrokeLineCap> line_cap_;
  std::optional<StrokeLineJoin> line_join_;
};

/*
 * Àáñòðàêòíûé áàçîâûé êëàññ Object ñëóæèò äëÿ óíèôèöèðîâàííîãî õðàíåíèÿ
 * êîíêðåòíûõ òåãîâ SVG-äîêóìåíòà
 * Ðåàëèçóåò ïàòòåðí "Øàáëîííûé ìåòîä" äëÿ âûâîäà ñîäåðæèìîãî òåãà
 */
class Object {
public:
  void Render(const RenderContext &context) const;

  virtual ~Object() = default;

private:
  virtual void RenderObject(const RenderContext &context) const = 0;
};

/*
 * Êëàññ Circle ìîäåëèðóåò ýëåìåíò <circle> äëÿ îòîáðàæåíèÿ êðóãà
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
  Circle &SetCenter(Point center);
  Circle &SetRadius(double radius);

private:
  void RenderObject(const RenderContext &context) const override;

  Point center_;
  double radius_ = 1.0;
};

/*
 * Êëàññ Polyline ìîäåëèðóåò ýëåìåíò <polyline> äëÿ îòîáðàæåíèÿ ëîìàíûõ ëèíèé
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
  // Äîáàâëÿåò î÷åðåäíóþ âåðøèíó ê ëîìàíîé ëèíèè
  Polyline &AddPoint(Point point);

private:
  void RenderObject(const RenderContext &context) const override;

  std::vector<Point> polyline_;
};

/*
 * Êëàññ Text ìîäåëèðóåò ýëåìåíò <text> äëÿ îòîáðàæåíèÿ òåêñòà
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
  // Çàäà¸ò êîîðäèíàòû îïîðíîé òî÷êè (àòðèáóòû x è y)
  Text &SetPosition(Point pos);

  // Çàäà¸ò ñìåùåíèå îòíîñèòåëüíî îïîðíîé òî÷êè (àòðèáóòû dx, dy)
  Text &SetOffset(Point offset);

  // Çàäà¸ò ðàçìåðû øðèôòà (àòðèáóò font-size)
  Text &SetFontSize(uint32_t size);

  // Çàäà¸ò íàçâàíèå øðèôòà (àòðèáóò font-family)
  Text &SetFontFamily(std::string font_family);

  // Çàäà¸ò òîëùèíó øðèôòà (àòðèáóò font-weight)
  Text &SetFontWeight(std::string font_weight);

  // Çàäà¸ò òåêñòîâîå ñîäåðæèìîå îáúåêòà (îòîáðàæàåòñÿ âíóòðè òåãà text)
  Text &SetData(std::string data);

  // Ïðî÷èå äàííûå è ìåòîäû, íåîáõîäèìûå äëÿ ðåàëèçàöèè ýëåìåíòà <text>
private:
  void RenderObject(const RenderContext &context) const override;

  Point pos_;
  Point offset_;
  uint32_t font_size_ = 1;
  std::string font_family_;
  std::string font_weight_;
  std::string text_;
};

class ObjectContainer {
public:
  template <typename T> void Add(T obj) {
    AddPtr(std::make_unique<T>(std::move(obj)));
  }

  virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;

  virtual ~ObjectContainer() = default;
};

class Drawable {
public:
  virtual void Draw(ObjectContainer &container) const = 0;

  virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
  void AddPtr(std::unique_ptr<Object> &&obj) override;

  void Render(std::ostream &out) const;

private:
  std::deque<std::unique_ptr<Object>> objects_;
};

} // namespace svg