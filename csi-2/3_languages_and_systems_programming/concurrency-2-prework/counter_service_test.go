// Author: Patch Neranartkomol

package counterservice

import (
	"runtime"
	"sync"
	"testing"
)

const (
	NUM_GOROUTINES      = 32
	CALLS_PER_GOROUTINE = 100000
)

func getNextMonotonicityChecker(counter CounterService, t *testing.T) {
	var prev uint64
	for i := 0; i < CALLS_PER_GOROUTINE; i++ {
		value := counter.getNext()
		if value <= prev {
			t.Fatalf("Values were NOT monotonically increasing; value: %d, prev: %d", value, prev)
		}
		prev = value
	}
}

func TestSynchronizedCounterServices(t *testing.T) {
	var wg sync.WaitGroup
	counters := []CounterService{
		&AtomicCounterService{},
		&MutexCounterService{},
		newChannelCounterService(),
	}
	for _, counter := range counters {
		for i := 0; i < NUM_GOROUTINES; i++ {
			wg.Add(1)
			go func() {
				defer wg.Done()
				getNextMonotonicityChecker(counter, t)
			}()
		}
		wg.Wait()
		nextVal := counter.getNext()
		if nextVal != (NUM_GOROUTINES*CALLS_PER_GOROUTINE)+1 {
			t.Errorf("Counter ID does not match total calls; nextVal: %d", nextVal)
		}
	}
}

// This test only checks that the unsynchronized version is correct when run in a single goroutine.
// It does not spawn additional goroutines.
func TestUnsynchronizedCounterService(t *testing.T) {
	var counter CounterService = &UnsynchronizedCounterService{}
	getNextMonotonicityChecker(counter, t)
	nextVal := counter.getNext()
	if nextVal != CALLS_PER_GOROUTINE+1 {
		t.Fatalf("Counter ID does not match total calls; nextVal: %d", nextVal)
	}
}

func BenchmarkCounterServices(b *testing.B) {
	for _, testCase := range []struct {
		name    string
		counter CounterService
	}{
		{"unsynchronized", &UnsynchronizedCounterService{}},
		{"atomic", &AtomicCounterService{}},
		{"mutex", &MutexCounterService{}},
		{"channel", newChannelCounterService()},
	} {
		b.Run(testCase.name, func(b *testing.B) {
			for n := 0; n < b.N; n++ {
				testCase.counter.getNext()
			}
		})
	}
}

func BenchmarkCounterServicesContended(b *testing.B) {
	b.SetParallelism(NUM_GOROUTINES / runtime.GOMAXPROCS(0))
	for _, testCase := range []struct {
		name    string
		counter CounterService
	}{
		{"unsynchronized", &UnsynchronizedCounterService{}},
		{"atomic", &AtomicCounterService{}},
		{"mutex", &MutexCounterService{}},
		{"channel", newChannelCounterService()},
	} {
		b.Run(testCase.name, func(b *testing.B) {
			b.RunParallel(func(pb *testing.PB) {
				for pb.Next() {
					testCase.counter.getNext()
				}
			})
		})
	}
}
