#!/bin/bash

echo "Downloading required SPEC-2017 traces"

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

echo "SPEC benchmarks Download complete."




# Define the directory to check
directory="traces/gap"

# Define the file containing the list of files to check
file_list="scripts/gap_file_list.txt"

# Check if the directory exists
if [ ! -d "$directory" ]; then
    echo "Error: Directory $directory does not exist."
    exit 1
fi

# Check if the file list exists
if [ ! -f "$file_list" ]; then
    echo "Error: File list $file_list does not exist."
    exit 1
fi

error_flag=0
# Read each line in the file list and validate each file
while IFS= read -r file; do
    file_path="$directory/$file"
    # Check if the file exists
    if [ ! -e "$file_path" ]; then
        error_flag=1
        break  # Exit the loop upon encountering a missing file
    else
        # Check if the file is readable
        if [ ! -r "$file_path" ]; then
            echo "File $file is not readable."
        else
            echo "File $file is valid and readable."
        fi
    fi
done < "$file_list"

if [ "$error_flag" -eq 1 ]; then
    echo "GAP benchmark validation failed: One or more files are missing."
    exit 1
else
    echo "Validation successful: All GAP benchmarks are present."
fi




