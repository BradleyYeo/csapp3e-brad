package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"

	"github.com/omarfq/golox/scan"
)

var (
	hadError        bool
	hadRuntimeError bool

	r = newRunner(os.Stdout, os.Stderr)
)

func main() {
	var filePath string

	flag.StringVar(&filePath, "filePath", "", "File path")
	flag.Parse()

	if filePath == "" {
		runPrompt()
	} else {
		runFile(filePath)
	}
}

func runFile(path string) {
	file, err := os.ReadFile(path)
	if err != nil {
		panic(err)
	}

	r.run(string(file))
	if hadError {
		os.Exit(65)
	}
	if hadRuntimeError {
		os.Exit(70)
	}
}

func runPrompt() {
	inputScanner := bufio.NewScanner(os.Stdin)
	for {
		fmt.Print("> ")
		if !inputScanner.Scan() {
			break
		}

		line := inputScanner.Text()
		fmt.Println(r.run(line))
		hadError = false
	}
}

func run(source string) {
	scanner := scan.NewScanner(source, r.stdErr)
	tokens := scanner.ScanTokens()

}
