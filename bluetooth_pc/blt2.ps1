cd D:\Elena\_Elena\Shared\Universita\Magistrale\IoT\Esame_progetto_iot\bluetooth_pc
$OUTPUT_FILE = "output.txt"
$ERROR_FILE = "error.txt"

# Create a named pipe
$pipePath = "\\.\pipe\MyPipeInput"
# $pipe = New-Object System.IO.Pipes.NamedPipeServerStream -ArgumentList $pipePath, [System.IO.Pipes.PipeDirection]::InOutIn
$pipe = New-Object System.IO.Pipes.NamedPipeServerStream -ArgumentList $pipePath, In

# Start Python script in a separate process
$pythonProcess = Start-Process "python" '.\bluetooth_connect_to_device.py' -RedirectStandardInput $pipe -RedirectStandardError $ERROR_FILE -RedirectStandardOutput $OUTPUT_FILE -NoNewWindow -Wait

# Add "hello" to the content of the named pipe (input stream for Python script)
# Add-Content -Path $pipePath -Value "hello"

# Wait for the Python process to finish
$pythonProcess.WaitForExit()
