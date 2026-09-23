package in_memory_leveldb

import (
	//"bytes"
	"database/sql"
	"encoding/binary"
	//"fmt"
	"testing"

	_ "github.com/lib/pq"
)

func seedDb() DB {
	return &LevelDb{
		entries: []Entry{
			{
				Key:   []byte("testkey1"),
				Value: []byte("testvalue1"),
			},
		},
	}
}

func emptyLevelDb() DB {
	return &LevelDb{
		entries: make([]Entry, 0),
	}
}

type entry struct {
	Key   []byte
	Value []byte
}

type entries []entry

func setupDB() *sql.DB {
	connStr := "postgres://omarflores@localhost:5432/bradfield?sslmode=disable"
	db, err := sql.Open("postgres", connStr)
	if err != nil {
		panic(err)
	}
	return db
}

func TestLevelDb_Get_Ok(t *testing.T) {
	leveldb := seedDb()

	testValue := "testvalue1"

	val, _ := leveldb.Get([]byte("testkey1"))
	if string(val) != testValue {
		t.Errorf("Expected %q, got %q", testValue, val)
	}
}

func TestLevelDb_Get_Error_When_Empty(t *testing.T) {
	leveldb := emptyLevelDb()

	val, err := leveldb.Get([]byte("foo"))

	if err == nil || val != nil {
		t.Error("Expected an error when calling Get() on empty DB")
	}
}

func TestLevelDb_Put_Ok(t *testing.T) {
	leveldb := emptyLevelDb()

	testKey, testValue := []byte("key1"), []byte("value1")

	leveldb.Put(testKey, testValue)

	val, err := leveldb.Get(testKey)

	if err != nil || val == nil {
		t.Errorf("Expected %q, got %q", testValue, val)
	}
}

func TestLevelDb_Has_True(t *testing.T) {
	leveldb := seedDb()

	if found, _ := leveldb.Has([]byte("testkey1")); !found {
		t.Error("Expected the DB to find the test key")
	}
}

func TestLevelDb_Has_False(t *testing.T) {
	leveldb := seedDb()

	if found, _ := leveldb.Has([]byte("testkey2")); found {
		t.Error("Expected the DB to NOT find the test key")
	}
}

func TestLevelDb_Delete_Ok(t *testing.T) {
	leveldb := seedDb()

	testKey := []byte("testkey1")

	err := leveldb.Delete(testKey)

	if err != nil {
		t.Error("Expected Delete to remove the test key correctly")
	}

	val, err := leveldb.Get(testKey)
	if val != nil || err == nil {
		t.Error("Expected Delete to remove the given test key")
	}
}

func TestLevelDb_Delete_Error(t *testing.T) {
	leveldb := seedDb()

	testKey := []byte("testkey2")

	err := leveldb.Delete(testKey)
	if err == nil {
		t.Error("Expected Delete to error out because of non-existent key")
	}
}

func TestLevelDb_RangeScan_Ok(t *testing.T) {
	leveldb := seedDb()

	data := []struct {
		key   []byte
		value []byte
	}{
		{[]byte("alpha"), []byte("Alpha")},
		{[]byte("bravo"), []byte("Bravo")},
		{[]byte("charlie"), []byte("Charlie")},
		{[]byte("delta"), []byte("Delta")},
		{[]byte("echo"), []byte("Echo")},
		{[]byte("foxtrot"), []byte("Foxtrot")},
	}

	for _, entry := range data {
		err := leveldb.Put(entry.key, entry.value)
		if err != nil {
			t.Fatalf("Failed to Put record with key %q in DB", entry.key)
		}
	}

	it, _ := leveldb.RangeScan([]byte("bravo"), []byte("delta"))

	expectedRangeScanResult := data[1:4]

	for _, val := range expectedRangeScanResult {
		if it.Next() {
			if string(it.Key()) != string(val.key) {
				t.Errorf("Expected %q, got %q", string(val.key), string(it.Key()))
			}

		}
	}
}

func Benchmark_InMemoryLevelDbPut(b *testing.B) {
	db := setupDB()
	defer db.Close()

	leveldb := emptyLevelDb()

	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		//var dbRecords entries

		rows, err := db.Query("SELECT id, title FROM movies")
		if err != nil {
			b.Fatal(err)
		}

		for rows.Next() {
			var id int64
			var title string
			if err := rows.Scan(&id, &title); err != nil {
				b.Fatal(err)
			}

			// Convert id to []byte
			keyBuf := make([]byte, 8)
			binary.BigEndian.PutUint64(keyBuf, uint64(id))

			// Assuming title is already a string, convert it to []byte
			valueBuf := []byte(title)

			// Now you can call Put with both key and value as []byte
			leveldb.Put(keyBuf, valueBuf)
		}
		rows.Close()
	}

	//it, _ := leveldb.Dump()
	//for it.Next() {
	//	var keyInt int64
	//	keyBytes := it.Key()

	//	err := binary.Read(bytes.NewReader(keyBytes), binary.BigEndian, &keyInt)
	//	if err != nil {
	//		fmt.Println("Error converting key bytes to int:", err)
	//		continue
	//	}

	//	fmt.Printf("Key: %d, Value: %s\n", keyInt, string(it.Value()))
	//}
}
