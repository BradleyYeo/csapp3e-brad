package iter

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"
)

// FileWriter writes Bradfield files.
type FileWriter struct {
	// Arguments.
	numRows     int
	columnNames []string
	w           io.Writer

	// State.
	numWritten int
	uvarintBuf []byte
}

// NewFileWriter creates a new Bradfield file format writer.
func NewFileWriter(columnNames []string, numRows int, w io.Writer) *FileWriter {
	return &FileWriter{
		columnNames: columnNames,
		numRows:     numRows,
		w:           w,
		uvarintBuf:  make([]byte, binary.MaxVarintLen64),
	}
}

// Append appends a tuple to the file.
func (w *FileWriter) Append(t Tuple) error {
	if w.numWritten == 0 {
		if err := w.writeHeader(); err != nil {
			return fmt.Errorf("Writer: Append: error writing header: %v", err)
		}
	}

	if len(t.Values) != len(w.columnNames) {
		return fmt.Errorf(
			"Writer: Append: tried to write tuple: %v with %d values, but writer expects: %d columns",
			t, len(t.Values), len(w.columnNames))
	}

	for _, v := range t.Values {
		if err := w.writeUVarint(uint64(len(v.StringValue))); err != nil {
			return fmt.Errorf("Writer: Append: error writing string length uvarint: %v", err)
		}

		// In practice we wouldn't want to allocate a temporary byte slice for each string we
		// want to write, we'd probably do an unsafe cast here, for this is fine for educational
		// purpses.
		if _, err := w.w.Write([]byte(v.StringValue)); err != nil {
			return fmt.Errorf("Writer: Append: error writing string: %s, err: %v", v, err)
		}
	}

	w.numWritten++

	return nil
}

func (w *FileWriter) writeHeader() error {
	header := Header{
		Version:     LatestVersion,
		NumRows:     w.numRows,
		ColumnNames: w.columnNames,
	}

	headerBytes, err := json.Marshal(&header)
	if err != nil {
		return fmt.Errorf(
			"writerHeader: error marshaling header: %v, err: %v",
			header, err)
	}

	if err := w.writeUVarint(uint64(len(headerBytes))); err != nil {
		return fmt.Errorf("writeHeader: error writing header bytes uvarint length: %v", err)
	}

	if _, err := w.w.Write(headerBytes); err != nil {
		return fmt.Errorf("writeHeader: error writing header bytes: %v", err)
	}

	return nil
}

func (w *FileWriter) writeUVarint(x uint64) error {
	varintLen := binary.PutUvarint(w.uvarintBuf, x)
	_, err := w.w.Write(w.uvarintBuf[:varintLen])
	if err != nil {
		return fmt.Errorf("writeUVarint: error writing uvarint: %v", err)
	}
	return nil
}

// Close closes the writer, completing the file.
func (w *FileWriter) Close() error {
	if w.numWritten != w.numRows {
		return fmt.Errorf(
			"Writer: Close: expected to write: %d rows, but wrote: %d",
			w.numWritten, w.numRows)
	}

	// Close is a no-op for the current implementation.
	return nil
}
