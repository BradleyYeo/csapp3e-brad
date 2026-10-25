# Unix Shell Scripting

Practical shell pipelines and automation utilities for data fetching and stream processing.

## Scripts

- [wiki.sh](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/shell-scripting/wiki.sh): Wikipedia terminal lookup tool leveraging `curl`, POSIX regex pattern matching (`grep -Po`), and text stream transformations (`sed`).

## Usage

### Article Overview
- Fetch article summary and section table of contents:
  ```bash
  ./wiki.sh "Linux"
  ```

### Section Lookup
- Fetch a specific article section:
  ```bash
  ./wiki.sh "Linux" "Architecture"
  ```

## Core Pipeline Concepts

- **API Queries**: Invoking REST JSON endpoints via `curl -s`.
- **Lookbehind Pattern Matching**: Using PCRE lookarounds `(?<=...)` and match resets `\K` in `grep -Po` to isolate JSON values without external parser dependencies.
- **HTML Tag Stripping**: Stream-editing HTML markup out of raw JSON payloads with `sed 's/<[^>]*>//g'`.
