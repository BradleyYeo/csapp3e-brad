package main

import (
	"fmt"
	"io"
	"log"
	"net/http"
	"pg_executor/parser"
	"strings"
)

func handleQuery(w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		// send back error code
	}
	defer r.Body.Close()

	plan := parser.Parse(body)

	op := parser.TranslatePlan(plan)

	var response string
	for {
		tup, err := op.Next()
		if err != nil {
			break
		}

		var vals []string
		for _, val := range tup.Columns {
			vals = append(vals, val.Value)
		}

		response = response + strings.Join(vals, ",") + "\n"
	}

	w.Write([]byte(response))
}

/*
Example query
[
  {"op": "PROJECTION", "val": ["movieId", "title"]},
  {"op": "LIMIT", "val": ["5"]},
  {"op": "SCAN", "val": ["movies"]}
]
*/

func main() {
	fmt.Printf("Starting server at port 8080\n")

	http.HandleFunc("/query", handleQuery)

	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}
