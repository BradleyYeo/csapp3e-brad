package main

import (
	"fmt"

	"github.com/omarfq/leveldb"
)

func main() {
	leveldb, err := leveldb.NewLevelDb(nil, nil)
	if err != nil {
		fmt.Printf("Error starting LevelDb: %s\n", err)
		return
	}

	data := []struct {
		key   []byte
		value []byte
	}{
		{[]byte("alpha"), []byte("Alpha")},
		{[]byte("bravo"), []byte("Bravo")},
		{[]byte("foxtrot"), []byte("Foxtrot")},
		{[]byte("charlie"), []byte("Charlie")},
		{[]byte("delta"), []byte("Delta")},
		{[]byte("echo"), []byte("Echo")},
	}

	for _, entry := range data {
		err := leveldb.Put(entry.key, entry.value)
		if err != nil {
			fmt.Printf("Failed to Put record with key %q in DB. Error: %q", entry.key, err)
			return
		}
	}
}
