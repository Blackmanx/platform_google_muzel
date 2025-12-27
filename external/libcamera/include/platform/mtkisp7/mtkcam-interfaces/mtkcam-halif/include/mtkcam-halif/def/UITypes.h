/*
 * Copyright (C) 2022 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INCLUDE_MTKCAM_HALIF_DEF_UITYPES_H_
#define INCLUDE_MTKCAM_HALIF_DEF_UITYPES_H_

namespace NSCam {

/**
 * Camera UI Types.
 */
struct MPoint;
struct MSize;
struct MRect;

/** Camera point type. */
struct MPoint {
  typedef int value_type;  ///< Data type of attributes

  value_type x = 0;  ///< Position x.
  value_type y = 0;  ///< Position y.

 public:  ////                Instantiation.
  /** Default constructor, set x and y to 0 */
  MPoint() = default;

  /** Constructs MPoint with a given x value */
  explicit MPoint(int _x) : x(_x) {}

  /** Constructs MPoint with the given x and y values */
  inline MPoint(int _x, int _y) : x(_x), y(_y) {}

 public:  ////                Operators.
  /**
   * Checks for equality between two points.
   *  @param rhs The target to compare.
   *  @return The compare result.
   *  @retval true Both x and y are the same.
   *  @retval false Any attribute doesn't equal.
   */
  inline bool operator==(MPoint const &rhs) const {
    return (x == rhs.x) && (y == rhs.y);
  }

  /**
   * Checks for inequality between two points.
   *  @see MPoint::operator==
   */
  inline bool operator!=(MPoint const &rhs) const { return !operator==(rhs); }

  /**
   * Compares if the position is smaller than the given target `rhs`.
   * If `rhs.y` is greather than `y`, this method retruns `true` directly, if
   * `rhs.y` is smaller than `y`, this method returns `false directly, if
   * `rhs.y == y`, returns `x < rhs.x`.
   *  @param rhs Target to compare
   *  @return Compare result.
   *  @retval true If `rhs` smaller than `this`.
   *  @retval false If `rhs` greater or the same of `this`.
   */
  inline bool operator<(MPoint const &rhs) const {
    return y < rhs.y || (y == rhs.y && x < rhs.x);
  }

