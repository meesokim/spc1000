#include <circle/sched/scheduler.h>
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

void HostUpdate();

static auto g_next_frame_time = std::chrono::steady_clock::now();
static bool g_timing_initialized = false;

CScheduler::CScheduler() {}

void CScheduler::Yield() {
    HostUpdate();
    g_timing_initialized = false;
}

void CScheduler::MsSleep(unsigned nMs) {
    if (!g_timing_initialized) {
        g_next_frame_time = std::chrono::steady_clock::now();
        g_timing_initialized = true;
    }

    g_next_frame_time += std::chrono::microseconds(nMs * 1000);
    auto now = std::chrono::steady_clock::now();

    // If we are too far behind (e.g. startup stalls or slow renders), resync smoothly
    if (now > g_next_frame_time + std::chrono::milliseconds(40)) {
        g_next_frame_time = now;
        HostUpdate();
        return;
    }

    // Call HostUpdate (SDL event polling and screen presentation) at ~60fps (every 16.6ms)
    static auto last_host_update = now;
    if (now - last_host_update >= std::chrono::microseconds(16666)) {
        HostUpdate();
        last_host_update = now;
    }

    // Low-overhead sleep: yield to OS when waiting >= 2ms, spin only for last < 1ms
    now = std::chrono::steady_clock::now();
    if (g_next_frame_time > now) {
        auto diff_us = std::chrono::duration_cast<std::chrono::microseconds>(g_next_frame_time - now).count();
        if (diff_us >= 2000) {
            std::this_thread::sleep_for(std::chrono::microseconds(diff_us - 1000));
        }
        while (std::chrono::steady_clock::now() < g_next_frame_time) {
            #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
            __builtin_ia32_pause();
            #endif
        }
    }
}
