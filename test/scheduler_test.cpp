#include "recovery/irecoveryaction.h"
#include "recovery/scheduler.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace {

class MockAction : public IRecoveryAction {
public:
  explicit MockAction(std::string name) : name_(std::move(name)) {}

  void Execute(const std::string&) const override {}
  std::string GetName() const override { return name_; }

private:
  std::string name_;
};

std::vector<std::shared_ptr<IRecoveryAction>> MakeSequence(
    std::initializer_list<std::string> names) {
  std::vector<std::shared_ptr<IRecoveryAction>> sequence;
  sequence.reserve(names.size());
  for (const auto& name : names) {
    sequence.push_back(std::make_shared<MockAction>(name));
  }
  return sequence;
}

}  // namespace

TEST(SchedulerTest, RegistersServiceWithRecoverySequence) {
  Scheduler scheduler;
  EXPECT_TRUE(scheduler.RegisterService("svc-a", MakeSequence({"RESTART", "STOP"})));
}

TEST(SchedulerTest, RejectsDuplicateRegistration) {
  Scheduler scheduler;
  EXPECT_TRUE(scheduler.RegisterService("svc-a", MakeSequence({"RESTART"})));
  EXPECT_FALSE(scheduler.RegisterService("svc-a", MakeSequence({"STOP"})));
}

TEST(SchedulerTest, RejectsEmptyRecoverySequence) {
  Scheduler scheduler;
  EXPECT_FALSE(scheduler.RegisterService("svc-a", {}));
}

TEST(SchedulerTest, EscalatesThroughRecoverySequence) {
  Scheduler scheduler;
  ASSERT_TRUE(scheduler.RegisterService(
      "svc-a", MakeSequence({"RESTART", "RESTART", "STOP", "DISABLE"})));

  EXPECT_EQ(scheduler.ReportFailure("svc-a"), "RESTART");
  EXPECT_EQ(scheduler.ReportFailure("svc-a"), "RESTART");
  EXPECT_EQ(scheduler.ReportFailure("svc-a"), "STOP");
  EXPECT_EQ(scheduler.ReportFailure("svc-a"), "DISABLE");
}

TEST(SchedulerTest, RepeatsLastActionAfterSequenceIsExhausted) {
  Scheduler scheduler;
  ASSERT_TRUE(scheduler.RegisterService("svc-a", MakeSequence({"RESTART", "STOP"})));

  scheduler.ReportFailure("svc-a");
  scheduler.ReportFailure("svc-a");
  EXPECT_EQ(scheduler.ReportFailure("svc-a"), "STOP");
  EXPECT_EQ(scheduler.ReportFailure("svc-a"), "STOP");
}

TEST(SchedulerTest, TracksCurrentLevelAsLastExecutedActionIndex) {
  Scheduler scheduler;
  ASSERT_TRUE(scheduler.RegisterService("svc-a", MakeSequence({"RESTART", "STOP"})));

  const auto initialState = scheduler.GetState("svc-a");
  ASSERT_TRUE(initialState.has_value());
  EXPECT_EQ(initialState->currentLevel, -1);
  EXPECT_TRUE(initialState->lastActionName.empty());

  scheduler.ReportFailure("svc-a");
  const auto afterFirstFailure = scheduler.GetState("svc-a");
  ASSERT_TRUE(afterFirstFailure.has_value());
  EXPECT_EQ(afterFirstFailure->currentLevel, 0);
  EXPECT_EQ(afterFirstFailure->lastActionName, "RESTART");

  scheduler.ReportFailure("svc-a");
  const auto afterSecondFailure = scheduler.GetState("svc-a");
  ASSERT_TRUE(afterSecondFailure.has_value());
  EXPECT_EQ(afterSecondFailure->currentLevel, 1);
  EXPECT_EQ(afterSecondFailure->lastActionName, "STOP");
}

TEST(SchedulerTest, KeepsServiceStateIndependent) {
  Scheduler scheduler;
  ASSERT_TRUE(scheduler.RegisterService("svc-a", MakeSequence({"RESTART", "STOP"})));
  ASSERT_TRUE(scheduler.RegisterService("svc-b", MakeSequence({"DISABLE"})));

  scheduler.ReportFailure("svc-a");
  scheduler.ReportFailure("svc-a");

  const auto stateA = scheduler.GetState("svc-a");
  const auto stateB = scheduler.GetState("svc-b");
  ASSERT_TRUE(stateA.has_value());
  ASSERT_TRUE(stateB.has_value());

  EXPECT_EQ(stateA->currentLevel, 1);
  EXPECT_EQ(stateA->lastActionName, "STOP");
  EXPECT_EQ(stateB->currentLevel, -1);
  EXPECT_TRUE(stateB->lastActionName.empty());
}

TEST(SchedulerTest, ReturnsNulloptForUnknownService) {
  Scheduler scheduler;
  EXPECT_FALSE(scheduler.ReportFailure("missing"));
  EXPECT_FALSE(scheduler.GetState("missing").has_value());
}
