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
        if hour == 1:
            hour_text = "Ein"
        else:
            hour_text = str(hour)

        text = f"Es ist {hour_text} Uhr"
        # Generate the mp3 name
        mp3_name = "Hour_" + hour_str

        # Generate the text-to-speech
        # Set parameters for the mp3 file
        tts = gTTS(text, lang='de')

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
            text = f"und {str(minutes)} Minute"
        else:
            text = f"und {str(minutes)} Minuten"

        # Generate the mp3 name
        mp3_name = "Minute_" + minutes_str

        # Generate the text-to-speech
        # Set parameters for the mp3 file
        tts = gTTS(text, lang='de')

        # Save the converted audio as an mp3 file inside the 'mp3_FOLDER' folder
        mp3_file = os.path.join(mp3_FOLDER, mp3_name + '.mp3')
        tts.save(mp3_file)
        print(f"Generated {mp3_file} for text: '{text}'")

def generate_text_to_speech(text_array):
    for text, mp3_name in text_array:
        # Text-to-Speech generation
        # Set parameters for the mp3 file
        tts = gTTS(text, lang='de')

        # Save the converted audio as an mp3 file inside the 'mp3_FOLDER' folder
        mp3_file = os.path.join(mp3_FOLDER, mp3_name + '.mp3')
        tts.save(mp3_file)
        print(f"Generated {mp3_file} for text: '{text}'")

# Example array of text and corresponding mp3 names
texts_and_names = [
    ('Fehler bei der Grundstellungsfahrt festgestellt. Bitte überprüfen Sie die Position des Grundstellungsschalters.', 'HomingError'),
    ('Es konnte keine Verbindung zum Gesten-Sensor hergestellt werden.', 'APDS9960Error'),
    ('Die Einstellungen auf der SD-Karte wurden zurückgesetzt.', 'ResetSettings'),
    ('Die WLAN-Verbindung konnte nicht hergestellt werden. Bitte überprüfen Sie die WLAN-Verbindungseinstellungen in der Datei settings.ini auf der SD-Karte.', 'WIFILoginFailed'),
    ('Das WLAN-Netzwerk ist derzeit nicht verfügbar. Bitte überprüfen Sie die WLAN-Verbindungseinstellungen in der Datei settings.ini auf der SD-Karte.', 'WIFINotAvailable'),
    ('Der Zeitabgleich ist fehlgeschlagen. Bitte überprüfen Sie die NTP-Einstellungen in der Datei settings.ini auf der SD-Karte.', 'NTPError'),
]

# Generate text-to-speech for each item in the array
generate_text_to_speech(texts_and_names)

# Generate mp3 files for each hour and minute combination
generate_mp3_for_hours()
generate_mp3_for_minutes()
