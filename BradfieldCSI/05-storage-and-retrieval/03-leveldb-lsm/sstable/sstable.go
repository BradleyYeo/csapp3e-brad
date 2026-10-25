package sstable

// Every entry in our Directory will be 16 bytes for the key.
// This is so we can have a fixed-size key length that's basicaly
// a prefix of the key.

//type SSTable struct {
//	filename string
//	dir      Directory
//}
//
//type Directory struct {
//	sparseKeys [][]byte
//	offsets    []int
//}

type SSTable struct {
	filename string
	entries  []*Entry
}

type Entry struct {
	keyLength   int32
	valueLength int32
	key         []byte
	value       []byte
}
