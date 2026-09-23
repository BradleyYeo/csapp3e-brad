#!/bin/bash

# Ensure proper number of arguments
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <search term> [section]"
    exit 1
fi

# Base Wikipedia API URL
BASE_URL="https://en.wikipedia.org/w/api.php?format=json&action=parse&page=$1&prop=text&section=0"

# Function to extract section titles from the Wikipedia API response
extract_sections() {
    grep -Po '"line":\K"[^"]+"' | sed 's/"//g'
}

# If only the search term is provided
if [ "$#" -eq 1 ]; then
    # Fetch the summary
    SUMMARY=$(curl -s "$BASE_URL" | grep -Po '"*?":\K"[^"]+"' | sed 's/<[^>]*>//g' | sed 's/"//g' | head -1)
    echo "Summary for $1: $SUMMARY"
    
    # Fetch the sections
    SECTIONS=$(curl -s "https://en.wikipedia.org/w/api.php?format=json&action=parse&page=$1&prop=sections" | extract_sections)
    echo -e "\nSections:"
    echo "$SECTIONS"

# If a section is provided
else
    # Fetch the section number for the provided section name
    SECTION_NUM=$(curl -s "https://en.wikipedia.org/w/api.php?format=json&action=parse&page=$1&prop=sections" | grep -Po "(?<=toclevel\":1,\"line\":\"$2\",\"number\":\")[^\"].*?(?=\")")
    # Handle case where section is not found
    if [ -z "$SECTION_NUM" ]; then
        echo "Section not found!"
        exit 1
    fi
    SUMMARY_URL="https://en.wikipedia.org/w/api.php?format=json&action=parse&page=$1&prop=text&section=$SECTION_NUM"
    SUMMARY=$(curl -s "$SUMMARY_URL" | grep -Po '"*?":\K"[^"]+"' | sed 's/<[^>]*>//g' | sed 's/"//g' | head -1)
    echo "Summary for $1 ($2): $SUMMARY"
    
    # Fetch the subsections for the provided section
    SUBSECTIONS=$(curl -s "https://en.wikipedia.org/w/api.php?format=json&action=parse&page=$1&prop=sections" | grep -Po "(?<=toclevel\":2,\"number\":\"$SECTION_NUM).*?\"line\":\K\"[^\"]+")
    echo -e "\nSubsections of $2:"
    echo "$SUBSECTIONS" | sed 's/"//g'
fi

