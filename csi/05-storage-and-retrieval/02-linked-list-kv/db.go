package linked_list_leveldb

import (
	"bytes"
	"errors"
	"fmt"

	"github.com/omarfq/linked_list_leveldb/iterator"
)

type DB interface {
	// Get gets the value for the given key. It returns an error if the
	// DB does not contain the key.
	Get(key []byte) (value []byte, err error)

	// Has returns true if the DB contains the given key.
	Has(key []byte) (ret bool, err error)

	// Put sets the value for the given key. It overwrites any previous value
	// for that key; a DB is not a multi-map.
	Put(key, value []byte) error

	// Delete deletes the value for the given key.
	Delete(key []byte) error

	// RangeScan returns an Iterator (see below) for scanning through all
	// key-value pairs in the given range, ordered by key ascending.
	RangeScan(start, limit []byte) (iterator.Iterator, error)

	// Dump returns all the records in the database in a readable format
	Dump() (iterator.Iterator, error)
}

type LevelDb struct {
	entries *iterator.Node
}

func NewNode(key, val []byte) *LevelDb {
	newNode := &iterator.Node{
		Key:   key,
		Value: val,
		Next:  nil,
	}
	return &LevelDb{
		entries: newNode,
	}
}

func (ldb *LevelDb) Get(key []byte) ([]byte, error) {
	curr := ldb.entries
	for curr != nil {
		if bytes.Equal(curr.Key, key) {
			return curr.Value, nil
		}
		curr = curr.Next
	}

	return nil, errors.New("Key not found")
}

func (ldb *LevelDb) Has(key []byte) (bool, error) {
	curr := ldb.entries
	for curr != nil {
		if bytes.Equal(curr.Key, key) {
			return true, nil
		}
		curr = curr.Next
	}

	return false, nil
}

func (ldb *LevelDb) Put(key, value []byte) error {
	newEntry := &iterator.Node{
		Key:   key,
		Value: value,
	}

	// Special case for the head end
	if ldb.entries == nil || bytes.Compare(ldb.entries.Key, key) > 0 {
		newEntry.Next = ldb.entries
		ldb.entries = newEntry
		return nil
	}

	// Initialize current and previous nodes
	curr := ldb.entries.Next
	prev := ldb.entries

	// Traverse the list to find the correct spot for insertion
	for curr != nil {
		if bytes.Equal(curr.Key, key) {
			curr.Value = value // Update the value if key is found
			return nil
		} else if bytes.Compare(curr.Key, key) > 0 {
			newEntry.Next = curr
			prev.Next = newEntry
			return nil
		}
		prev = curr
		curr = curr.Next
	}

	prev.Next = newEntry

	return nil
}

func (ldb *LevelDb) Delete(key []byte) error {
	if ldb.entries == nil {
		return errors.New("list is empty")
	}

	// Handle the case where the key is at the beginning of the list
	if bytes.Equal(ldb.entries.Key, key) {
		ldb.entries = ldb.entries.Next
		return nil
	}

	prev := ldb.entries
	curr := ldb.entries.Next

	for curr != nil {
		if bytes.Equal(curr.Key, key) {
			prev.Next = curr.Next
			return nil
		}
		prev = curr
		curr = curr.Next
	}

	return errors.New("key not found")
}

func (ldb *LevelDb) RangeScan(start, end []byte) (iterator.Iterator, error) {
	var startNode, endNode *iterator.Node
	current := ldb.entries

	// Find start node
	for current != nil && bytes.Compare(current.Key, start) < 0 {
		current = current.Next
	}
	if current == nil {
		return nil, errors.New("start key not found")
	}
	startNode = current

	// Find end node
	for current != nil && bytes.Compare(current.Key, end) <= 0 {
		endNode = current
		current = current.Next
	}

	if startNode == nil || endNode == nil {
		return nil, errors.New("invalid range")
	}

	return iterator.NewIterator(startNode, endNode.Next), nil
}

func (ldb *LevelDb) PrintList() {
	curr := ldb.entries
	for curr != nil {
		fmt.Printf("Key: %s, Value: %s\n", curr.Key, curr.Value)
		curr = curr.Next
	}
}

//func (ldb *LevelDb) Dump() (iterator.Iterator, error) {
//	newIterator := iterator.NewIter()
//
//	for _, Entry := range ldb.entries {
//		newTuple := iterator.Tuple{
//			Key:   Entry.Key,
//			Value: Entry.Value,
//		}
//		newIterator.Tuples = append(newIterator.Tuples, newTuple)
//	}
//
//	return newIterator, nil
//}
