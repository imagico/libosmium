
#include "osm_object_wrap.hpp"

namespace node_osmium {

    v8::Persistent<v8::FunctionTemplate> OSMObjectWrap::constructor;

    void OSMObjectWrap::Initialize(v8::Handle<v8::Object> target) {
        v8::HandleScope scope;
        constructor = v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New(OSMObjectWrap::New));
        constructor->InstanceTemplate()->SetInternalFieldCount(1);
        constructor->SetClassName(v8::String::NewSymbol("OSMObject"));
        auto attributes = static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
        set_accessor(constructor, "id", get_id, attributes);
        set_accessor(constructor, "version", get_version, attributes);
        set_accessor(constructor, "changeset", get_changeset, attributes);
        set_accessor(constructor, "visible", get_visible, attributes);
        set_accessor(constructor, "timestamp_seconds_since_epoch", get_timestamp, attributes);
        set_accessor(constructor, "uid", get_uid, attributes);
        set_accessor(constructor, "user", get_user, attributes);
        node::SetPrototypeMethod(constructor, "tags", tags);
        target->Set(v8::String::NewSymbol("OSMObject"), constructor->GetFunction());
    }

    v8::Handle<v8::Value> OSMObjectWrap::New(const v8::Arguments& args) {
        if (args.Length() == 1 && args[0]->IsExternal()) {
            v8::Local<v8::External> ext = v8::Local<v8::External>::Cast(args[0]);
            static_cast<OSMObjectWrap*>(ext->Value())->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(v8::Exception::TypeError(v8::String::New("osmium.OSMObject cannot be created in Javascript")));
        }
    }

    v8::Handle<v8::Value> OSMObjectWrap::tags(const v8::Arguments& args) {
        v8::HandleScope scope;

        const osmium::OSMObject& object = static_cast<const osmium::OSMObject&>(*(node::ObjectWrap::Unwrap<OSMObjectWrap>(args.This())->m_entity));

        switch (args.Length()) {
            case 0:
                {
                    v8::Local<v8::Object> tags = v8::Object::New();
                    for (const auto& tag : object.tags()) {
                        tags->Set(v8::String::New(tag.key()), v8::String::New(tag.value()));
                    }
                    return scope.Close(tags);
                }
            case 1:
                {
                    if (!args[0]->IsString()) {
                        return ThrowException(v8::Exception::TypeError(v8::String::New("call tags() without parameters or with a string (key)")));
                    }
                    const char* value = object.tags().get_value_by_key(*v8::String::Utf8Value(args[0]));
                    return scope.Close(value ? v8::String::New(value) : v8::Undefined());
                }
        }

        return ThrowException(v8::Exception::TypeError(v8::String::New("call tags() without parameters or with a string (key)")));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_id(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).id()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_version(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).version()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_changeset(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).changeset()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_visible(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Boolean::New(wrapped(info.This()).visible()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_timestamp(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).timestamp()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_uid(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::Number::New(wrapped(info.This()).uid()));
    }

    v8::Handle<v8::Value> OSMObjectWrap::get_user(v8::Local<v8::String> /* property */, const v8::AccessorInfo& info) {
        v8::HandleScope scope;
        return scope.Close(v8::String::New(wrapped(info.This()).user()));
    }

} // namespace node_osmium