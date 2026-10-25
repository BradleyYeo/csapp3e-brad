package in_memory_leveldb

import (
	"bytes"
	"errors"

	"slices"

	"github.com/omarfq/in_memory_leveldb/iterator"
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

type Entry struct {
	Key   []byte
	Value []byte
}

type LevelDb struct {
	entries []Entry
}

func (ldb *LevelDb) Get(key []byte) ([]byte, error) {
	idx, found := slices.BinarySearchFunc(ldb.entries, key, func(entry Entry, target []byte) int {
		return bytes.Compare(entry.Key, target)
	})

	if found {
		return ldb.entries[idx].Value, nil
	}

	return nil, errors.New("Key not found")
}

func (ldb *LevelDb) Has(key []byte) (bool, error) {
	_, found := slices.BinarySearchFunc(ldb.entries, key, func(entry Entry, target []byte) int {
		return bytes.Compare(entry.Key, target)
	})

	return found, nil
}

func (ldb *LevelDb) Put(key, value []byte) error {
	idx, found := slices.BinarySearchFunc(ldb.entries, key, func(entry Entry, target []byte) int {
		return bytes.Compare(entry.Key, target)
	})

	newEntry := Entry{key, value}

	if found {
		ldb.entries[idx].Value = value
	} else {
		ldb.entries = append(ldb.entries, newEntry)
	}

	slices.SortFunc(ldb.entries, func(entry1, entry2 Entry) int {
		return bytes.Compare(entry1.Key, entry2.Key)
	})

	return nil

}

func (ldb *LevelDb) Delete(key []byte) error {
	idx, found := slices.BinarySearchFunc(ldb.entries, key, func(entry Entry, target []byte) int {
		return bytes.Compare(entry.Key, target)
	})

	if found {
		ldb.entries = append(ldb.entries[0:idx], ldb.entries[idx+1:]...)
		return nil
	}

	return errors.New("Could not delete Entry. Provided key not found")
}

func (ldb *LevelDb) RangeScan(start, end []byte) (iterator.Iterator, error) {
	idxStart, foundStart := slices.BinarySearchFunc(ldb.entries, start, func(entry Entry, target []byte) int {
		return bytes.Compare(entry.Key, target)
	})

	idxEnd, foundEnd := slices.BinarySearchFunc(ldb.entries, end, func(entry Entry, target []byte) int {
		return bytes.Compare(entry.Key, target)
	})

	if !foundStart || !foundEnd {
		return nil, errors.New("One or both of the provided range keys could not be found")
	}

	newIterator := iterator.NewIter()
	rangeSlice := ldb.entries[idxStart : idxEnd+1] // +2 to include the last range Entry in the Iterator

	for _, Entry := range rangeSlice {
		newTuple := iterator.Tuple{
			Key:   Entry.Key,
			Value: Entry.Value,
		}
		newIterator.Tuples = append(newIterator.Tuples, newTuple)
	}

	return newIterator, nil
}

func (ldb *LevelDb) Dump() (iterator.Iterator, error) {
	newIterator := iterator.NewIter()

	for _, Entry := range ldb.entries {
		newTuple := iterator.Tuple{
			Key:   Entry.Key,
			Value: Entry.Value,
		}
		newIterator.Tuples = append(newIterator.Tuples, newTuple)
	}

	return newIterator, nil
}
