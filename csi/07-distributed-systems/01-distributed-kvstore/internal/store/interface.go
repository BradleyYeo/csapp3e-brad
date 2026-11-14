package store

import (
	pb "github.com/omarfq/kvstore/api/v1"
)

type KVStore interface {
	// Returns the value by a given key or an error
	// if the key does not exist
	Get(key *pb.Data) (string, error)
	// Sets or updates a new key-value pair in the
	// data store
	Set(keyValue *pb.Data) error
}
