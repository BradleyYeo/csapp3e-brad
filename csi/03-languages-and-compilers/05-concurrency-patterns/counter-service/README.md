# Concurrent Counter Service Benchmark

An exploration of concurrency primitives in Go, comparing hardware atomics, mutex locks, and channel-based message passing under heavy multithreaded contention.

## Problem Statement

Implement a thread-safe counter service implementing the [CounterService](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/05-concurrency-patterns/counter-service/counter_service.go) interface:
```go
type CounterService interface {
    getNext() uint64
}
```
All implementations must return strictly monotonically increasing `uint64` values without duplicates or lost updates when invoked across 32 concurrent goroutines.

## Synchronization Architectures

- **Unsynchronized**: Baseline non-thread-safe counter suffering from data races under multi-goroutine access.
- **Hardware Atomics (`sync/atomic`)**: Lock-free fetch-and-add mapped directly to the hardware bus locking instruction (`LOCK XADDQ` on x86-64).
- **Mutex Locks (`sync.Mutex`)**: Mutual exclusion locks managed by the Go runtime, spinning briefly before parking goroutines via futex/scheduler semaphores.
- **Communicating Sequential Processes (`chan uint64`)**: Dedicated counter worker goroutine streaming monotonically increasing IDs over a buffered Go channel.

## Files

- [counter_service.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/05-concurrency-patterns/counter-service/counter_service.go): Starter template with `TODO` stubs.
- [counter_service_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/05-concurrency-patterns/counter-service/counter_service_test.go): Monotonicity correctness tests and contention benchmarks.
- [reference-solution/counter_service.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/05-concurrency-patterns/counter-service/reference-solution/counter_service.go): Fully verified reference implementations.

## Verification & Benchmarks

### Run Unit Tests
- Execute race-detector enabled test suite:
  ```bash
  go test -v -race
  ```

### Run Benchmarks
- Compare throughput under low vs high contention:
  ```bash
  go test -bench=. -benchmem
  ```
