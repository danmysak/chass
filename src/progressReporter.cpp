#include <ctime>
#include <vector>

#include "progressReporter.h"

ProgressReporter::ProgressReporter(void (*progressCallback)(const std::vector<std::pair<int, int>> &))
    : progressCallback(progressCallback), lastProgress(0) {}

void ProgressReporter::start() {
    reportProgress({{0, 0}}, true);
}

void ProgressReporter::reportProgress(const std::vector<std::pair<int, int>> &progress, bool force) {
    if (progressCallback != nullptr) {
        std::time_t currentTime = std::time(nullptr);
        if (force || currentTime != lastProgress) {
            progressCallback(progress);
            lastProgress = currentTime;
        }
    }
}

void ProgressReporter::end() {
    reportProgress({}, true);
}