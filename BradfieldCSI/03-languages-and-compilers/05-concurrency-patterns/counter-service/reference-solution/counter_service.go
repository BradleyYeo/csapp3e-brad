package counterservice

import (
	"sync"
	"sync/atomic"
)

type CounterService interface {
	getNext() uint64
}

type UnsynchronizedCounterService struct {
	counter uint64
}

func (c *UnsynchronizedCounterService) getNext() uint64 {
	c.counter++
	return c.counter
}

type AtomicCounterService struct {
	counter uint64
}

func (c *AtomicCounterService) getNext() uint64 {
	return atomic.AddUint64(&c.counter, 1)
}

type MutexCounterService struct {
	mu      sync.Mutex
	counter uint64
}

func (c *MutexCounterService) getNext() uint64 {
	c.mu.Lock()
	defer c.mu.Unlock()
	c.counter++
	return c.counter
}

type ChannelCounterService struct {
	ch chan uint64
}

func newChannelCounterService() *ChannelCounterService {
	cs := &ChannelCounterService{
		ch: make(chan uint64, 1000),
	}
	go func() {
		var i uint64 = 1
		for {
			cs.ch <- i
			i++
		}
	}()
	return cs
}

func (c *ChannelCounterService) getNext() uint64 {
	return <-c.ch
}
