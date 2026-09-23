package scan

import (
	"flag"
	"io/ioutil"
)

func main() {
    var filePath string

    flag.StringVar(&filePath, "filePath", "", "File Path")
    flag.Parse()

    if filePath == "" {
        runPrompt()
    } else {
        runFile(filePath)
    }
}

func runFile(path string) {
    file, err := ioutil.ReadFile(path)
    if err != nil {
        panic(err)
}

    r.run(string(file))
    if 
