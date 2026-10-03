#include "linux_engine_loop.h"
#include "sciter_window.h"
#include "sciter_hwindow.h"
#include <algorithm>
#include <vector>

namespace SciterUI
{
namespace
{
std::vector<std::shared_ptr<SciterWindow>> pendingCloses;
bool closing = false;
}

void QueueLinuxEngineClose(std::shared_ptr<SciterWindow> window)
{
    if (!window || window->GetHandle() == nullptr)
        return;
    const auto found = std::find_if(pendingCloses.begin(), pendingCloses.end(),
        [&](const auto & queued) { return queued.get() == window.get(); });
    if (found == pendingCloses.end())
        pendingCloses.push_back(std::move(window));
}

void FlushLinuxEngineCloses()
{
    if (closing)
        return;
    closing = true;
    // A close callback may queue another window. Consume each batch after
    // the native iteration has fully returned, retaining strong references.
    while (!pendingCloses.empty())
    {
        auto windows = std::move(pendingCloses);
        pendingCloses.clear();
        for (const auto & window : windows)
        {
            if (window->GetHandle() != nullptr)
                SciterWindowExec((SciterHWINDOW)window->GetHandle(),
                    SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_CLOSED, TRUE);
        }
    }
    closing = false;
}

bool LinuxEngineLoopIteration()
{
    const bool running = SciterExec(SCITER_APP_LOOP_ITERATION, 0, 0) != 0;
    FlushLinuxEngineCloses();
    return running;
}
}
