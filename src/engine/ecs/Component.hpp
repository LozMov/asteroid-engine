#pragma once

#include <cstddef>

namespace ast {

class Component {
public:
    using TypeId = unsigned;

    virtual ~Component() = default;

    // Get the type ID for a component type using type tagging
    template <typename T>
    static TypeId getTypeId() {
        static TypeId typeId = s_typeId++;
        return typeId;
    }

protected:
    // Protected constructor to ensure components are created through derived classes
    Component() = default;

private:
    inline static TypeId s_typeId = 0;
};

}  // namespace ast