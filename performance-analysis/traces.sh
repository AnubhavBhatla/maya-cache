#!/bin/bash

# Check if wget is installed
if ! command -v wget &> /dev/null; then
    echo "wget is not installed. Please install wget before running this script."
    exit 1
fi

# Check if input file is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <input_file>"
    exit 1
fi

input_file=$1

# Check if input file exists
if [ ! -f "$input_file" ]; then
    echo "Input file not found: $input_file"
    exit 1
fi

# Create directory 'traces' if it doesn't exist
mkdir -p traces

# Read URLs from the input file and download each file into 'traces' directory
while IFS= read -r url; do
    echo "Downloading: $url"
    wget -N -P traces/spec "$url"
done < "$input_file"

echo "Download complete."

