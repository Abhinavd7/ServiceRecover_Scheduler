#include "irecoveryaction.h"
#include "scheduler.h"

#include <iostream>
#include <memory>
#include <vector>

int main() {
  Scheduler scheduler;

  auto restart = std::make_shared<RestartAction>();
  auto stop = std::make_shared<StopAction>();
  auto disable = std::make_shared<DisableAction>();

  scheduler.RegisterService("network-daemon", {restart, restart, stop, disable});
  scheduler.RegisterService("audio-service", {restart, stop});

  const bool duplicateRegistered =
      scheduler.RegisterService("network-daemon", {restart});
  std::cout << "Duplicate registration succeeded? " << std::boolalpha
            << duplicateRegistered << "\n\n";

  std::cout << "-- network-daemon failure sequence --\n";
  for (int i = 0; i < 5; ++i) {
    const auto action = scheduler.ReportFailure("network-daemon");
    std::cout << "Failure #" << (i + 1) << " -> executed: "
              << (action ? *action : "UNKNOWN_SERVICE") << '\n';
  }

  if (const auto status = scheduler.GetState("network-daemon")) {
    std::cout << "\nnetwork-daemon state -> level: " << status->currentLevel
              << ", last action: " << status->lastActionName << "\n\n";
  }

  std::cout << "-- audio-service failure sequence --\n";
  scheduler.ReportFailure("audio-service");
  if (const auto status = scheduler.GetState("audio-service")) {
    std::cout << "audio-service state -> level: " << status->currentLevel
              << ", last action: " << status->lastActionName << "\n\n";
  }

  const auto unknownFailure = scheduler.ReportFailure("nonexistent-service");
  std::cout << "Failure on unknown service -> "
            << (unknownFailure ? *unknownFailure : "UNKNOWN_SERVICE") << '\n';

  if (const auto status = scheduler.GetState("nonexistent-service")) {
    std::cout << "State query on unknown service -> level: "
              << status->currentLevel << ", last action: "
              << status->lastActionName << '\n';
  } else {
    std::cout << "State query on unknown service -> not registered\n";
  }

  return 0;
}