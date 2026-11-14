package iter

import (
	"fmt"
	"io"
	"log"
	"os"
)

type ScanIterator struct {
	tuples       []Tuple
	currTupleIdx int
}

func (s *ScanIterator) Next() (Tuple, error) {
	if s.currTupleIdx >= len(s.tuples) {
		return Tuple{}, ERROR_EOF
	}

	curr := s.currTupleIdx
	s.currTupleIdx++

	return s.tuples[curr], nil
}

type FileScanner struct {
	header  *Header
	r       *byteReader
	numRead int
	next    Tuple
}

// NewFileScanner creates a new FileScanner that scans the provided file.
func NewFileScanner(r io.Reader) *FileScanner {
	return &FileScanner{
		r: newByteReader(r),
	}
}

func NewScanIterator(tuples []Tuple) ScanIterator {
	return ScanIterator{
		tuples:       tuples,
		currTupleIdx: 0,
	}
}

func NewFileScan(filename string) *ScanIterator {
	wd, err := os.Getwd()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("Current working directory:", wd)
	f, err := os.Open(filename)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	fileScanner := solution.NewFileScanner(f)
	headers, err := r.Read()
	if err != nil {
		log.Fatal(err)
	}

	var tuples []Tuple
	for {
		record, err := r.Read()
		if err == io.EOF {
			break
		}

		if err != nil {
			log.Fatal(err)
		}

		var tup Tuple
		for idx, val := range record {
			tup.Columns = append(tup.Columns, Column{Name: headers[idx], Value: val})
		}

		tuples = append(tuples, tup)
	}

	scan := NewScanIterator(tuples)
	return &scan
}
