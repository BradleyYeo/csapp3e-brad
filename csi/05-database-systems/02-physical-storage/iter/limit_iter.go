package iter

type LimitIterator struct {
	source Iterator
	limit  int
	count  int
}

func (l *LimitIterator) Next() (Tuple, error) {
	if l.count >= l.limit {
		return Tuple{}, ERROR_EOF
	}

	tup, err := l.source.Next()
	if err != nil {
		return Tuple{}, err
	}

	l.count++
	return tup, nil
}

func NewLimitIterator(source Iterator, limit int) LimitIterator {
	return LimitIterator{
		source: source,
		limit:  limit,
	}
}
