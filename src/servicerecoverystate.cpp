#include "servicerecoverystate.h"

#include <algorithm>

IRecoveryAction& ServiceRecoveryState::HandleFailure(const std::string& serviceName) {
  const int maxIndex = static_cast<int>(actionSequence_.size()) - 1;
  const int nextIndex = std::min(currentLevel_ + 1, maxIndex);
  currentLevel_ = nextIndex;

  auto action = actionSequence_[nextIndex];
  lastAction_ = action;
  action->Execute(serviceName);
  return *action;
}

std::string ServiceRecoveryState::GetLastAction() const noexcept {
  return lastAction_ ? lastAction_->GetName() : "";
}

int ServiceRecoveryState::GetLevel() const noexcept {
  return currentLevel_;
}
