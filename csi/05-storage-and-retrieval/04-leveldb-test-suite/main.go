package main

import (
	"fmt"

	"github.com/syndtr/goleveldb/leveldb"
)

func main() {
	db, err := leveldb.OpenFile("./db", nil)
	if err != nil {
		fmt.Println("Could not open db")
	}
	defer db.Close()

	value := "A new value1"

	err = db.Put([]byte("key1"), []byte(value), nil)

	data, err := db.Get([]byte("key1"), nil)

	if err != nil {
		fmt.Println("Could not read read from DB")
	}

	fmt.Println(string(data))
}
