#!/bin/bash

# Define source and destination directories
src_dir="output"
dest_dir="output_renamed"

# Create destination directory if it doesn't exist
mkdir -p "$dest_dir"

# Initialize counter
count=1

# Enable nullglob to handle cases where no files match the pattern
shopt -s nullglob

# Iterate over sorted frame files
for file in "$src_dir"/frames*.png; do
  # Check if it's a regular file
  if [[ -f "$file" ]]; then
    # Generate new filename with leading zeros
    new_name=$(printf "frames%06d.png" "$count")
    
    # Copy the file to the destination directory with the new name
    cp "$file" "$dest_dir/$new_name"
    
    # Increment the counter
    count=$((count + 1))
  fi
done

# Disable nullglob after use
shopt -u nullglob

echo "Renaming complete. Total frames renamed: $((count - 1))"
