# This python3.4 script is the computer-side component of my
# auto-rotating display project, which rotates a computer display
# automatically when the physical monitor is rotated

# Terminate this script before trying to program the Arduino.
# Similarly, do not have the Arduino serial viewer open whilst this script is
# running

import serial
import string
import time
from subprocess import call # for sending the command to rotate
import traceback # for debugging

# This function tries to initialise a serial connection on dev
# If no device is connected, it will fail
# If a non-Arduino device is connected there, it may connect
def initSerial(dev):
    ser = serial.Serial(dev, 9600, timeout=1,
                        xonxoff=False, rtscts=False, dsrdtr=False)
    ser.flushInput()
    ser.flushOutput()
    return ser



# This function tries to initialise a serial connection
# It blindly tries to connect to anything on possibleDevices
# If if fails, it waits then tries again.
# This function does not return until it succeeds
def waitForSerialInit():

    # The Arduino can appear on any of these ports on my computer
    # It may appear on different ports for you.
    # To figure out which ones to use,
    # 1) open the Arduino IDE
    # 2) Click on Tools > Serial Port
    # The devices listed there are what you should type here
    # possibleDevices = ["COM3"] # Windows
    possibleDevices = ["/dev/ttyACM0","/dev/ttyACM1","/dev/ttyACM2"] # Linux

    while True:
        for dev in possibleDevices:
            try:
                ser = initSerial(dev)
                print("device found on " + dev)
                return ser
            except Exception:
                print("Failed to initialise device on " + dev)
        time.sleep(5)


# depending on what orientation your accelerometer is relative to your monitor,
# you may have to adjust these.
# This is for when the Y axis points to the top of the monitor,
# And the bottom of the Arduino is against the monitor back
#
# The second string on each of these lines are the arguments sent in the
# terminal command to rotate. So if you want to try this on Mac or Windows,
# this is one of the things you'll need to change
#
# linux
translation = {"Y_POS":"left",
               "X_POS":"inverted",
               "X_NEG":"normal",
               "Y_NEG":"right"}
#
# #Windows
# translation = {"Y_POS":"0",
#                "X_POS":"270",
#                "X_NEG":"90",
#                "Y_NEG":"180"}
#

# Only some of the stuff the Arduino sends will be a command
# Other stuff is just diagnostics
# We only want to rotate the display when the line starts and ends with
# these substrings. These must match what's in monitor.ino
line_start = "Rotate Monitor <"
line_end = ">"

# Ok, let's go.
# Start by initialising a serial connection to the Arduino
ser = waitForSerialInit()

while True:
    try:
        line = ser.readline().decode("utf-8")
    except Exception:
        # It's probably been unplugged
        #
        # But this also catches other types of errors,
        # Which is not ideal.
        print("error: ")
        traceback.print_exc()
        print("probably not plugged in")
        time.sleep(5)
        print("trying to init serial again")
        ser = waitForSerialInit()
        continue


    if line == "":
        continue # ignore empty lines

    # print line for debugging purposes
    print("line: " + line)

    # check if the line starts with the special command start
    if line.find(line_start) == 0:
        #yes this is a command
        direction = line.replace(line_start,"")
        direction = direction[0:direction.find(line_end)]
        print("direction: " + direction)

        # check the direction is valid (so not NOT_SURE)
        if direction in translation:
            print("translation: " + translation[direction])

            #for multiple monitors, on Linux:
            command = "xrandr --output HDMI1 --rotate " + translation[direction] + " --primary --output eDP1 --auto --right-of HDMI1 &"

            ##for a single monitor on Windows:
            # command = "display.exe /rotate:" + translation[direction]

            print("running: " + command)
            # os.system(command)
            call(command, shell=True)
        else:
            print("invalid direction: " + direction)
            print("ignoring")
