package iter

type SortIterator struct {
	source       Iterator
	sortedColumn []*Tuple
	columnToSort string
}

// func (s *SortIterator) Next() (*Tuple, error) {
// TODO: Implement SortIterator
//nextTuple, err := s.source.Next()
//if err != nil {
//	return &Tuple{}, err
//}
// }
