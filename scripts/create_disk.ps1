# create_disk.ps1
# Usage: .\scripts\create_disk.ps1 <filename> <size_in_mb>

param (
    [string]$DiskName = "xffs.img",
    [int]$SizeMB = 64
)

$SizeBytes = $SizeMB * 1024 * 1024
Write-Host "Creating virtual disk: $DiskName ($SizeMB MB)..."

# Create a zero-filled file
$file = [System.IO.File]::Create($DiskName)
$file.SetLength($SizeBytes)
$file.Close()

Write-Host "Done! You can now format it using mkxffs."
