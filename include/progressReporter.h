#ifndef CHASS_PROGRESS_REPORTER_H
#define CHASS_PROGRESS_REPORTER_H

#include <ctime>
#include <vector>

class ProgressReporter {
    void (*progressCallback)(const std::vector<std::pair<int, int>> &);
    time_t lastProgress;

public:
    explicit ProgressReporter(void (*progressCallback)(const std::vector<std::pair<int, int>> &));
    void start();
    void reportProgress(const std::vector<std::pair<int, int>> &progress, bool force = false);
    void end();
};

#endif // CHASS_PROGRESS_REPORTER_H