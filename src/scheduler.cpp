#include "scheduler.h"

bool Scheduler::RegisterService(
    const std::string& serviceName,
    std::vector<std::shared_ptr<IRecoveryAction>> actionSequence) {
  if (actionSequence.empty()) {
    return false;
  }

  const auto [it, inserted] = services_.emplace(
      serviceName, ServiceRecoveryState(std::move(actionSequence)));
  return inserted;
}

std::optional<std::string> Scheduler::ReportFailure(const std::string& serviceName) {
  const auto it = services_.find(serviceName);
  if (it == services_.end()) {
    return std::nullopt;
  }

  return it->second.HandleFailure(serviceName).GetName();
}

std::optional<ServiceStatus> Scheduler::GetState(const std::string& serviceName) const {
  const auto it = services_.find(serviceName);
  if (it == services_.end()) {
    return std::nullopt;
  }

  return ServiceStatus{it->second.GetLevel(), it->second.GetLastAction()};
}
