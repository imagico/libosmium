#ifndef OSMIUM_AREA_SEGMENT
#define OSMIUM_AREA_SEGMENT

/*

This file is part of Osmium (http://osmcode.org/osmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <boost/operators.hpp>

#include <osmium/osm/noderef.hpp>
#include <osmium/osm/ostream.hpp>

namespace osmium {

    namespace area {

        /**
         * Helper class for Assembler class.
         */
        class NodeRefSegment : boost::less_than_comparable<NodeRefSegment> {

            osmium::NodeRef m_first;
            osmium::NodeRef m_second;

            bool m_cw;

            void swap_locations() {
                using std::swap;
                swap(m_first, m_second);
            }

        public:

            NodeRefSegment() :
                m_first(),
                m_second() {
            }

            NodeRefSegment(const osmium::NodeRef& nr1, const osmium::NodeRef& nr2) :
                m_first(nr1),
                m_second(nr2) {
                if (nr2.location() < nr1.location()) {
                    swap_locations();
                }
            }

            NodeRefSegment(const NodeRefSegment&) = default;
            NodeRefSegment(NodeRefSegment&&) = default;

            NodeRefSegment& operator=(const NodeRefSegment&) = default;
            NodeRefSegment& operator=(NodeRefSegment&&) = default;

            ~NodeRefSegment() = default;

            /// Return first NodeRef of Segment.
            const osmium::NodeRef& first() const {
                return m_first;
            }

            /// Return second NodeRef of Segment.
            const osmium::NodeRef& second() const {
                return m_second;
            }

            bool cw() const {
                return m_cw;
            }

            NodeRefSegment make_cw() {
                m_cw = true;
                NodeRefSegment seg(*this);
                if (seg.first().location().y() > seg.second().location().y()) {
                    seg.swap_locations();
                }
                return seg;
            }

            NodeRefSegment make_ccw() {
                m_cw = false;
                NodeRefSegment seg(*this);
                if (seg.first().location().y() < seg.second().location().y()) {
                    seg.swap_locations();
                }
                return seg;
            }

        }; // class NodeRefSegment

        /// NodeRefSegments are equal if both their locations are equal
        inline constexpr bool operator==(const NodeRefSegment& lhs, const NodeRefSegment& rhs) {
            return lhs.first().location() == rhs.first().location() && lhs.second().location() == rhs.second().location();
        }

        /**
         * NodeRefSegments are "smaller" if they are to the left and down of another
         * segment. The first() location is checked first() and only if they have the
         * same first() location the second() location is taken into account.
         */
        inline bool operator<(const NodeRefSegment& lhs, const NodeRefSegment& rhs) {
            return (lhs.first().location() == rhs.first().location() && lhs.second().location() < rhs.second().location()) || lhs.first().location() < rhs.first().location();
        }

        inline std::ostream& operator<<(std::ostream& out, const NodeRefSegment& segment) {
            out << segment.first() << "--" << segment.second();
            return out;
        }

        inline bool outside_x_range(const NodeRefSegment& s1, const NodeRefSegment& s2) {
            if (s1.first().location().x() > s2.second().location().x()) {
                return true;
            }
            return false;
        }

        inline bool y_range_overlap(const NodeRefSegment& s1, const NodeRefSegment& s2) {
            int tmin = s1.first().location().y() < s1.second().location().y() ? s1.first().location().y( ) : s1.second().location().y();
            int tmax = s1.first().location().y() < s1.second().location().y() ? s1.second().location().y() : s1.first().location().y();
            int omin = s2.first().location().y() < s2.second().location().y() ? s2.first().location().y()  : s2.second().location().y();
            int omax = s2.first().location().y() < s2.second().location().y() ? s2.second().location().y() : s2.first().location().y();
            if (tmin > omax || omin > tmax) {
                return false;
            }
            return true;
        }

        inline osmium::Location calculate_intersection(const NodeRefSegment& s1, const NodeRefSegment& s2) {
            if (s1.first()  == s2.first()  ||
                s1.first()  == s2.second() ||
                s1.second() == s2.first()  ||
                s1.second() == s2.second()) {
                return osmium::Location();
            }

            double denom = ((s2.second().lat() - s2.first().lat())*(s1.second().lon() - s1.first().lon())) -
                           ((s2.second().lon() - s2.first().lon())*(s1.second().lat() - s1.first().lat()));

            if (denom != 0) {
                double nume_a = ((s2.second().lon() - s2.first().lon())*(s1.first().lat() - s2.first().lat())) -
                                ((s2.second().lat() - s2.first().lat())*(s1.first().lon() - s2.first().lon()));

                double nume_b = ((s1.second().lon() - s1.first().lon())*(s1.first().lat() - s2.first().lat())) -
                                ((s1.second().lat() - s1.first().lat())*(s1.first().lon() - s2.first().lon()));

                if ((denom > 0 && nume_a >= 0 && nume_a <= denom && nume_b >= 0 && nume_b <= denom) ||
                    (denom < 0 && nume_a <= 0 && nume_a >= denom && nume_b <= 0 && nume_b >= denom)) {
                    double ua = nume_a / denom;
                    double ix = s1.first().lon() + ua*(s1.second().lon() - s1.first().lon());
                    double iy = s1.first().lat() + ua*(s1.second().lat() - s1.first().lat());
                    return osmium::Location(ix, iy);
                }
            }

            return osmium::Location();
        }

    } // namespace area

} // namespace osmium

#endif // OSMIUM_AREA_SEGMENT