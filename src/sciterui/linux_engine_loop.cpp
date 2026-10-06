#include "linux_engine_loop.h"
#include "sciter_hwindow.h"
#include "sciter_window.h"
#include <algorithm>
#include <vector>

namespace SciterUI
{
namespace
{
std::vector<std::shared_ptr<SciterWindow>> pendingCloses;
bool closing = false;
} // namespace

void QueueLinuxEngineClose(std::shared_ptr<SciterWindow> window)
{
    if (!window || window->GetHandle() == nullptr)
    {
        return;
    }
    const bool found = std::find(pendingCloses.begin(), pendingCloses.end(), window) != pendingCloses.end();
    if (!found)
    {
        pendingCloses.push_back(std::move(window));
    }
}

void FlushLinuxEngineCloses()
{
    if (closing)
    {
        return;
    }
    closing = true;
    while (!pendingCloses.empty())
    {
        auto windows = std::move(pendingCloses);
        pendingCloses.clear();
        for (const auto & window : windows)
        {
            if (window->GetHandle() != nullptr)
            {
                SciterWindowExec((SciterHWINDOW)window->GetHandle(), SCITER_WINDOW_SET_STATE, SCITER_WINDOW_STATE_CLOSED, TRUE);
            }
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
} // namespace SciterUI
