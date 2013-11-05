#ifndef OSMIUM_IO_READER_HPP
#define OSMIUM_IO_READER_HPP

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

#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

#include <osmium/io/input.hpp>

namespace osmium {

    namespace io {

        class Reader {

            osmium::io::File m_file;
            std::unique_ptr<osmium::io::Input> m_input;

            osmium::item_flags_type m_read_types {osmium::item_flags_type::all};
            osmium::memory::Buffer m_buffer {};

            Reader(const Reader&) = delete;
            Reader& operator=(const Reader&) = delete;

            template <class THandler>
            osmium::item_type apply_helper(osmium::memory::Buffer& buffer, osmium::item_type type, THandler& handler) {
                return handler(buffer, type);
            }

            template <class THead, class ...TTail>
            osmium::item_type apply_helper(osmium::memory::Buffer& buffer, osmium::item_type type, THead& handler, TTail&... more) {
                handler(buffer, type);
                return apply_helper(buffer, type, more...);
            }

            template <class THandler>
            void apply_helper(osmium::item_type type, THandler& handler) {
                handler(type);
            }

            template <class THead, class ...TTail>
            void apply_helper(osmium::item_type type, THead& handler, TTail&... more) {
                handler(type);
                apply_helper(type, more...);
            }

        public:

            Reader(const osmium::io::File& file) :
                m_file(file),
                m_input(osmium::io::InputFactory::instance().create_input(m_file)) {
            }

            Reader(const std::string& filename = "") :
                m_file(filename),
                m_input(osmium::io::InputFactory::instance().create_input(m_file)) {
            }

            osmium::io::Header open(osmium::item_flags_type read_types = osmium::item_flags_type::all) {
                m_read_types = read_types;
                return m_input->read(read_types);
            }

            osmium::memory::Buffer read() {
                if (m_read_types == osmium::item_flags_type::nothing) {
                    // If the caller didn't want anything but the header, it will
                    // always get an empty buffer here.
                    return osmium::memory::Buffer();
                }
                return m_input->next_buffer();
            }

            template <class ...THandlers>
            void apply(THandlers&... handlers) {
                osmium::item_type type = osmium::item_type::undefined;
                while (osmium::memory::Buffer buffer = read()) {
                    type = apply_helper(buffer, type, handlers...);
                }
                apply_helper(type, handlers...);
            }

            typedef std::pair<osmium::memory::Buffer::iterator, osmium::memory::Buffer::iterator> buffer_iterator_pair;

            buffer_iterator_pair get_next_iter() {
                m_buffer = read();
                return std::make_pair(m_buffer.begin(), m_buffer.end());
            }

            /**
             * This iterator class allows you to iterate over all items in a file.
             * It hides all the buffer handling and makes the contents of an
             * OSM file accessible as a normal STL input iterator.
             */
            class iterator : public std::iterator<std::input_iterator_tag, const osmium::Object> {

                Reader* m_reader;
                buffer_iterator_pair m_iterators;

            public:

                iterator(Reader* reader, buffer_iterator_pair iterators) :
                    m_reader(reader),
                    m_iterators(iterators) {
                }

                // end iterator
                iterator(Reader* reader) :
                    m_reader(reader),
                    m_iterators(std::make_pair(nullptr, nullptr)) {
                }

                iterator& operator++() {
                    assert(m_iterators.first != nullptr);
                    ++m_iterators.first;
                    if (m_iterators.first == m_iterators.second) {
                        m_iterators = m_reader->get_next_iter();
                    }
                    return *this;
                }

                iterator operator++(int) {
                    iterator tmp(*this);
                    operator++();
                    return tmp;
                }

                bool operator==(const iterator& rhs) const {
                    return m_reader == rhs.m_reader &&
                           m_iterators == rhs.m_iterators;
                }

                bool operator!=(const iterator& rhs) const {
                    return !(*this == rhs);
                }

                reference operator*() const {
                    assert(m_iterators.first != nullptr);
                    return static_cast<reference>(*m_iterators.first);
                }

                pointer operator->() const {
                    assert(m_iterators.first != nullptr);
                    return &static_cast<reference>(*m_iterators.first);
                }

            }; // class iterator

            iterator begin() {
                return iterator { this, get_next_iter() };
            }

            iterator end() {
                return iterator { this };
            }

        }; // class Reader

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_READER_HPP