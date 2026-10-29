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

type Node struct {
	Key   []byte
	Value []byte
	Next  *Node
}

type Iter struct {
	current *Node
	end     *Node
}

func NewIterator(start, end *Node) *Iter {
	return &Iter{
		current: start,
		end:     end,
	}
}

func (it *Iter) Next() bool {
	if it.current == nil || it.current == it.end {
		return false
	}
	it.current = it.current.Next
	return it.current != nil && it.current != it.end
}

func (it *Iter) Error() error {
	return nil
}

func (it *Iter) Key() []byte {
	if it.current == nil {
		return nil
	}
	return it.current.Key
}

func (it *Iter) Value() []byte {
	if it.current == nil {
		return nil
	}
	return it.current.Value
}
