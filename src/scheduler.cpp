#include "scheduler.h"

bool Scheduler::RegisterService(
    const std::string& serviceName,
    std::vector<std::shared_ptr<IRecoveryAction>> actionSequence) {
 // to implement
    return false;

}

std::optional<std::string> Scheduler::ReportFailure(const std::string& serviceName) {
  // to implement
 return std::nullopt;
}

std::optional<ServiceStatus> Scheduler::GetState(const std::string& serviceName) const {
    // to implement
 return std::nullopt;
}
