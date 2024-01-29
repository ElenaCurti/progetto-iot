import os
import time

# Get the current timestamp
current_timestamp = time.time()

# Calculate the timestamp of 7 days ago (in seconds)
seven_days_ago = current_timestamp - (7 * 24 * 60 * 60)
one_minute_ago = current_timestamp - 60


# Directory containing the images
directory = "D:\\Elena\\_Elena\\Shared\\Universita\\Magistrale\\IoT\\Esame_progetto_iot\\immagini_salvate\\"


# Iterate over the files in the directory
for filename in os.listdir(directory):
    # Check if the filename ends with ".png"
    if filename.endswith(".png"):
        # Parse the timestamp from the filename
        timestamp = int(filename.split(".")[0])
        
        # Convert the timestamp to seconds
        timestamp_seconds = timestamp / 1000  # Assuming the timestamp is in milliseconds
        
        
        # if timestamp_seconds < seven_days_ago:
        if timestamp_seconds < one_minute_ago:
            # Remove the file
            os.remove(os.path.join(directory, filename))
            print(f"Removed file: {filename}")

