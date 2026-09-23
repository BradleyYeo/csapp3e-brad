package main

import (
	"fmt"
	"go/ast"
	"go/parser"
	"go/token"
	"log"
	"strconv"
)

// Given an expression containing only int types, evaluate
// the expression and return the result.
func Evaluate(expr ast.Expr) (int, error) {
	switch node := expr.(type) {
	case *ast.BasicLit:
		num, err := strconv.Atoi(node.Value)
		if err != nil {
			fmt.Println("Error converting node. Value to an integer")
		}
		return num, err
	case *ast.BinaryExpr:
		leftOperand, err := Evaluate(node.X)
		if err != nil {
			return 0, err
		}
		rightOperand, err := Evaluate(node.Y)
		if err != nil {
			return 0, err
		}
		switch node.Op {
		case token.ADD:
			return leftOperand + rightOperand, err
		case token.SUB:
			return leftOperand - rightOperand, err
		case token.MUL:
			return leftOperand * rightOperand, err
		case token.QUO:
			return leftOperand / rightOperand, err
		}
	case *ast.ParenExpr:
		return Evaluate(node.X)
	}
	return 0, nil
}

func main() {
	expr, err := parser.ParseExpr("1 + 2 - 3 * 4")
	if err != nil {
		log.Fatal(err)
	}
	fset := token.NewFileSet()
	err = ast.Print(fset, expr)
	if err != nil {
		log.Fatal(err)
	}
	Evaluate(expr)
}
