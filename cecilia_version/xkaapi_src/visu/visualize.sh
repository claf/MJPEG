#!/bin/bash

# Convert the trace using Awk :
cat $1 | awk '{print $6 "\t" $10 "\t" $13 "\t" $7}' > converted.trace

# Create the Paje trace :
./convert.py converted.trace > $1.paje

# Remove temp file :
rm converted.trace

# Launch Vite :
vite $1.paje
