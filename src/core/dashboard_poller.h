#pragma once

#include <atomic>
#include <map>
#include <string>
#include <thread>

#include <windows.h>

#include "board_i2c_catalog.h"
#include "bridge_service.h"

namespace mfc_tool::core {

constexpr UINT WM_DASHBOARD_POLLER_EVENT = WM_APP + 101;

struct DashboardPollerSettings {
    int port = 0;
    int sda_pin = board_i2c::MakePin(board_i2c::kPortB, 4);
    int scl_pin = board_i2c::MakePin(board_i2c::kPortB, 5);
    int baud = 400000;
    int addr7 = 0x5A;
    int interval_ms = 1000;
    bool pec = true;
    bool multi_rail_pages = false;
};

struct DashboardValue {
    std::wstring value;
    std::wstring status;
    bool ok = true;
};

struct DashboardSnapshot {
    std::map<std::wstring, DashboardValue> values;
    SYSTEMTIME timestamp = {};
    int cycle = 0;
    int consecutive_errors = 0;
    bool running = false;
};

struct DashboardPollerEvent {
    enum class Kind {
        Snapshot = 0,
        Info,
        Error,
        Stopped,
    };

    Kind kind = Kind::Info;
    DashboardSnapshot snapshot;
    std::wstring message;
};

class DashboardPoller {
public:
    DashboardPoller() = default;
    ~DashboardPoller();

    DashboardPoller(const DashboardPoller&) = delete;
    DashboardPoller& operator=(const DashboardPoller&) = delete;

    bool Start(BridgeService* service, HWND notify_hwnd, const DashboardPollerSettings& settings);
    void Stop();
    [[nodiscard]] bool IsRunning() const noexcept { return running_.load(); }

private:
    void Run(BridgeService* service, HWND notify_hwnd, DashboardPollerSettings settings);
    void PostEvent(HWND hwnd, DashboardPollerEvent::Kind kind, const std::wstring& message, const DashboardSnapshot* snapshot = nullptr);

private:
    std::atomic_bool cancel_requested_ = false;
    std::atomic_bool running_ = false;
    std::thread worker_;
};

} // namespace mfc_tool::core
