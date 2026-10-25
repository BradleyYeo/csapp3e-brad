package custom_file_format

import "errors"

// LatestVersion is the latest version of the Bradfield file format.
const LatestVersion = 1

// Header represents a Bradfield file format header.
type Header struct {
	Version     int
	NumRows     int
	ColumnNames []string
}

// Validate validates the Header.
func (h *Header) Validate() error {
	if h.Version == 0 {
		return errors.New("Header: Validate: Version must not be zero")
	}
	if h.NumRows == 0 {
		return errors.New("Header: Validate: NumRows must not be zero")
	}
	if len(h.ColumnNames) == 0 {
		return errors.New("Header: Validate: len(Columns) must not be zero")
	}

	return nil
}
