package iter

type BinaryExpression interface {
	Execute(Tuple) bool
}

type EqualExpression struct {
	Column string
	Value  string
}

func (ee *EqualExpression) Execute(tup Tuple) bool {
	for _, val := range tup.Columns {
		if val.Name == ee.Column && val.Value == ee.Value {
			return true
		}
	}

	return false
}

type SelectionIterator struct {
	source     Iterator
	expression BinaryExpression
}

func (s *SelectionIterator) Next() (Tuple, error) {
	for {
		tup, err := s.source.Next()
		if err != nil {
			break
		}

		if s.expression.Execute(tup) {
			return tup, nil
		}
	}

	return Tuple{}, ERROR_EOF
}

func NewSelectionIterator(source Iterator, exp BinaryExpression) *SelectionIterator {
	return &SelectionIterator{
		source:     source,
		expression: exp,
	}
}
