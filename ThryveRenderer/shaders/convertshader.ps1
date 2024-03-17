# Assuming the script is located in the shaders directory
$shaderDir = Get-Location
$outputDir = "$shaderDir\SPIRV"

# Create the output directory if it doesn't exist
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

# Function to compile shaders
function Compile-Shaders {
    param (
        [string]$ExtensionFilter
    )
    Get-ChildItem $shaderDir -Filter $ExtensionFilter | ForEach-Object {
        $inputFile = $_.FullName
        # Update here to retain old suffix before adding .spv
        $outputFile = Join-Path $outputDir ($_.BaseName + $_.Extension + ".spv")
        & glslc $inputFile -o $outputFile
    }
}

# Compile vertex and fragment shaders
Compile-Shaders -ExtensionFilter "*.vert"
Compile-Shaders -ExtensionFilter "*.frag"


