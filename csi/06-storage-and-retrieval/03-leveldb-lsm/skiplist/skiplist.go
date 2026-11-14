package skiplist

import (
	"bytes"
	"fmt"
	"math/rand"
	"strconv"
	"strings"
	"time"
)

const (
	MaxLevel    int     = 16
	Probability float64 = 0.5
)

type SkipListNode struct {
	Key, Value []byte
	Forwards   []*SkipListNode
}

type SkipList struct {
	Head     *SkipListNode
	Level    int
	MaxLevel int
	rnd      *rand.Rand
}

func NewSkipList() *SkipList {
	src := rand.NewSource(time.Now().UnixNano())
	rnd := rand.New(src)

	head := &SkipListNode{
		Key:      nil,
		Value:    nil,
		Forwards: make([]*SkipListNode, MaxLevel),
	}

	return &SkipList{
		Head:     head,
		Level:    0,
		MaxLevel: MaxLevel,
		rnd:      rnd,
	}
}

func (sl *SkipList) randomLevel() int {
	level := 1
	for sl.rnd.Float64() < Probability && level < sl.MaxLevel {
		level++
	}
	return level
}

func (sl *SkipList) Insert(key, value []byte) {
	update := make([]*SkipListNode, sl.MaxLevel)
	current := sl.Head

	for i := sl.Level - 1; i >= 0; i-- {
		for current.Forwards[i] != nil && bytes.Compare(current.Forwards[i].Key, key) < 0 {
			current = current.Forwards[i]
		}
		update[i] = current
	}

	level := sl.randomLevel()

	if level > sl.Level {
		for i := sl.Level; i < level; i++ {
			update[i] = sl.Head
		}
		sl.Level = level
	}

	newNode := &SkipListNode{
		Key:      key,
		Value:    value,
		Forwards: make([]*SkipListNode, level),
	}

	for i := 0; i < level; i++ {
		newNode.Forwards[i] = update[i].Forwards[i]
		update[i].Forwards[i] = newNode
	}
}

// Challenge accepted.... Challenge failed successfully
func (sl *SkipList) PrintSkipList() {
	highestUsed := len(sl.Head.Forwards) - 1
	outputLines := make([]string, highestUsed+1)
	var keys []string

	current := sl.Head
	var nodeNum int
	for current != nil {
		repeatLen := 1
		keyLen := len(current.Key)
		if keyLen > repeatLen {
			repeatLen = keyLen
		}

		for i := 0; i <= highestUsed; i++ {
			if nodeNum != 0 {
				outputLines[i] += strings.Repeat("-", repeatLen)
				if i < len(current.Forwards) && current.Forwards[i] != nil {
					outputLines[i] += ">"
				} else {
					outputLines[i] += strings.Repeat("-", repeatLen)
				}
			}

			if i < len(current.Forwards) && current.Forwards[i] != nil {
				outputLines[i] += strconv.Itoa(i)
			} else if i == 0 {
				outputLines[i] += "-"
			} else {
				outputLines[i] += strings.Repeat(" ", repeatLen)
			}
		}

		if current != sl.Head {
			keys = append(keys, string(current.Key))
		}

		current = current.Forwards[0]
		nodeNum++
	}

	for i := highestUsed; i >= 0; i-- {
		fmt.Println(outputLines[i])
	}

	keyLine := strings.Repeat(" ", 3)
	keyLine += strings.Join(keys, " ")
	fmt.Println(keyLine)
}

func (sl *SkipList) Search(key []byte) *SkipListNode {
	current := sl.Head
	for i := sl.Level - 1; i >= 0; i-- {
		for current.Forwards[i] != nil && bytes.Compare(current.Forwards[i].Key, key) < 0 {
			current = current.Forwards[i]
		}
	}
	possibleMatch := current.Forwards[0]
	if possibleMatch != nil && bytes.Compare(possibleMatch.Key, key) >= 0 {
		return possibleMatch
	}
	return nil
}

func (sl *SkipList) Delete(key []byte) error {
	update := make([]*SkipListNode, sl.MaxLevel)
	current := sl.Head

	for i := sl.Level - 1; i >= 0; i-- {
		for current.Forwards[i] != nil && bytes.Compare(current.Forwards[i].Key, key) < 0 {
			current = current.Forwards[i]
		}
		update[i] = current
	}

	current = current.Forwards[0]

	if current != nil && bytes.Compare(current.Key, key) == 0 {
		for i := 0; i < sl.Level; i++ {
			if update[i].Forwards[i] != current {
				break
			}
			update[i].Forwards[i] = current.Forwards[i]
		}

		for sl.Level > 1 && sl.Head.Forwards[sl.Level-1] == nil {
			sl.Level--
		}
		return nil
	}

	return fmt.Errorf("Key not found: %s", key)
}

func (sl *SkipList) Update(key, value []byte) {
	current := sl.Head

	for i := sl.Level - 1; i >= 0; i-- {
		for current.Forwards[i] != nil && bytes.Compare(current.Forwards[i].Key, key) < 0 {
			current = current.Forwards[i]
		}
	}

	current = current.Forwards[0]

	if current != nil && bytes.Compare(current.Key, key) == 0 {
		current.Value = value
	} else {
		sl.Insert(key, value)
	}
}
