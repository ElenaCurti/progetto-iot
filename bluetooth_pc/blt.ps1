cd D:\Elena\_Elena\Shared\Universita\Magistrale\IoT\Esame_progetto_iot\bluetooth_pc
$INPUT_FILE =  "input.txt"
$OUTPUT_FILE = "output.txt"
$ERROR_FILE = "error.txt"

Set-Content -Path $INPUT_FILE -Value "" -NoNewline
Set-Content -Path $OUTPUT_FILE -Value "" -NoNewline
Set-Content -Path $ERROR_FILE -Value "" -NoNewline

Start-Process "python" '.\bluetooth_connect_to_device.py' -RedirectStandardInput $INPUT_FILE -RedirectStandardError $ERROR_FILE -RedirectStandardOutput $OUTPUT_FILE -NoNewWindow -Wait
