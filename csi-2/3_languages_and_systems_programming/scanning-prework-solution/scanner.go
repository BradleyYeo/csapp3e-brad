package main

import (
	"errors"
	"fmt"
	"strings"
	"unicode"
	"unicode/utf8"
)

const (
	INVALID TokenType = iota
	L_PAREN
	R_PAREN
	NOT
	AND
	OR
	TERM
	PHRASE
	EOF
)

var typeToString = map[TokenType]string{
	INVALID: "INVALID",
	L_PAREN: "L_PAREN",
	R_PAREN: "R_PAREN",
	NOT:     "NOT",
	AND:     "AND",
	OR:      "OR",
	TERM:    "TERM",
	PHRASE:  "PHRASE",
	EOF:     "EOF",
}

var keywordToType = map[string]TokenType{
	"NOT": NOT,
	"AND": AND,
	"OR":  OR,
}

type TokenType int

type Token struct {
	tokenType TokenType
	literal   string
}

func (t Token) String() string {
	return fmt.Sprintf("%s('%s')", typeToString[t.tokenType], t.literal)
}

var (
	tokenInvalid = Token{INVALID, ""}
	tokenEOF     = Token{EOF, ""}
)

var symbolToType = map[rune]TokenType{
	'(': L_PAREN,
	')': R_PAREN,
	'-': NOT,
}

type Scanner struct {
	idx       int
	src       []byte
	nextToken Token
}

func newScanner(src []byte) *Scanner {
	return &Scanner{
		idx:       0,
		src:       src,
		nextToken: tokenInvalid,
	}
}

func (s *Scanner) isAtEnd() bool {
	return s.idx == len(s.src)
}

func (s *Scanner) reset() {
	s.idx = 0
	s.nextToken = tokenInvalid
}

// Precondition: !s.isAtEnd()
func (s *Scanner) peekChar() (rune, int) {
	return utf8.DecodeRune(s.src[s.idx:])
}

// Precondition: !s.isAtEnd()
func (s *Scanner) consume() (rune, int) {
	r, size := s.peekChar()
	s.idx += size
	return r, size
}

func (s *Scanner) skipWhitespace() {
	for !s.isAtEnd() {
		r, _ := s.peekChar()
		if !unicode.IsSpace(r) {
			return
		}
		s.consume()
	}
}

// Precondition: the first rune at s.src[s.idx] is a letter
func (s *Scanner) scanLetters() (Token, error) {
	startIdx := s.idx
	s.consume()
	for !s.isAtEnd() {
		r, _ := s.peekChar()
		if !unicode.IsLetter(r) {
			break
		}
		s.consume()
	}
	literal := string(s.src[startIdx:s.idx])
	if keywordType, ok := keywordToType[strings.ToUpper(string(literal))]; ok {
		return Token{keywordType, literal}, nil
	}
	return Token{TERM, literal}, nil
}

// Precondition: the first rune at s.src[s.idx] is "
func (s *Scanner) scanPhrase() (Token, error) {
	startIdx := s.idx
	s.consume()
	for !s.isAtEnd() {
		r, _ := s.consume()
		if r == '"' {
			literal := string(s.src[startIdx:s.idx])
			return Token{PHRASE, literal}, nil
		} else if r == '\\' {
			if s.isAtEnd() {
				return tokenInvalid, errors.New("incomplete escape sequence")
			}
			// Unconditionally consume second character of escape sequence
			s.consume()
		}
	}
	return tokenInvalid, errors.New("could not find matching quote")
}

func (s *Scanner) scanInternal() (Token, error) {
	s.skipWhitespace()
	if s.isAtEnd() {
		return tokenEOF, nil
	}
	r, _ := s.peekChar()
	if r == utf8.RuneError {
		return tokenInvalid, fmt.Errorf("invalid UTF-8 character at position %d", s.idx)
	} else if t, ok := symbolToType[r]; ok {
		startIdx := s.idx
		s.consume()
		literal := string(s.src[startIdx:s.idx])
		return Token{t, literal}, nil
	} else if unicode.IsLetter(r) {
		return s.scanLetters()
	} else if r == '"' {
		return s.scanPhrase()
	} else {
		return tokenInvalid, fmt.Errorf("unsupported character %c\n", r)
	}
}

func (s *Scanner) Scan() (Token, error) {
	if s.nextToken != tokenInvalid {
		result := s.nextToken
		s.nextToken = tokenInvalid
		return result, nil
	}
	return s.scanInternal()
}

func (s *Scanner) Peek() (Token, error) {
	if s.nextToken != tokenInvalid {
		return s.nextToken, nil
	}
	result, err := s.scanInternal()
	if err != nil {
		return tokenInvalid, err
	}
	s.nextToken = result
	return result, nil
}

func (s *Scanner) Consume(t TokenType) error {
	token, err := s.Scan()
	if err != nil {
		return err
	}
	if token.tokenType != t {
		return fmt.Errorf("expected %v, got %v", typeToString[t], typeToString[token.tokenType])
	}
	return nil
}
