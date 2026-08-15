// Platform/PlatformMsg.cpp — Implementación del NppMsgBus (Linux/Qt6)
// Copyright (C) Notepad++ contributors. GPL v3+

#include "PlatformMsg.h"

#ifdef NPP_PLATFORM_LINUX

#include <QMetaObject>
#include <QCoreApplication>
#include <Qt>

// ─── NppMsgBus ───────────────────────────────────────────────────────────────

NppMsgBus& NppMsgBus::instance() {
    static NppMsgBus bus;
    return bus;
}

int NppMsgBus::subscribe(NppMsg msg, NppMsgHandler handler) {
    std::lock_guard<std::mutex> lock(_mutex);
    int id = _nextId++;
    _handlers[static_cast<uint32_t>(msg)].push_back({id, std::move(handler)});
    return id;
}

void NppMsgBus::unsubscribe(NppMsg msg, int handlerId) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto key = static_cast<uint32_t>(msg);
    auto it = _handlers.find(key);
    if (it == _handlers.end()) return;
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [handlerId](const HandlerEntry& e){ return e.id == handlerId; }),
        vec.end());
}

NppLresult NppMsgBus::send(NppMsg msg, NppWparam wp, NppLparam lp) {
    // Snapshot para evitar deadlock si el handler llama a subscribe/unsubscribe
    std::vector<HandlerEntry> snapshot;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _handlers.find(static_cast<uint32_t>(msg));
        if (it != _handlers.end())
            snapshot = it->second;
    }
    NppLresult result = 0;
    for (auto& entry : snapshot)
        result = entry.fn(wp, lp);
    return result;
}

void NppMsgBus::post(NppMsg msg, NppWparam wp, NppLparam lp) {
    // Equivalente a PostMessage en Win32:
    // Encola la ejecución en el event loop de Qt (main thread)
    QMetaObject::invokeMethod(
        QCoreApplication::instance(),
        [this, msg, wp, lp]() {
            this->send(msg, wp, lp);
        },
        Qt::QueuedConnection
    );
}

#endif // NPP_PLATFORM_LINUX
