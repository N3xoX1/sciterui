#pragma once
#include <memory>

namespace SciterUI
{
class SciterWindow;
void QueueLinuxEngineClose(std::shared_ptr<SciterWindow> window);
void FlushLinuxEngineCloses();
bool LinuxEngineLoopIteration();
} // namespace SciterUI
