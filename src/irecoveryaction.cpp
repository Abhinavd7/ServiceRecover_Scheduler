#include "irecoveryaction.h"

#include <iostream>

void RestartAction::Execute(const std::string& serviceName) const {
  std::cout << "[RESTART] " << serviceName << '\n';
}

std::string RestartAction::GetName() const {
  return "RESTART";
}

void StopAction::Execute(const std::string& serviceName) const {
  std::cout << "[STOP] " << serviceName << '\n';
}

std::string StopAction::GetName() const {
  return "STOP";
}

void DisableAction::Execute(const std::string& serviceName) const {
  std::cout << "[DISABLE] " << serviceName << '\n';
}

std::string DisableAction::GetName() const {
  return "DISABLE";
}
