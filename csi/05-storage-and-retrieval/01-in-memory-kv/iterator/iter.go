package iterator

type Iterator interface {
	// Next moves the iterator to the next key/value pair.
	// It returns false if the iterator is exhausted.
	Next() bool

	// Error returns any accumulated error. Exhausting all the key/value pairs
	// is not considered to be an error.
	Error() error

	// Key returns the key of the current key/value pair, or nil if done.
	Key() []byte

	// Value returns the value of the current key/value pair, or nil if done.
	Value() []byte
}

type Tuple struct {
	Key   []byte
	Value []byte
}

type Iter struct {
	Tuples  []Tuple
	CurrIdx int
}

func NewIter() *Iter {
	return &Iter{
		Tuples:  make([]Tuple, 0),
		CurrIdx: -1,
	}
}

func (it *Iter) Next() bool {
	if it.CurrIdx < len(it.Tuples) {
		it.CurrIdx++
	}
	return it.CurrIdx < len(it.Tuples)
}

func (i *Iter) Error() error {
	return nil
}

func (it *Iter) Key() []byte {
	return it.Tuples[it.CurrIdx].Key
}

func (it *Iter) Value() []byte {
	return it.Tuples[it.CurrIdx].Value
}
