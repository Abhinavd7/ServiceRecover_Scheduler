#pragma once

#include "irecoveryaction.h"
#include "servicerecoverystate.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct ServiceStatus {
  int currentLevel = -1;
  std::string lastActionName;
};

class Scheduler {
public:
  Scheduler() = default;
  ~Scheduler() = default;

  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  Scheduler(Scheduler&&) = default;
  Scheduler& operator=(Scheduler&&) = default;

  bool RegisterService(const std::string& serviceName,
                       std::vector<std::shared_ptr<IRecoveryAction>> actionSequence);

  std::optional<std::string> ReportFailure(const std::string& serviceName);
  std::optional<ServiceStatus> GetState(const std::string& serviceName) const;

private:
  std::unordered_map<std::string, ServiceRecoveryState> services_;
};
