#pragma once

#include <algorithm>
#include <functional>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Event.hpp"

namespace ast {
    using events::Event;

class EventBus {
public:
    using SubscriptionId = std::size_t;
    using EventTypeId = std::size_t;

    template <typename T>
    using Handler = std::function<void(const T&)>;

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    /**
     * Get the singleton instance of EventBus.
     * @return The EventBus instance
     */
    static EventBus& getInstance() {
        static EventBus instance;
        return instance;
    }

    template <typename T>
    static EventTypeId getTypeId() {
        return getTypeIndexImpl<std::decay_t<T>>();
    }

    /**
     * Register a handler for a specific event type.
     * @tparam T The type of event to subscribe to
     * @param handler The handler to invoke when the event occurs
     * @return A Subscription ID that can be used to unsubscribe later
     */
    template <typename T>
    static SubscriptionId subscribe(const Handler<Event>& handler) {
        return getInstance().subscribeImpl(getTypeId<T>(), handler);
    }

    template <typename T>
    static SubscriptionId subscribe(Handler<Event>&& handler) {
        return getInstance().subscribeImpl(getTypeId<T>(), std::move(handler));
    }

    /**
     * Unsubscribe from a specific event type using a Subscription ID.
     * @tparam T The type of event to unsubscribe from
     * @param id The Subscription ID returned when subscribing
     */
    template <typename T>
    static void unsubscribe(SubscriptionId id) {
        getInstance().unsubscribeImpl(getTypeId<T>(), id);
    }

    /**
     * Publish an event to all subscribers.
     * @param event The event to publish
     */
    // template <typename T>
    // static void publish(const T& event) {
    //     getInstance().publishImpl(event);
    // }

    template <typename T, typename... Args>
    static void publish(Args&&... args) {
        getInstance().publishImpl(T(std::forward<Args>(args)...));
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.clear();
    }

private:
    EventBus() = default;

    template <typename T>
    static EventTypeId getTypeIndexImpl() {
        static const EventTypeId index = s_typeIndex++;
        return index;
    }

    SubscriptionId subscribeImpl(EventTypeId typeIndex, Handler<Event> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto id = s_id++;
        handlers_[typeIndex].emplace_back(id, std::move(handler));
        return id;
    }

    void unsubscribeImpl(EventTypeId typeIndex, SubscriptionId id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& vec = handlers_[typeIndex];
        auto it =
            std::find_if(vec.begin(), vec.end(), [id](const auto& p) { return p.first == id; });
        if (it != vec.end()) {
            vec.erase(it);
        }
    }

    template <typename T>
    void publishImpl(const T& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(getTypeId<T>());
        if (it != handlers_.end()) {
            // Iterate over all handlers for this event type
            for (auto& pair : it->second) {
                pair.second(event);
            }
        }
    }

    inline static SubscriptionId s_id = 0;
    inline static EventTypeId s_typeIndex = 0;

    // Map of event type ID to a vector of pairs (subscription ID, handler)
    std::unordered_map<EventTypeId, std::vector<std::pair<SubscriptionId, Handler<Event>>>>
        handlers_;
    std::mutex mutex_;
};

}  // namespace ast