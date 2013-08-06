#ifndef OSMIUM_INDEX_MAP_VECTOR_HPP
#define OSMIUM_INDEX_MAP_VECTOR_HPP

/*

This file is part of Osmium (http://osmcode.org/osmium).

Copyright 2013 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include <osmium/osm/types.hpp>
#include <osmium/index/map.hpp>
#include <osmium/io/detail/read_write.hpp>

namespace osmium {

    namespace index {

        namespace map {

            /**
            * This class uses a vector of TKey/TValue pairs to store the
            * data. The vector must be filled ordered by ID (OSM files
            * are generally ordered that way, so that is usually not a
            * problem). If the vector is not filled in order, the sort()
            * method must be called before reading values back. Lookup
            * uses a binary search.
            *
            * This has very low memory overhead for small OSM datasets.
            */
            template <typename TKey, typename TValue>
            class Vector : public osmium::index::map::Map<TKey, TValue> {

                struct element_type {
                    TKey id;
                    TValue value;

                    element_type(TKey i, TValue v = TValue()) :
                        id(i),
                        value(v) {
                    }

                    bool operator<(const element_type& other) const {
                        return this->id < other.id;
                    }

                    bool operator==(const element_type& other) const {
                        return this->id == other.id;
                    }

                    bool operator!=(const element_type& other) const {
                        return !(*this == other);
                    }
                };

                std::vector<element_type> m_elements;

            public:

                Vector() = default;

                ~Vector() noexcept override final = default;

                void set(const TKey id, const TValue value) override final {
                    m_elements.push_back(element_type(id, value));
                }

                const TValue get(const TKey id) const override final {
                    const element_type item(id);
                    const auto result = std::lower_bound(m_elements.begin(), m_elements.end(), item);
                    if (result == m_elements.end() || *result != item) {
                        throw std::out_of_range("Unknown ID");
                    } else {
                        return result->value;
                    }
                }

                size_t size() const override final {
                    return m_elements.size();
                }

                size_t used_memory() const override final {
                    return size() * sizeof(element_type);
                }

                void clear() override final {
                    m_elements.clear();
                    m_elements.shrink_to_fit();
                }

                void sort() override final {
                    std::sort(m_elements.begin(), m_elements.end());
                }

                void dump_as_list(const int fd) const {
                    osmium::io::detail::reliable_write(fd, reinterpret_cast<const char*>(m_elements.data()), sizeof(element_type) * m_elements.size());
                }

            }; // class Vector

        } // namespace map

    } // namespace index

} // namespace osmium

#endif // OSMIUM_INDEX_MAP_VECTOR_HPP
