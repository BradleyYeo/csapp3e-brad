package leveldb

import (
	//"bytes"
	"database/sql"
	"encoding/binary"

	//"encoding/binary"
	//"fmt"
	"testing"

	_ "github.com/lib/pq"
)

func seedDb() *LevelDb {
	key := []byte("testkey1")
	value := []byte("testvalue1")

	ldb, _ := NewLevelDb(key, value)
	return ldb
}

func emptyLevelDb() *LevelDb {
	key := make([]byte, 0)
	value := make([]byte, 0)

	ldb, _ := NewLevelDb(key, value)
	return ldb
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
	leveldb := emptyLevelDb()

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
			t.Fatalf("Failed to Put record with key %q in DB", entry.key)
		}
	}

	leveldb.entries.PrintSkipList()

	it, _ := leveldb.RangeScan([]byte("bravo"), []byte("delta"))

	expectedRangeScanResult := data[1:3]

	for _, val := range expectedRangeScanResult {
		if string(it.Key()) != string(val.key) && !it.Next() {
			t.Errorf("Expected %q, got %q", string(val.key), string(it.Key()))
		}

	}
}

func Benchmark_LevelDb(b *testing.B) {
	db := setupDB()
	defer db.Close()

	leveldb := emptyLevelDb()

	b.Run("Put", func(b *testing.B) {
		b.ResetTimer()
		for i := 0; i < b.N; i++ {
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

				keyBuf := make([]byte, 8)
				binary.BigEndian.PutUint64(keyBuf, uint64(id))

				valueBuf := []byte(title)

				leveldb.Put(keyBuf, valueBuf)
			}
			rows.Close()
		}
		//leveldb.entries.PrintSkipList()
	})

	b.Run("Get", func(b *testing.B) {
		b.ResetTimer()
		keyBuf := []byte("Innocence (2014)")
		for i := 0; i < b.N; i++ {
			leveldb.Get(keyBuf)
		}
	})

	b.Run("RangeScan", func(b *testing.B) {
		b.ResetTimer()
		start := []byte("Whisky (2004)")
		end := []byte("Elevator Girl (2010)")
		for i := 0; i < b.N; i++ {
			leveldb.RangeScan(start, end)
		}
	})

	b.Run("Delete", func(b *testing.B) {
		b.ResetTimer()
		key := []byte("Whisky (2004)")
		for i := 0; i < b.N; i++ {
			leveldb.Delete(key)
		}
	})
}
