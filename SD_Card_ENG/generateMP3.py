import os
from gtts import gTTS

# Change the current working directory to the directory where the script is located
script_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(script_dir)

# Define the path for saving generated mp3 files
mp3_FOLDER = 'voice'

def generate_mp3_for_hours():
    # Create the 'generic' folder if it doesn't exist
    if not os.path.exists(mp3_FOLDER):
        os.makedirs(mp3_FOLDER)

    for hour in range(0, 24):
        # Convert hour and minute to strings with leading zeros for file name
        hour_str = str(hour).zfill(2)

        # Determine the text for the hour
        #if hour == 1:
        #    hour_text = "Ein"
        #else:
        hour_text = str(hour)

        text = f"It's {hour_text} a Clock"
        # Generate the mp3 name
        mp3_name = "Hour_" + hour_str

        # Generate the text-to-speech
        # Set parameters for the mp3 file
        tts = gTTS(text, lang='en')

        # Save the converted audio as an mp3 file inside the 'mp3_FOLDER' folder
        mp3_file = os.path.join(mp3_FOLDER, mp3_name + '.mp3')
        tts.save(mp3_file)
        print(f"Generated {mp3_file} for text: '{text}'")

def generate_mp3_for_minutes():
    # Create the 'generic' folder if it doesn't exist
    if not os.path.exists(mp3_FOLDER):
        os.makedirs(mp3_FOLDER)

    for minutes in range(0, 60):
        # Convert minutes and minute to strings with leading zeros for file name
        minutes_str = str(minutes).zfill(2)

        # Determine the text for the minutes
        if minutes == 1:
            text = f"and {str(minutes)} Minutes"
        else:
            text = f"and {str(minutes)} Minutes"

        # Generate the mp3 name
        mp3_name = "Minute_" + minutes_str

        # Generate the text-to-speech
        # Set parameters for the mp3 file
        tts = gTTS(text, lang='en')

        # Save the converted audio as an mp3 file inside the 'mp3_FOLDER' folder
        mp3_file = os.path.join(mp3_FOLDER, mp3_name + '.mp3')
        tts.save(mp3_file)
        print(f"Generated {mp3_file} for text: '{text}'")

def generate_text_to_speech(text_array):
    for text, mp3_name in text_array:
        # Text-to-Speech generation
        # Set parameters for the mp3 file
        tts = gTTS(text, lang='en')

        # Save the converted audio as an mp3 file inside the 'mp3_FOLDER' folder
        mp3_file = os.path.join(mp3_FOLDER, mp3_name + '.mp3')
        tts.save(mp3_file)
        print(f"Generated {mp3_file} for text: '{text}'")

# Example array of text and corresponding mp3 names
texts_and_names = [
    ('Error detected during homing operation. Please check the position of the homing switch.', 'HomingError'),
    ('Could not establish a connection to the gesture sensor.', 'APDS9960Error'),
    ('Settings on the SD card have been reset.', 'ResetSettings'),
    ('Failed to establish a Wi-Fi connection. Please check the Wi-Fi connection settings in the settings.ini file on the SD card.', 'WIFILoginFailed'),
    ('The Wi-Fi network is currently unavailable. Please check the Wi-Fi connection settings in the settings.ini file on the SD card.', 'WIFINotAvailable'),
    ('Time synchronization failed. Please check the NTP settings in the settings.ini file on the SD card.', 'NTPError'),
]

# Generate text-to-speech for each item in the array
generate_text_to_speech(texts_and_names)

# Generate mp3 files for each hour and minute combination
generate_mp3_for_hours()
generate_mp3_for_minutes()
