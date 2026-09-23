package iter

type ProjectionIterator struct {
	source  Iterator
	columns map[string]bool
}

func (p *ProjectionIterator) Next() (Tuple, error) {
	tup, err := p.source.Next()
	if err != nil {
		return Tuple{}, err
	}

	var vals []Column
	for _, v := range tup.Columns {
		if _, ok := p.columns[v.Name]; ok {
			vals = append(vals, v)
		}
	}

	tup.Columns = vals
	return tup, nil
}

func NewProjectionIterator(source Iterator, columns []string) ProjectionIterator {
	columnMap := make(map[string]bool)
	for _, c := range columns {
		columnMap[c] = true
	}
	return ProjectionIterator{
		source:  source,
		columns: columnMap,
	}
}
