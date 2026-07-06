#pragma once

#include "irecoveryaction.h"
#include <memory>
#include <string>
#include <vector>

class ServiceRecoveryState {
public:
  ServiceRecoveryState() = default;
  explicit ServiceRecoveryState(std::vector<std::shared_ptr<IRecoveryAction>> seq)
      : actionSequence_(std::move(seq)) {}

  ServiceRecoveryState(const ServiceRecoveryState&) = delete;
  ServiceRecoveryState& operator=(const ServiceRecoveryState&) = delete;
  ServiceRecoveryState(ServiceRecoveryState&&) = default;
  ServiceRecoveryState& operator=(ServiceRecoveryState&&) = default;

  IRecoveryAction& HandleFailure(const std::string& serviceName);
  std::string GetLastAction() const noexcept;
  int GetLevel() const noexcept;

private:
  std::vector<std::shared_ptr<IRecoveryAction>> actionSequence_;
  int currentLevel_ = -1;
  std::shared_ptr<IRecoveryAction> lastAction_;
};
