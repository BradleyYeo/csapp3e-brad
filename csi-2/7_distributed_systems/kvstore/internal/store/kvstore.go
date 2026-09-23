package store

import (
	"encoding/binary"
	"fmt"
	pb "github.com/omarfq/kvstore/api/v1"
	"google.golang.org/protobuf/proto"
	"io"
	"os"
	"path/filepath"
	"strings"
)

const FILE_PATH = "data/kvstore.dat"

type FileKVStore struct {
	File *os.File
}

func FileKVStoreInstance() (*FileKVStore, error) {
	dir := filepath.Dir(FILE_PATH)
	if _, err := os.Stat(dir); os.IsNotExist(err) {
		os.MkdirAll(dir, 0755)
	}

	file, err := os.OpenFile(FILE_PATH, os.O_RDWR|os.O_CREATE, 0644)
	if err != nil {
		return nil, err
	}

	return &FileKVStore{
		File: file,
	}, nil
}

func (store *FileKVStore) Get(key *pb.Data) (string, error) {
	// Set file pointer to the beginning of the File
	_, err := store.File.Seek(0, io.SeekStart)
	if err != nil {
		return "", err
	}

	prefixBuf := make([]byte, 8)

	for {
		// Read the FULL prefix length
		if _, err := io.ReadFull(store.File, prefixBuf); err != nil {
			if err == io.EOF {
				break
			}
			return "", err
		}

		// Decode the prefix length to an int
		recordLength := binary.BigEndian.Uint64(prefixBuf)

		// Read data
		dataBuf := make([]byte, recordLength)

		if _, err := io.ReadFull(store.File, dataBuf); err != nil {
			if err == io.EOF {
				break
			}
			return "", err
		}

		// Unmarshall binary data
		data := &pb.Data{}
		if err := proto.Unmarshal(dataBuf, data); err != nil {
			return "", fmt.Errorf("failed to unmarshal record data: %w", err)
		}

		// Compare keys
		if key.Key == data.Key {
			return data.Value, nil
		}
	}

	return "", fmt.Errorf("the key: %s does not exist in the key-value store", key.Key)
}

func (store *FileKVStore) Set(keyValue *pb.Data) error {
	// Check if key exists, if it does update it
	existingValue, err := store.Get(keyValue)
	if err == nil {
		if existingValue == keyValue.Value {
			return nil // No update required
		}
	} else if !strings.Contains(err.Error(), "does not exist") {
		return fmt.Errorf("failed to check existing key: %w", err)
	}

	// Marshal the new key-value pair
	dataBytes, err := proto.Marshal(keyValue)
	if err != nil {
		return fmt.Errorf("failed to serialize key-value pair: %w", err)
	}

	lengthBytes := make([]byte, 8)
	binary.BigEndian.PutUint64(lengthBytes, uint64(len(dataBytes)))

	// Append to the end of the file
	_, err = store.File.Seek(0, io.SeekEnd)
	if err != nil {
		return fmt.Errorf("failed to seek to end of file: %w", err)
	}

	if _, err := store.File.Write(lengthBytes); err != nil {
		return fmt.Errorf("failed to write length prefix: %w", err)
	}
	if _, err := store.File.Write(dataBytes); err != nil {
		return fmt.Errorf("failed to write data: %w", err)
	}

	// Flush
	if err := store.File.Sync(); err != nil {
		return fmt.Errorf("failed to flush data to disk: %w", err)
	}

	return nil
}
