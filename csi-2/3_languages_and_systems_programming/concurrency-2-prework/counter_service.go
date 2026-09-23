// Author: Patch Neranartkomol

package counterservice

type CounterService interface {
	// Returns values in ascending order; it should be safe to call
	// getNext() concurrently from multiple goroutines without any
	// additional synchronization on the caller's side.
	getNext() uint64
}

type UnsynchronizedCounterService struct {
	/* Please implement this struct and its getNext method */
	counter int
}

// getNext() - This one can be UNSAFE
func (counter *UnsynchronizedCounterService) getNext() uint64 {
	return uint64(counter.counter) + 1
}

type AtomicCounterService struct {
	/* Please implement this struct and its getNext method */
}

// getNext() with sync/atomic
func (counter *AtomicCounterService) getNext() uint64 {
	panic("getNext not implemented")
}

type MutexCounterService struct {
	/* Please implement this struct and its getNext method */
}

// getNext() with sync/Mutex
func (counter *MutexCounterService) getNext() uint64 {
	panic("getNext not implemented")
}

type ChannelCounterService struct {
	/* Please implement this struct and its getNext method */
}

// A constructor for ChannelCounterService
func newChannelCounterService() *ChannelCounterService {
	cs := ChannelCounterService{}
	return &cs
}

// getNext() with goroutines and channels
func (counter *ChannelCounterService) getNext() uint64 {
	panic("getNext not implemented")
}
