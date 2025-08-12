#pragma once

#include "EventBus.hpp"

namespace ast {

template <typename T>
class EventSubscriber {
protected:
    EventSubscriber()
        : subscription_(EventBus::subscribe<T>(
              [this](const void* event) { onEvent(*static_cast<const T*>(event)); })) {}

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