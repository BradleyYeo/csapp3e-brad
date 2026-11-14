package iter

import (
	"encoding/csv"
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
	f, err := os.Open("data/movies.csv")
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	r := csv.NewReader(f)
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
