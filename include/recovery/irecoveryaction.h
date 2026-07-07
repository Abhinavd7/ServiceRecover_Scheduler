#pragma once

#include <string>

class IRecoveryAction {
public:
  virtual ~IRecoveryAction() = default;

  IRecoveryAction() = default;
  IRecoveryAction(const IRecoveryAction&) = delete;
  IRecoveryAction& operator=(const IRecoveryAction&) = delete;
  IRecoveryAction(IRecoveryAction&&) = delete;
  IRecoveryAction& operator=(IRecoveryAction&&) = delete;

  virtual void Execute(const std::string& serviceName) const = 0;
  virtual std::string GetName() const = 0;
};

class RestartAction : public IRecoveryAction {
public:
  void Execute(const std::string& serviceName) const override;
  std::string GetName() const override;
};

class StopAction : public IRecoveryAction {
public:
  void Execute(const std::string& serviceName) const override;
  std::string GetName() const override;
};

class DisableAction : public IRecoveryAction {
public:
  void Execute(const std::string& serviceName) const override;
  std::string GetName() const override;
};
