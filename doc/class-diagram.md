# Service Recovery Scheduler — Class Diagram

## Overview

The scheduler registers services with ordered recovery action sequences. On failure, it delegates to `ServiceRecoveryState`, which selects and executes the next `IRecoveryAction` in the sequence.

---

## Class diagram

```
+---------------------------+         +-----------------------------+
|        Scheduler          | 1    *  |    ServiceRecoveryState     |
|---------------------------|-------->|-----------------------------|
| - services_               |  owns   | - actionSequence_           |
|---------------------------|         | - currentLevel_             |
| + RegisterService()       |         | - lastAction_               |
| + ReportFailure()         |         |-----------------------------|
| + GetState()              |         | + HandleFailure()           |
+-------------+-------------+         | + GetLastAction()           |
              |                       | + GetLevel()                |
              | returns               +-------------+---------------+
              v                                     |
+---------------------------+                       | 1..* actionSequence_
|      ServiceStatus        |                       v
|---------------------------|         +-----------------------------+
| + currentLevel            |         |  <<interface>>              |
| + lastActionName          |         |     IRecoveryAction         |
+---------------------------+         |-----------------------------|
                                      | + Execute()                 |
                                      | + GetName()                 |
                                      +-------------+---------------+
                                                    ^
                                      implements    |
                         +------------+-------------+-------------+
                         |            |                           |
                  +-------------+ +-------------+ +----------------+
                  | RestartAction| | StopAction  | | DisableAction |
                  |-------------| |-------------| |----------------|
                  | + Execute() | | + Execute() | | + Execute()    |
                  | + GetName() | | + GetName() | | + GetName()    |
                  +-------------+ +-------------+ +----------------+

  lastAction_ (0..1) points to one action already held in actionSequence_
```

---

## Classes

### Scheduler

Central registry. Maps each service name to its `ServiceRecoveryState`.

| Visibility | Member | Type / signature |
|---|---|---|
| private | `services_` | `std::unordered_map<std::string, ServiceRecoveryState>` |
| public | `RegisterService` | `(serviceName, actionSequence) → bool` |
| public | `ReportFailure` | `(serviceName) → std::optional<std::string>` |
| public | `GetState` | `(serviceName) → std::optional<ServiceStatus>` |

### ServiceStatus

Value object returned when querying service state.

| Visibility | Member | Type |
|---|---|---|
| public | `currentLevel` | `int` — index of last executed action; `-1` before first failure |
| public | `lastActionName` | `std::string` — empty when no failure has occurred yet |

### ServiceRecoveryState

Per-service escalation tracker. Holds the ordered action sequence and current position.

| Visibility | Member | Type |
|---|---|---|
| private | `actionSequence_` | `std::vector<std::shared_ptr<IRecoveryAction>>` |
| private | `currentLevel_` | `int` |
| private | `lastAction_` | `std::shared_ptr<IRecoveryAction>` |
| public | `HandleFailure` | `(serviceName) → IRecoveryAction&` |
| public | `GetLastAction` | `() → std::string` |
| public | `GetLevel` | `() → int` |

### IRecoveryAction (interface)

| Visibility | Method | Return type |
|---|---|---|
| public | `Execute` | `void` |
| public | `GetName` | `std::string` |

### RestartAction / StopAction / DisableAction

Concrete dummy implementations of `IRecoveryAction`. Each logs a message to stdout and returns a fixed name (`RESTART`, `STOP`, or `DISABLE`).

---

## Relationships

| From | To | UML | Description |
|---|---|---|---|
| `Scheduler` | `ServiceRecoveryState` | composition (1 → *) | One scheduler owns many per-service states, keyed by name |
| `Scheduler` | `ServiceStatus` | dependency | `GetState()` returns a snapshot value object |
| `ServiceRecoveryState` | `IRecoveryAction` | aggregation (1 → *) | `actionSequence_` — ordered list of shared actions |
| `ServiceRecoveryState` | `IRecoveryAction` | dependency (0..1) | `lastAction_` — pointer to the most recently executed action |
| `RestartAction` | `IRecoveryAction` | realization | Implements restart recovery |
| `StopAction` | `IRecoveryAction` | realization | Implements stop recovery |
| `DisableAction` | `IRecoveryAction` | realization | Implements disable recovery |

---

## Call flow (failure reported)

```
 Client          Scheduler           ServiceRecoveryState       IRecoveryAction
   |                 |                        |                       |
   | ReportFailure   |                        |                       |
   |---------------->|                        |                       |
   |                 | HandleFailure          |                       |
   |                 |----------------------->|                       |
   |                 |                        | Execute               |
   |                 |                        |---------------------->|
   |                 |                        |        done           |
   |                 |                        |<----------------------|
   |                 |     action ref         |                       |
   |                 |<-----------------------|                       |
   |  action name    |                        |                       |
   |<----------------|                        |                       |
```

**Steps:**

1. Client calls `Scheduler::ReportFailure(serviceName)`.
2. Scheduler looks up the service in `services_`.
3. Scheduler delegates to `ServiceRecoveryState::HandleFailure`.
4. State selects the next action index, executes it, and updates `currentLevel_` / `lastAction_`.
5. Scheduler returns the executed action name (`std::optional<std::string>`).

---

## C++ type reference (quick lookup)

| Method | Return type (C++) |
|---|---|
| `Scheduler::RegisterService` | `bool` |
| `Scheduler::ReportFailure` | `std::optional<std::string>` |
| `Scheduler::GetState` | `std::optional<ServiceStatus>` |
| `ServiceRecoveryState::HandleFailure` | `IRecoveryAction&` |
| `ServiceRecoveryState::GetLastAction` | `std::string` |
| `ServiceRecoveryState::GetLevel` | `int` |

---

## Example escalation

For `network-daemon` registered with `[RESTART, RESTART, STOP, DISABLE]`:

| Failure # | Action executed | Level after |
|---|---|---|
| — | (none yet) | -1 |
| 1 | RESTART | 0 |
| 2 | RESTART | 1 |
| 3 | STOP | 2 |
| 4 | DISABLE | 3 |
| 5+ | DISABLE (repeated) | 3 |