  /**
   * Increases `this` by the given `rhs`.
   *  @param rhs Target to add.
   *  @return The reference of `this`.
   */
  inline MPoint &operator+=(MPoint const &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  /**
   * Subtracts `this` by the given `rhs`.
   *  @param rhs Target to subtract.
   *  @return The reference of `this`.
   */
  inline MPoint &operator-=(MPoint const &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  /**
   * Returns a MPoint composed by `this + rhs`.
   *  @param rhs Target to add.
   *  @return The added MPoint instance.
   */
  inline MPoint operator+(MPoint const &rhs) const {
    MPoint const result(x + rhs.x, y + rhs.y);
    return result;
  }

  /**
   * Returns a MPoint composed by `this - rhs`.
   *  @param rhs Target to subtract.
   *  @return The subtracted MPoint instance.
   */
  inline MPoint operator-(MPoint const &rhs) const {
    MPoint const result(x - rhs.x, y - rhs.y);
    return result;
  }

  /**
   * Inverses signed flags of `x` and `y`.
   *  @return The reference of `this`.
   */
  inline MPoint &operator-() {
    x = -x;
    y = -y;
    return *this;
  }

  /**
   * Checks if `this` is at point (0, 0).
   *  @return If `this` is at point (0, 0), `true` indicates yes, `false`
   *          indicates no.
   */
  inline bool isOrigin() const { return !(x | y); }
};

/** Camera size type. */
struct MSize {
  typedef int value_type;  ///< Data type of attributes.
  value_type w = 0;  ///< Width.
  value_type h = 0;  ///< Height.

 public:  ////                Instantiation.
  /** Default constructor, set w and h to 0 */
  MSize() = default;

  /**
   * Constructs `MSize` with a given width `_w`.
   *  @param _w The given width.
   */
  explicit MSize(int _w) : w(_w) {}

  /**
   * Constructs `MSize` with given width and height `_w`, `_h`.
   *  @param _w The given width.
   *  @param _h The given height.
   */
  inline MSize(int _w, int _h) : w(_w), h(_h) {}

  /**
   * Constructs `MSize` with top-left and bottom-right points which draws a
   * rectangle and assign `w` and `h` to the width and height of this
   * rectangle.
   *  @param topLeft The top-left point.
   *  @param bottomRight The bottom-right point.
   */
  inline MSize(MPoint const &topLeft, MPoint const &bottomRight)
      : w(bottomRight.x - topLeft.x), h(bottomRight.y - topLeft.y) {}

 public:  ////                Operations.
  /**
   * Returns the rectangle area by production of w and h.
   *  @return Rectangle area.
   */
  inline value_type size() const { return w * h; }

 public:  ////                Operators.
  /**
   * Checks for invalid size with width <= 0 or height <= 0.
   *  @return The check result, `true` indicates to invalid size.
   */
  inline bool operator!() const { return (w <= 0) || (h <= 0); }

  /**
   * Checks for equality between two sizes.
   *  @param rhs Target to compare.
   *  @return The check result, `true` if equal.
   */
  inline bool operator==(MSize const &rhs) const {
    return (w == rhs.w) && (h == rhs.h);
  }

  /**
   * Checks for inequality between two sizes.
   *  @see MSize::operator==
   */
  inline bool operator!=(MSize const &rhs) const { return !operator==(rhs); }

  /**
   * Adds a size to this size.
   *  @param rhs Target to add.
   *  @return The reference of `this`.
   */
  inline MSize &operator+=(MSize const &rhs) {
    w += rhs.w;
    h += rhs.h;
    return *this;
  }

  /**
   * Subtracts a size from this size.
   *  @param rhs Target to subtract.
   *  @return The reference of `this`.
   */
  inline MSize &operator-=(MSize const &rhs) {
    w -= rhs.w;
    h -= rhs.h;
    return *this;
  }

  /**
   * Return a MSize instance that adds `this` and `rhs`.
   *  @param rhs Target to add.
   *  @return The added MSize instance.
   */
  inline MSize operator+(MSize const &rhs) const {
    MSize const result(w + rhs.w, h + rhs.h);
    return result;
  }

  /**
   * Return a MSize instance that substracts `this` and `rhs`.
   *  @param rhs Target to substract.
   *  @return The substracted MSize instance.
   */
  inline MSize operator-(MSize const &rhs) const {
    MSize const result(w - rhs.w, h - rhs.h);
    return result;
  }

  /**
   * Return a MSize instance that multiplies `this` by a scalar.
   *  @param scalar The scalar in integer.
   *  @return The multipled MSize instance.
   */
  inline MSize operator*(value_type scalar) const {
    MSize const result(w * scalar, h * scalar);
    return result;
  }

  /**
   * Return a MSize instance that divides `this` by a scalar.
   *  @param scalar The scalar in integer.
   *  @return The divided MSize instance.
   */
  inline MSize operator/(value_type scalar) const {
    MSize const result(w / scalar, h / scalar);
    return result;
  }

  /**
   * Return a MSize instance that shifts the bits count `shift` to the right.
   *  @param shift The bit count to right shift.
   *  @return The shifted MSize instance.
   */
  inline MSize operator>>(value_type shift) const {
    MSize const result(w >> shift, h >> shift);
    return result;
  }

  /**
   * Return a MSize instance that shifts the bit count `shift` to the left.
   *  @param shift The bit count to left shift.
   *  @return The shifted MSize instance.
   */
  inline MSize operator<<(value_type shift) const {
    MSize const result(w << shift, h << shift);
    return result;
  }
};

/** Camera rectangle type. */
struct MRect {
  typedef int value_type;  ///< Data type of attributes.
  MPoint p;  ///<  Top-left corner
  MSize s;   ///<  Width and height

 public:  ////                Instantiation.
  /** Constructs MRect as an empty rectangle. */
  MRect() = default;

  /**
   * Constructs MRect with a given width `_w`.
   *  @param _w The given width.
   */
  explicit MRect(int _w) : s(_w) {}

  /**
   * Constructs MRect with the given size.
   *  @param _w The given width.
   *  @param _h The given height
   */
  inline MRect(int _w, int _h) : s(_w, _h) {}

  /**
   * Constructs MRect with a rectangle described by top-left and
   * bottom-right points.
   *  @param topLeft The top-left point of the given rectangle
   *  @param bottomRight The bottom-right point the of the given rectangle.
   */
  inline MRect(MPoint const &topLeft, MPoint const &bottomRight)
      : p(topLeft), s(topLeft, bottomRight) {}

  /**
   * Constructs MRect with a rectangle described by point top-left `_p` and
   * the rectangle size `_s`.
   */
  inline MRect(MPoint const &_p, MSize const &_s) : p(_p), s(_s) {}

 public:  ////                Operators.
  /**
   * Checks for equality between two rectangles.
   *  @param rhs Target to compare.
   *  @return `true` indicates to the same points, size rectangle,
   *          otherwise `false`.
   */
  inline bool operator==(MRect const &rhs) const {
    return (p == rhs.p) && (s == rhs.s);
  }

  /**
   * Checks for inequality between two rectangles.
   *  @see MRect::operator==
   */
  inline bool operator!=(MRect const &rhs) const { return !operator==(rhs); }

 public:  ////                Accessors.
  /**
   * Get the top-left point.
   *  @return Top-left point.
   */
  inline MPoint leftTop() const { return p; }

  /**
   * Get the bottom-left point.
   *  @return Bottom-left point.
   */
  inline MPoint leftBottom() const { return MPoint(p.x, p.y + s.h); }

  /**
   * Get the Top-right point.
   *  @return Top-right point.
   */
  inline MPoint rightTop() const { return MPoint(p.x + s.w, p.y); }

  /**
   * Get the bottom-right point.
   *  @return Bottom-right point.
   */
  inline MPoint rightBottom() const { return MPoint(p.x + s.w, p.y + s.h); }

  /**
   * Get the rectangle size.
   *  @return Rectangle size.
   */
  inline MSize const &size() const { return s; }

  /**
   * Get the rectangle width.
   *  @return Rectangle width.
   */
  inline value_type width() const { return s.w; }

  /**
   * Get the rectangle height.
   *  @return Rectangle height.
   */
  inline value_type height() const { return s.h; }

 public:  ////                Operations.
  /**
   * Reset this rectangle to point (0, 0) and size = 0.
   */
  inline void clear() { p.x = p.y = s.w = s.h = 0; }
};

struct MPointF;
struct MSizeF;
struct MRectF;

/**
 * Camera point type, same behavior of MPoint but attribute `x` and `y`
 * are type of `float`.
 *  @see MPoint
 */
struct MPointF {
  typedef float value_type;

  value_type x = 0;
  value_type y = 0;

 public:  ////                Instantiation.
  // we don't provide copy-ctor and copy assignment on purpose
  // because we want the compiler generated versions
  MPointF() = default;
  explicit MPointF(value_type _x) : x(_x) {}
  inline MPointF(value_type _x, value_type _y) : x(_x), y(_y) {}

  explicit inline MPointF(MPoint const &rhs) : x(rhs.x), y(rhs.y) {}

 public:  ////                Operators.
  // Checks for equality between two points.
  inline bool operator==(MPointF const &rhs) const {
    return (x == rhs.x) && (y == rhs.y);
  }

  // Checks for inequality between two points.
  inline bool operator!=(MPointF const &rhs) const { return !operator==(rhs); }

  inline bool operator<(MPointF const &rhs) const {
    return y < rhs.y || (y == rhs.y && x < rhs.x);
  }

  inline MPointF &operator+=(MPoint const &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  inline MPointF &operator+=(MPointF const &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  inline MPointF &operator-=(MPointF const &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  inline MPointF &operator=(MPoint const &rhs) {
    x = rhs.x;
    y = rhs.y;
    return *this;
  }

  inline MPointF operator+(MPointF const &rhs) const {
    MPointF const result(x + rhs.x, y + rhs.y);
    return result;
  }

  inline MPointF operator+(MPoint const &rhs) const {
    MPointF const result(x + rhs.x, y + rhs.y);
    return result;
  }

  inline MPointF operator-(MPointF const &rhs) const {
    MPointF const result(x - rhs.x, y - rhs.y);
    return result;
  }

 public:  ////                Attributes.
  inline bool isOrigin() const { return (x == 0.0f) && (y == 0.0f); }

  inline MPoint toMPoint() const {
    MPoint const result(x, y);
    return result;
  }
};

/**
 * Camera size type, same behavior of MSize but attributes `w` and `h` are
 * type of `float`.
 *  @see MSize
 */
struct MSizeF {
  typedef float value_type;
  value_type w = 0;
  value_type h = 0;

 public:  ////                Instantiation.
  // we don't provide copy-ctor and copy assignment on purpose
  // because we want the compiler generated versions
  MSizeF() = default;
  explicit MSizeF(value_type _w) : w(_w) {}
  inline MSizeF(value_type _w, value_type _h) : w(_w), h(_h) {}

  inline MSizeF(MPointF const &topLeft, MPointF const &bottomRight)
      : w(bottomRight.x - topLeft.x), h(bottomRight.y - topLeft.y) {}

  explicit MSizeF(MSize const &rhs) : w(rhs.w), h(rhs.h) {}

 public:  ////                Operations.
  // Returns the product of w and h.
  inline value_type size() const { return w * h; }

 public:  ////                Operators.
  // Checks for invalid size with width <= 0 or height <= 0.
  inline bool operator!() const { return (w <= 0) || (h <= 0); }

  // Checks for equality between two sizes.
  inline bool operator==(MSizeF const &rhs) const {
    return (w == rhs.w) && (h == rhs.h);
  }

  // Checks for inequality between two sizes.
  inline bool operator!=(MSizeF const &rhs) const { return !operator==(rhs); }

  // Adds a size to this size.
  inline MSizeF &operator+=(MSizeF const &rhs) {
    w += rhs.w;
    h += rhs.h;
    return *this;
  }

  inline MSizeF &operator+=(MSize const &rhs) {
    w += rhs.w;
    h += rhs.h;
    return *this;
  }

  // Subtracts a size from this size.
  inline MSizeF &operator-=(MSizeF const &rhs) {
    w -= rhs.w;
    h -= rhs.h;
    return *this;
  }

  inline MSizeF &operator=(MSize const &rhs) {
    w = rhs.w;
    h = rhs.h;
    return *this;
  }

  // Adds two sizes.
  inline MSizeF operator+(MSizeF const &rhs) const {
    MSizeF const result(w + rhs.w, h + rhs.h);
    return result;
  }

  inline MSizeF operator+(MSize const &rhs) const {
    MSizeF const result(w + rhs.w, h + rhs.h);
    return result;
  }

  // Subtracts two sizes.
  inline MSizeF operator-(MSizeF const &rhs) const {
    MSizeF const result(w - rhs.w, h - rhs.h);
    return result;
  }

  // Multiplies a size by a scalar.
  inline MSizeF operator*(value_type scalar) const {
    MSizeF const result(w * scalar, h * scalar);
    return result;
  }

  // Divides a size by a scalar.
  inline MSizeF operator/(value_type scalar) const {
    MSizeF const result(w / scalar, h / scalar);
    return result;
  }

 public:  ////                Operations.
  inline MSize toMSize() const {
    MSize const result(w, h);
    return result;
  }
};

/**
 * Camera rectangle type, same behaviors of MRect but data attributes are
 * type of `float`.
 *  @see MRect
 */
struct MRectF {
  typedef float value_type;
  MPointF p;  //  left-top corner
  MSizeF s;   //  width, height

 public:  ////                Instantiation.
  // we don't provide copy-ctor and copy assignment on purpose
  // because we want the compiler generated versions
  MRectF() = default;
  explicit MRectF(value_type _w) : s(_w, 0) {}
  inline MRectF(value_type _w, value_type _h) : s(_w, _h) {}

  inline MRectF(MPointF const &topLeft, MPointF const &bottomRight)
      : p(topLeft), s(topLeft, bottomRight) {}

  inline MRectF(MPointF const &_p, MSizeF const &_s) : p(_p), s(_s) {}

  inline MRectF(MPoint const &_p, MSize const &_s) : p(_p), s(_s) {}

 public:  ////                Operators.
  // Checks for equality between two rectangles.
  inline bool operator==(MRectF const &rhs) const {
    return (p == rhs.p) && (s == rhs.s);
  }

  // Checks for inequality between two rectangles.
  inline bool operator!=(MRectF const &rhs) const { return !operator==(rhs); }

  inline MRectF &operator=(MRect const &rhs) {
    p = rhs.p;
    s = rhs.s;
    return *this;
  }

 public:  ////                Accessors.
  inline MPointF leftTop() const { return p; }
  inline MPointF leftBottom() const { return MPointF(p.x, p.y + s.h); }
  inline MPointF rightTop() const { return MPointF(p.x + s.w, p.y); }
  inline MPointF rightBottom() const { return MPointF(p.x + s.w, p.y + s.h); }

  inline MSizeF const &size() const { return s; }

  inline value_type width() const { return s.w; }
  inline value_type height() const { return s.h; }

 public:  ////                Operations.
  inline void clear() { p.x = p.y = s.w = s.h = 0; }
  inline MRect toMRect() const {
    MRect const result(p.toMPoint(), s.toMSize());
    return result;
  }
};

}       // namespace NSCam
#endif  // INCLUDE_MTKCAM_HALIF_DEF_UITYPES_H_
