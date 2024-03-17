#!/bin/bash

# Assuming the script is run from the shaders directory
shaderDir=$(pwd)
outputDir="$shaderDir/SPIRV"

# Create the output directory if it doesn't exist
if [ ! -d "$outputDir" ]; then
    mkdir -p "$outputDir"
fi

# Function to compile shaders
compile_shaders() {
    extensionFilter=$1
    for inputFile in "$shaderDir"/*$extensionFilter; do
        # Skip if no files are found
        [ -e "$inputFile" ] || continue
        baseName=$(basename -- "$inputFile")
        outputFile="$outputDir/${baseName}${extensionFilter}.spv"
        glslc "$inputFile" -o "$outputFile"
    done
}

# Compile vertex and fragment shaders
compile_shaders ".vert"
compile_shaders ".frag"
