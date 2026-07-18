#include <circle/sched/scheduler.h>
#include <SDL2/SDL.h>

void HostUpdate();

CScheduler::CScheduler() {}

void CScheduler::Yield() {
    HostUpdate();
}

void CScheduler::MsSleep(unsigned nMs) {
    unsigned start = SDL_GetTicks();
    while (SDL_GetTicks() - start < nMs) {
        HostUpdate();
        SDL_Delay(1);
    }
}
