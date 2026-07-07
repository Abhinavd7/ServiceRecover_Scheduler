# Service Recovery Scheduler

A small C++ library that manages per-service recovery action sequences. When a monitored service reports a failure, the scheduler executes the next action in that service's configured escalation ladder.

## Features

- Register services with ordered recovery sequences at startup
- Dispatch the appropriate recovery action on failure notifications
- Query per-service state (current escalation level and last action taken)
- Three built-in dummy actions: `RESTART`, `STOP`, `DISABLE`

## Build

Requires CMake 3.15+ and a C++17 compiler (Linux).

```bash
cmake -S . -B build
cmake --build build
```

This produces:

| Artifact | Path (typical) |
|---|---|
| Shared library | `build/librecovery_scheduler.so` |
| Demo executable | `build/recovery_demo` |
| Test executable | `build/tests/recovery_scheduler_tests` |

### Runtime library path (`LD_LIBRARY_PATH`)

The demo and tests link against the shared library at **runtime**. If the loader cannot find `librecovery_scheduler.so`, set `LD_LIBRARY_PATH` to the build directory:

```bash
export LD_LIBRARY_PATH="${PWD}/build:${LD_LIBRARY_PATH}"
```

Run the demo:

```bash
./build/recovery_demo
```

Run unit tests:

```bash
ctest --test-dir build --output-on-failure
```

Or run the test binary directly:

```bash
./build/tests/recovery_scheduler_tests
```

CMake also sets `BUILD_RPATH` so binaries built in-tree often work without exporting `LD_LIBRARY_PATH`. If you copy `recovery_demo` elsewhere without the `.so`, you must either place `librecovery_scheduler.so` next to it or set `LD_LIBRARY_PATH` accordingly.

## Project layout

```
include/recovery/   Public headers
src/                Library and demo implementation
tests/              GoogleTest unit tests
doc/                Architecture documentation (class diagram)
```

- `recovery_scheduler` — shared library (`librecovery_scheduler.so`) with scheduler and action implementations
- `recovery_demo` — minimal executable demonstrating registration, failures, and state queries
- `recovery_scheduler_tests` — unit test suite

## Documentation

See [`doc/class-diagram.md`](doc/class-diagram.md) for the ASCII class diagram, relationships, failure call flow, and per-class member reference.

## API overview

```cpp
Scheduler scheduler;

scheduler.RegisterService("network-daemon", {restart, restart, stop, disable});
auto action = scheduler.ReportFailure("network-daemon");  // std::optional<std::string>
auto state = scheduler.GetState("network-daemon");        // std::optional<ServiceStatus>
```

- `RegisterService` returns `false` for duplicate names or empty sequences.
- `ReportFailure` returns the executed action name, or `std::nullopt` if the service is unknown.
- `GetState` returns `std::nullopt` for unknown services.

### Level semantics

`ServiceStatus::currentLevel` is the **0-based index of the last executed action** in the service's sequence. It is `-1` before the first failure. After exhausting the sequence, further failures repeat the final action and the level stays at the last index.

## Design decisions

| Decision | Rationale |
|---|---|
| `Scheduler` owns a map of `ServiceRecoveryState` | Keeps registration/routing separate from per-service escalation logic |
| `IRecoveryAction` strategy interface | New recovery actions are added without changing scheduler code |
| `SHARED` library (`.so`) | Separates reusable logic from demo/tests; typical for C++ service libraries on Linux |
| `shared_ptr` action instances | Stateless dummy actions can be reused across multiple services |
| `std::optional` for errors | Avoids overloading action names with sentinel strings like `"UNKNOWN_SERVICE"` |
| Repeat final action when exhausted | Assumes the harshest recovery step should be retried on continued failure |

## Assumptions

- Recovery actions are stateless and safe to share between services
- Failures are reported synchronously; no threading model is implemented
- There is no automatic reset when a service recovers — escalation only moves forward on failure
- Action implementations are dummies (log to stdout); no real system calls are made

## Example escalation

For `network-daemon` registered with `[RESTART, RESTART, STOP, DISABLE]`:

| Failure # | Action executed | Level after |
|---|---|---|
| 1 | RESTART | 0 |
| 2 | RESTART | 1 |
| 3 | STOP | 2 |
| 4 | DISABLE | 3 |
| 5+ | DISABLE | 3 |
