#pragma once

#include "Component.hpp"
#include "SystemBase.hpp"

namespace ast {

class Registry;

template <typename... Components>
class System : public SystemBase {
public:
    System(Registry& registry) : SystemBase(registry) {
        // Set the signature for this system based on the component types
        (signature_.set(Component::getTypeId<Components>()), ...);
    }
};

}  // namespace ast