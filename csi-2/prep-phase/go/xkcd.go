package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"strings"
)

const (
	xkcdURL   = "https://xkcd.com/%d/info.0.json"
	indexFile = "xkcdIndex.json"
)

type Comic struct {
	Num        int
	Year       string
	Month      string
	Day        string
	Link       string
	News       string
	SafeTitle  string `json:"safe_title"`
	Transcript string
	Alt        string
	Img        string
	Title      string
}

var Comics []Comic

func main() {
	comics, err := loadIndex()
	if err != nil || len(comics) == 0 {
		fmt.Println("Downloading comic index, please wait...")
		comics = downloadComics()
	}

	searchTerm := os.Args[1]
	for _, comic := range comics {
		if strings.Contains(comic.Transcript, searchTerm) {
			fmt.Printf("URL: %s\nTranscript: %s\n\n", comic.Img, comic.Transcript)
		}
	}
}

func downloadComics() []Comic {
	i := 1
	for {
		resp, err := http.Get(fmt.Sprintf(xkcdURL, i))
		if err != nil || resp.StatusCode != 200 {
			break
		}

		var comic Comic
		if err := json.NewDecoder(resp.Body).Decode(&comic); err != nil {
			resp.Body.Close()
			break
		}
		resp.Body.Close()
		Comics = append(Comics, comic)
		i++
	}
	saveIndex(Comics)
	return Comics
}

func loadIndex() ([]Comic, error) {
	bytes, err := ioutil.ReadFile(indexFile)
	if err != nil {
		return nil, err
	}

	var comics []Comic
	if err := json.Unmarshal(bytes, &comics); err != nil {
		return nil, err
	}
	return comics, nil
}

func saveIndex(comics []Comic) {
	bytes, err := json.Marshal(comics)
	if err != nil {
		panic(err)
	}

	if err := ioutil.WriteFile(indexFile, bytes, 0644); err != nil {
		panic(err)
	}
}
