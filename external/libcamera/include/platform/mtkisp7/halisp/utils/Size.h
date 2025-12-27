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

#ifndef AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_SIZE_H_
#define AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_SIZE_H_

#include <cmath>
#include <cstddef>  // size_t
#include <cstdint>

namespace mtk {
namespace isphal {

/**
 * Describes a size.
 */
class Size {
 public:
  explicit Size(size_t w, size_t h) : m_width(w), m_height(h) {}
  Size() : Size(0, 0) {}
  ~Size() = default;

  // copyable & movable
 public:
  Size(const Size&) = default;
  Size(Size&&) = default;
  Size& operator=(const Size&) = default;
  Size& operator=(Size&&) = default;

 public:
  /**
   * Get width, both read and write.
   *  @return The reference of width.
   */
  inline size_t& width() { return m_width; }

  /**
   * Get height, both read and write.
   *  @return The reference of height.
   */
  inline size_t& height() { return m_height; }

  /**
   * Get width, read-only.
   *  @return The reference of width.
   */
  inline const size_t& width() const { return m_width; }

  /**
   * Get height, read-only.
   *  @return The reference of height.
   */
  inline const size_t& height() const { return m_height; }

  /**
   * Check the size is valid or not (width * height > 0).
   *  @return True for a valid size.
   */
  inline operator bool() const { return (m_width == 0) || (m_height == 0); }

  /**
   * Get the size area.
   *  @return Area size.
   */
  inline size_t getArea() const { return m_width * m_height; }

 private:
  size_t m_width;
  size_t m_height;
};

/**
 * Describes a position (x, y).
 */
class Position {
 public:
  explicit Position(int x, int y) : m_x(x), m_y(y) {}
  Position() : Position(0, 0) {}

  // copyable & movable
 public:
  Position(const Position&) = default;
  Position(Position&&) = default;
  Position& operator=(const Position&) = default;
  Position& operator=(Position&&) = default;

 public:
  /**
   * Calculate distance between points |p1| and |p2|.
   *  @param p1 Point 1.
   *  @param p2 Point 2.
   *  @return The linear distance in double.
   */
  static double distance(Position p1, Position p2) { return p1.distanceTo(p2); }

  /**
   * Calculate the distance between the given point |p|.
   *  @param p The given point.
   *  @return The linear distance in double.
   */
  inline double distanceTo(Position p) { return distanceTo(p.x(), p.y()); }

  /**
   * Calculate the distance between the given point [x, y].
   *  @param x X-coordination value.
   *  @param y Y-coordination value.
   *  @return The linear distance in double.
   */
  inline double distanceTo(int x, int y) {
    const int x_dist = x - m_x;
    const int y_dist = y - m_y;
    return static_cast<double>(std::sqrt(x_dist * x_dist + y_dist * y_dist));
  }

 public:
  inline int& x() { return m_x; }
  inline int& y() { return m_y; }
  inline const int& x() const { return m_x; }
  inline const int& y() const { return m_y; }

 private:
  int m_x;
  int m_y;
};

/**
 * Describes a rectangle which contains position and size.
 */
class Rectangle {
 public:
  explicit Rectangle(int x, int y, size_t w, size_t h)
      : m_pos(x, y), m_sz(w, h) {}
  Rectangle() : Rectangle(0, 0, 0, 0) {}

  // copyable and movable
 public:
  Rectangle(const Rectangle&) = default;
  Rectangle(Rectangle&&) = default;
  Rectangle& operator=(const Rectangle&) = default;
  Rectangle& operator=(Rectangle&&) = default;

 public:
  /**
   * Retrieve mtk::isphal::Position instance.
   *  @return Reference of |m_pos|.
   */
  inline mtk::isphal::Position& pos() { return m_pos; }

  /**
   * Retrieve mtk::isphal::Position instance.
   *  @return Reference of |m_pos|.
   */
  inline const mtk::isphal::Position& pos() const { return m_pos; }

  /**
   * Retrieve mtk::isphal::Size instance.
   *  @return Reference of |m_sz|.
   */
  inline mtk::isphal::Size& size() { return m_sz; }

  /**
   * Retrieve mtk::isphal::Size instance.
   *  @return Reference of |m_sz|.
   */
  inline const mtk::isphal::Size& size() const { return m_sz; }

  /**
   * Get the current area.
   *  @return Area, equals to |width| * |height|.
   */
  inline size_t getArea() const { return m_sz.getArea(); }

  /**
   * Checks if the rectangle is valid (area is greater than 0).
   *  @return true for yes, no for not.
   */
  inline operator bool() const { return !!m_sz; }

 private:
  mtk::isphal::Position m_pos;
  mtk::isphal::Size m_sz;
};

}  // namespace isphal
}  // namespace mtk

#endif  // AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_SIZE_H_
