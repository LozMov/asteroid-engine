#pragma once

#include <type_traits>

#include "EventBus.hpp"

namespace ast {

template <typename T>
class EventSubscriber {
    static_assert(std::is_base_of_v<Event, T>, "EventSubscriber must be used with an Event type");

protected:
    EventSubscriber()
        : subscription_(EventBus::subscribe<T>(
              [this](const Event& event) { onEvent(static_cast<const T&>(event)); })) {}

    virtual ~EventSubscriber() { EventBus::unsubscribe<T>(subscription_); }

    /**
     * Called when an event of type T is received.
     * Implement this method to handle the event.
     * @param event The event to process.
     */
    virtual void onEvent(const T& event) = 0;

private:
    EventBus::SubscriptionId subscription_;
};

}  // namespace ast