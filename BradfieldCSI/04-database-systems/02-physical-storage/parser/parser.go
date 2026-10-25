package parser

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"query_executor_physical_storage/iter"
	"strconv"
)

const (
	PROJECTION = "PROJECTION"
	SELECTION  = "SELECTION"
	SCAN       = "SCAN"
	LIMIT      = "LIMIT"
	EQUALS     = "EQUALS"
)

type PlanOperation struct {
	Op  string   `json:"op"`
	Val []string `json:"val"`
}

type Plan []PlanOperation

func Parse(input []byte) Plan {
	var plan Plan
	json.Unmarshal(input, &plan)
	return plan
}

func parseBinaryExpr(vals []string) (iter.BinaryExpression, error) {
	if len(vals) != 3 {
		return nil, errors.New("wrong number of arguments to create binary expr")
	}

	switch vals[1] {
	case EQUALS:
		return &iter.EqualExpression{Column: vals[0], Value: vals[2]}, nil
	default:
		return nil, fmt.Errorf("binary expression type not implemented: %s", vals[1])
	}
}

func TranslatePlan(plan Plan) iter.Iterator {
	return translate(nil, plan, len(plan)-1)
}

func translate(child iter.Iterator, plan Plan, idx int) iter.Iterator {
	var operator iter.Iterator

	switch plan[idx].Op {
	case SCAN:
		operator = iter.NewFileScan(plan[idx].Val[0])
	case SELECTION:
		expr, err := parseBinaryExpr(plan[idx].Val)
		if err != nil {
			log.Fatal(err)
		}
		operator = iter.NewSelectionIterator(child, expr)
	case LIMIT:
		i, err := strconv.Atoi(plan[idx].Val[0])
		if err != nil {
			log.Fatal("invalid arg for LIMIT", err)
		}
		limit := iter.NewLimitIterator(child, i)
		operator = &limit
	case PROJECTION:
		proj := iter.NewProjectionIterator(child, plan[idx].Val)
		operator = &proj
	default:
		log.Fatal("operator not implemented")
	}

	if idx == 0 {
		return operator
	} else {
		return translate(operator, plan, idx-1)
	}
}
