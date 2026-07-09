#include "dashboard_poller.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>

#include "dashboard_field_profile.h"
#include "pmbus_reader.h"

namespace mfc_tool::core {
namespace {

constexpr int kMaxConsecutiveErrors = 5;
constexpr int kStaticRefreshCycles = 30;

void PutValue(DashboardSnapshot* snapshot, const std::wstring& id, const std::wstring& value) {
    DashboardValue item;
    item.value = value;
    item.status = L"OK";
    item.ok = true;
    (*snapshot).values[id] = item;
}

void PutError(DashboardSnapshot* snapshot, const std::wstring& id, const std::wstring& message) {
    DashboardValue item;
    item.value = L"N/A";
    item.status = message;
    item.ok = false;
    (*snapshot).values[id] = item;
}

void PutNotAvailable(DashboardSnapshot* snapshot, const std::wstring& id) {
    DashboardValue item;
    item.value = L"N/A";
    item.status = L"Not available in current rail mode";
    item.ok = false;
    (*snapshot).values[id] = item;
}

void PutNotAutoPolled(DashboardSnapshot* snapshot, const std::wstring& id) {
    DashboardValue item;
    item.value = L"N/A";
    item.status = L"Requires parameter; not auto-polled in V1";
    item.ok = false;
    (*snapshot).values[id] = item;
}

std::wstring AnsiToWide(const char* text) {
    if (text == nullptr) {
        return L"";
    }
    const int n = MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
    if (n <= 0) {
        return L"";
    }
    std::wstring out(static_cast<std::size_t>(n - 1), L'\0');
    MultiByteToWideChar(CP_ACP, 0, text, -1, out.data(), n);
    return out;
}

} // namespace

DashboardPoller::~DashboardPoller() {
    Stop();
}

bool DashboardPoller::Start(BridgeService* service, HWND notify_hwnd, const DashboardPollerSettings& settings) {
    if (service == nullptr || notify_hwnd == nullptr) {
        return false;
    }
    if (running_.load()) {
        return false;
    }
    if (worker_.joinable()) {
        worker_.join();
    }
    running_.store(true);

    cancel_requested_.store(false);
    worker_ = std::thread(&DashboardPoller::Run, this, service, notify_hwnd, settings);
    return true;
}

void DashboardPoller::Stop() {
    cancel_requested_.store(true);
    if (worker_.joinable()) {
        worker_.join();
    }
    running_.store(false);
}

void DashboardPoller::Run(BridgeService* service, HWND notify_hwnd, DashboardPollerSettings settings) {
    DashboardSnapshot cache;
    int cycle = 0;
    int consecutive_errors = 0;
    bool master_initialized = false;

    try {
        PostEvent(notify_hwnd, DashboardPollerEvent::Kind::Info, L"Initializing I2C master...");
        service->I2cMasterInit(settings.port, settings.sda_pin, settings.scl_pin, settings.baud);
        master_initialized = true;
        service->I2cMasterBusStatus(settings.port, true);

        PmbusReader reader(service);
        reader.SetSettings({settings.port, settings.addr7, settings.pec});
        try {
            PmbusReadResult vout_mode = reader.ReadByte(0x20u);
            if (!vout_mode.data.empty()) {
                reader.SetVoutMode(vout_mode.data[0]);
                PutValue(&cache, L"vout_mode", reader.FormatResult({L"vout_mode", L"VOUT_MODE", 0x20u, DashboardProtocol::ReadByte,
                                                                     DashboardDecode::RawByte, DashboardFieldKind::Static, 0, 0, L""},
                                                                    vout_mode));
            }
        } catch (const std::exception& e) {
            PutError(&cache, L"vout_mode", AnsiToWide(e.what()));
        }

        while (!cancel_requested_.load()) {
            DashboardSnapshot snapshot = cache;
            snapshot.running = true;
            snapshot.cycle = ++cycle;
            snapshot.consecutive_errors = consecutive_errors;
            GetLocalTime(&snapshot.timestamp);

            bool cycle_error = false;
            auto read_one = [&](const DashboardFieldDefinition& field, const std::wstring& id_override, bool critical) {
                const std::wstring id = id_override.empty() ? field.id : id_override;
                if (field.protocol == DashboardProtocol::NotAvailable) {
                    PutNotAutoPolled(&snapshot, id);
                    return;
                }
                try {
                    PutValue(&snapshot, id, reader.ReadFormatted(field));
                } catch (const std::exception& e) {
                    if (critical) {
                        cycle_error = true;
                    }
                    PutError(&snapshot, id, AnsiToWide(e.what()));
                }
            };

            const bool refresh_static = (cycle == 1) || ((cycle % kStaticRefreshCycles) == 0);
            if (refresh_static) {
                for (const DashboardFieldDefinition& field : DashboardMainFieldDefinitions()) {
                    if (field.kind == DashboardFieldKind::Static) {
                        read_one(field, L"", false);
                    }
                }
                for (const DashboardFieldDefinition& field : DashboardDetailFieldDefinitions()) {
                    read_one(field, L"", false);
                }
            }

            if (settings.multi_rail_pages) {
                std::uint8_t original_page = 0u;
                bool have_original_page = false;
                try {
                    PmbusReadResult page_result = reader.ReadByte(0x00u);
                    if (!page_result.data.empty()) {
                        original_page = page_result.data[0];
                        have_original_page = true;
                    }
                } catch (...) {
                }

                const struct Rail {
                    const wchar_t* suffix;
                    std::uint8_t page;
                } rails[] = {
                    {L"12", 0u},
                    {L"5", 1u},
                    {L"33", 2u},
                };

                for (const Rail& rail : rails) {
                    if (cancel_requested_.load()) {
                        break;
                    }
                    try {
                        reader.WriteByte(0x00u, rail.page);
                    } catch (const std::exception& e) {
                        cycle_error = true;
                        const std::wstring error = AnsiToWide(e.what());
                        PutError(&snapshot, std::wstring(L"voltage_") + rail.suffix, error);
                        PutError(&snapshot, std::wstring(L"current_") + rail.suffix, error);
                        PutError(&snapshot, std::wstring(L"watt_") + rail.suffix, error);
                        PutError(&snapshot, std::wstring(L"ovp_") + rail.suffix, error);
                        PutError(&snapshot, std::wstring(L"uvp_") + rail.suffix, error);
                        PutError(&snapshot, std::wstring(L"ocp_") + rail.suffix, error);
                        continue;
                    }

                    read_one({L"", L"READ_VOUT", 0x8Bu, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Dynamic, 0, 3, L"V"},
                             std::wstring(L"voltage_") + rail.suffix, true);
                    read_one({L"", L"READ_IOUT", 0x8Cu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"A"},
                             std::wstring(L"current_") + rail.suffix, true);
                    read_one({L"", L"READ_POUT", 0x96u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"W"},
                             std::wstring(L"watt_") + rail.suffix, true);
                    read_one({L"", L"VOUT_OV_FAULT_LIMIT", 0x40u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Dynamic, 0, 3, L"V"},
                             std::wstring(L"ovp_") + rail.suffix, true);
                    read_one({L"", L"VOUT_UV_FAULT_LIMIT", 0x44u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Dynamic, 0, 3, L"V"},
                             std::wstring(L"uvp_") + rail.suffix, true);
                    read_one({L"", L"IOUT_OC_FAULT_LIMIT", 0x46u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"A"},
                             std::wstring(L"ocp_") + rail.suffix, true);
                }

                if (have_original_page) {
                    try {
                        reader.WriteByte(0x00u, original_page);
                    } catch (...) {
                    }
                }
            } else {
                read_one({L"", L"READ_VOUT", 0x8Bu, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Dynamic, 0, 3, L"V"}, L"voltage_12", true);
                read_one({L"", L"READ_IOUT", 0x8Cu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"A"}, L"current_12", true);
                read_one({L"", L"READ_POUT", 0x96u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"W"}, L"watt_12", true);
                read_one({L"", L"VOUT_OV_FAULT_LIMIT", 0x40u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Dynamic, 0, 3, L"V"}, L"ovp_12", true);
                read_one({L"", L"VOUT_UV_FAULT_LIMIT", 0x44u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Dynamic, 0, 3, L"V"}, L"uvp_12", true);
                read_one({L"", L"IOUT_OC_FAULT_LIMIT", 0x46u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"A"}, L"ocp_12", true);
                PutNotAvailable(&snapshot, L"voltage_5");
                PutNotAvailable(&snapshot, L"current_5");
                PutNotAvailable(&snapshot, L"watt_5");
                PutNotAvailable(&snapshot, L"ovp_5");
                PutNotAvailable(&snapshot, L"uvp_5");
                PutNotAvailable(&snapshot, L"ocp_5");
                PutNotAvailable(&snapshot, L"voltage_33");
                PutNotAvailable(&snapshot, L"current_33");
                PutNotAvailable(&snapshot, L"watt_33");
                PutNotAvailable(&snapshot, L"ovp_33");
                PutNotAvailable(&snapshot, L"uvp_33");
                PutNotAvailable(&snapshot, L"ocp_33");
            }

            for (const DashboardFieldDefinition& field : DashboardMainFieldDefinitions()) {
                if (field.kind == DashboardFieldKind::Dynamic) {
                    read_one(field, L"", true);
                }
            }

            consecutive_errors = cycle_error ? consecutive_errors + 1 : 0;
            snapshot.consecutive_errors = consecutive_errors;
            cache = snapshot;
            PostEvent(notify_hwnd, DashboardPollerEvent::Kind::Snapshot, L"", &snapshot);

            if (consecutive_errors >= kMaxConsecutiveErrors) {
                PostEvent(notify_hwnd, DashboardPollerEvent::Kind::Error,
                          L"Auto-stopped after 5 consecutive polling errors.");
                break;
            }

            const int sleep_ms = std::clamp(settings.interval_ms, 100, 60000);
            for (int elapsed = 0; elapsed < sleep_ms && !cancel_requested_.load(); elapsed += 50) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    } catch (const std::exception& e) {
        PostEvent(notify_hwnd, DashboardPollerEvent::Kind::Error, AnsiToWide(e.what()));
    }

    if (master_initialized) {
        try {
            service->I2cDeinit(settings.port);
        } catch (...) {
        }
    }
    running_.store(false);
    PostEvent(notify_hwnd, DashboardPollerEvent::Kind::Stopped, L"Polling stopped.");
}

void DashboardPoller::PostEvent(HWND hwnd,
                                DashboardPollerEvent::Kind kind,
                                const std::wstring& message,
                                const DashboardSnapshot* snapshot) {
    if (hwnd == nullptr) {
        return;
    }
    auto* event = new DashboardPollerEvent();
    event->kind = kind;
    event->message = message;
    if (snapshot != nullptr) {
        event->snapshot = *snapshot;
    }
    if (::PostMessageW(hwnd, WM_DASHBOARD_POLLER_EVENT, 0, reinterpret_cast<LPARAM>(event)) == FALSE) {
        delete event;
    }
}

} // namespace mfc_tool::core
