package iterator

import (
	"errors"

	"github.com/omarfq/leveldb/skiplist"
)

type SkipListIter struct {
	current *skiplist.SkipListNode
	err     error
}

func NewSkipListIterator(startNode *skiplist.SkipListNode) *SkipListIter {
	return &SkipListIter{current: startNode}
}

func (iter *SkipListIter) Next() bool {
	if iter.current != nil && iter.current.Forwards[0] != nil {
		iter.current = iter.current.Forwards[0]
		return true
	}
	if iter.current == nil {
		iter.err = errors.New("Iterator has reached end")
	}
	return false
}

func (iter *SkipListIter) Key() []byte {
	if iter.current != nil {
		return iter.current.Key
	}
	return nil
}

func (iter *SkipListIter) Value() []byte {
	if iter.current != nil {
		return iter.current.Value
	}
	return nil
}

func (iter *SkipListIter) Error() error {
	return iter.err
}
