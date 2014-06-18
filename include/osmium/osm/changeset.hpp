#ifndef OSMIUM_OSM_CHANGESET_HPP
#define OSMIUM_OSM_CHANGESET_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

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

#include <cstdint>
#include <cstring>

#include <boost/operators.hpp>

#include <osmium/memory/collection.hpp>
#include <osmium/memory/item.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/entity.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/osm/types.hpp>

namespace osmium {

    namespace builder {
        template <class T> class ObjectBuilder;
    }

    class Changeset : public osmium::OSMEntity, boost::totally_ordered<Changeset> {

        friend class osmium::builder::ObjectBuilder<osmium::Changeset>;

        osmium::Timestamp m_created_at {};
        osmium::Timestamp m_closed_at {};
        osmium::Box       m_bounds {};
        changeset_id_type m_id {0};
        num_changes_type  m_num_changes {0};
        user_id_type      m_uid {0};
        string_size_type  m_user_size;

        Changeset() :
            OSMEntity(sizeof(Changeset), osmium::item_type::changeset) {
        }

        void user_size(string_size_type size) {
            m_user_size = size;
        }

        unsigned char* subitems_position() {
            return data() + osmium::memory::padded_length(sizeof(Changeset) + m_user_size);
        }

        const unsigned char* subitems_position() const {
            return data() + osmium::memory::padded_length(sizeof(Changeset) + m_user_size);
        }

        template <class T>
        T& subitem_of_type() {
            for (iterator it = begin(); it != end(); ++it) {
                if (it->type() == T::itemtype) {
                    return reinterpret_cast<T&>(*it);
                }
            }

            static T subitem;
            return subitem;
        }

        template <class T>
        const T& subitem_of_type() const {
            for (const_iterator it = cbegin(); it != cend(); ++it) {
                if (it->type() == T::itemtype) {
                    return reinterpret_cast<const T&>(*it);
                }
            }

            static const T subitem;
            return subitem;
        }

    public:

        /// Get ID of this changeset
        changeset_id_type id() const noexcept {
            return m_id;
        }

        /**
         * Set ID of this changeset
         *
         * @return Reference to changeset to make calls chainable.
         */
        Changeset& id(changeset_id_type id) noexcept {
            m_id = id;
            return *this;
        }

        /**
         * Set ID of this changeset.
         *
         * @return Reference to object to make calls chainable.
         */
        Changeset& id(const char* id) {
            return this->id(osmium::string_to_changeset_id(id));
        }

        /// Get user id.
        user_id_type uid() const noexcept {
            return m_uid;
        }

        /**
         * Set user id.
         *
         * @return Reference to changeset to make calls chainable.
         */
        Changeset& uid(user_id_type uid) noexcept {
            m_uid = uid;
            return *this;
        }

        /**
         * Set user id.
         * Sets uid to 0 (anonymous) if the given uid is smaller than 0.
         *
         * @return Reference to changeset to make calls chainable.
         */
        Changeset& uid_from_signed(int32_t uid) noexcept {
            m_uid = uid < 0 ? 0 : uid;
            return *this;
        }

        /**
         * Set user id.
         *
         * @return Reference to changeset to make calls chainable.
         */
        Changeset& uid(const char* uid) {
            return this->uid_from_signed(string_to_user_id(uid));
        }

        /// Is this user anonymous?
        bool user_is_anonymous() const noexcept {
            return m_uid == 0;
        }

        /// Get timestamp when this changeset was created.
        osmium::Timestamp created_at() const noexcept {
            return m_created_at;
        }

        /**
         * Get timestamp when this changeset was closed.
         *
         * This will return the empty Timestamp when the
         * changeset is not yet closed.
         */
        osmium::Timestamp closed_at() const noexcept {
            return m_closed_at;
        }

        bool open() const noexcept {
            return m_closed_at == osmium::Timestamp();
        }

        bool closed() const noexcept {
            return !open();
        }

        /**
         * Set the timestamp when this changeset was created.
         *
         * @param timestamp Timestamp
         * @return Reference to changeset to make calls chainable.
         */
        Changeset& created_at(const osmium::Timestamp timestamp) {
            m_created_at = timestamp;
            return *this;
        }

        /**
         * Set the timestamp when this changeset was closed.
         *
         * @param timestamp Timestamp
         * @return Reference to changeset to make calls chainable.
         */
        Changeset& closed_at(const osmium::Timestamp timestamp) {
            m_closed_at = timestamp;
            return *this;
        }

        num_changes_type num_changes() const noexcept {
            return m_num_changes;
        }

        Changeset& num_changes(num_changes_type num_changes) noexcept {
            m_num_changes = num_changes;
            return *this;
        }

        Changeset& num_changes(const char* num_changes) noexcept {
            return this->num_changes(osmium::string_to_num_changes(num_changes));
        }

        osmium::Box& bounds() noexcept {
            return m_bounds;
        }

        const osmium::Box& bounds() const noexcept {
            return m_bounds;
        }

        /// Get user name.
        const char* user() const {
            return reinterpret_cast<const char*>(data() + sizeof(Changeset));
        }

        /// Get the list of tags.
        TagList& tags() {
            return subitem_of_type<TagList>();
        }

        /// Get the list of tags.
        const TagList& tags() const {
            return subitem_of_type<const TagList>();
        }

        /**
         * Set named attribute.
         *
         * @param attr Name of the attribute (must be one of "id", "version", "changeset", "timestamp", "uid", "visible")
         * @param value Value of the attribute
         */
        void set_attribute(const char* attr, const char* value) {
            if (!strcmp(attr, "id")) {
                id(value);
            } else if (!strcmp(attr, "num_changes")) {
                num_changes(value);
            } else if (!strcmp(attr, "created_at")) {
                created_at(osmium::Timestamp(value));
            } else if (!strcmp(attr, "closed_at")) {
                closed_at(osmium::Timestamp(value));
            } else if (!strcmp(attr, "uid")) {
                uid(value);
            }
        }

        typedef osmium::memory::CollectionIterator<Item> iterator;
        typedef osmium::memory::CollectionIterator<const Item> const_iterator;

        iterator begin() {
            return iterator(subitems_position());
        }

        iterator end() {
            return iterator(data() + padded_size());
        }

        const_iterator cbegin() const {
            return const_iterator(subitems_position());
        }

        const_iterator cend() const {
            return const_iterator(data() + padded_size());
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator end() const {
            return cend();
        }

    }; // Changeset

    /**
     * Changesets are equal if their IDs are equal.
     */
    inline bool operator==(const Changeset& lhs, const Changeset& rhs) {
        return lhs.id() == rhs.id();
    }

    /**
     * Changesets can be ordered by id.
     */
    inline bool operator<(const Changeset& lhs, const Changeset& rhs) {
        return lhs.id() < rhs.id();
    }

} // namespace osmium

#endif // OSMIUM_OSM_CHANGESET_HPP
