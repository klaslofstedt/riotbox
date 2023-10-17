import serial
import time
import json
import os
import base64
from enum import Enum
import sys

if len(sys.argv) < 5:
    print("Error: Not enough arguments provided. Expected THING_SERIAL_PORT, THING_TYPE, THING_HW_VERSION, and ESP_DEPLOY_PATH")
    sys.exit(1)

print('Running production')
serial_port = sys.argv[1]
thing_type = sys.argv[2]
thing_hw_version = sys.argv[3]
esp_deploy_file_path = sys.argv[4]
print('Use port: ', serial_port)

# print('Running production')
# serial_port = sys.argv[1] if len(sys.argv) > 1 else '/dev/tty.usbserial-0001'
# print('Use port: ', serial_port)
# Initialize serial port
ser = serial.Serial(serial_port, 115200)
print("Serial done")
ser.reset_input_buffer()
print("Buffer cleared")


class StateCode(Enum):
    READ_ID = 5
    WRITE_BLOB = 6
    READ_STATUS = 7
    WRITE_TO_FILE = 8


class StatusCode(Enum):
    OK_ID = 0
    OK_STATUS = 1
    ERROR_JSON = 2
    ERROR_DECODE = 3
    ERROR_STATUS = 4


def try_decoding(line):
    # Try to decode the line from base64
    decoded_line = None
    try:
        decoded_line = base64.b64decode(line).decode('utf-8')
        print("Decoded line: ", decoded_line)
        # Parse the decoded line as JSON if it was successfully decoded
        if decoded_line is not None:
            data = json.loads(decoded_line)
            print(data)
            for key, value in data.items():
                if key == 'id':
                    print("Valid ID:", value)
                    return StatusCode.OK_ID, value
                elif key == 'status' and value == 'ok':
                    print("Status: ", value)
                    return StatusCode.OK_STATUS, None
                elif key == 'status' and value == 'error':
                    print("Status: ", value)
                    return StatusCode.ERROR_STATUS, None
                else:
                    print("Error: Unexpected key in JSON data")
                    return StatusCode.ERROR_JSON, None
        else:
            print("Empty JSON")
            return StatusCode.ERROR_JSON, None
    except:
        print("Failed to decode: ", line)
        return StatusCode.ERROR_DECODE, None


def try_writing_blob():
    pop = str(os.urandom(16).hex())
    aes = str(os.urandom(16).hex())

    response = {
        "pop": pop,
        "aes": aes,
        "type": thing_type,
        "hw_version": thing_hw_version,
    }
    response_json = json.dumps(response)
    response_base64 = base64.b64encode(
        response_json.encode('utf-8')).decode('utf-8') + '\n'
    ser.write(response_base64.encode('utf-8'))
    return pop, aes


pop = None
aes = None
id = None
state = StateCode.READ_ID
while True:
    if (state == StateCode.READ_ID):
        if (ser.in_waiting > 0):
            line = ser.read(ser.in_waiting).decode('utf-8')
            print("Print ID: ", line)
            status, id = try_decoding(line)
            print(status, id)
            if status == StatusCode.OK_ID:
                state = StateCode.WRITE_BLOB
                #id = value
    if (state == StateCode.WRITE_BLOB):
        print("Write blob")
        pop, aes = try_writing_blob()
        state = StateCode.READ_STATUS
    if (state == StateCode.READ_STATUS):
        if (ser.in_waiting > 0):
            line = ser.read(ser.in_waiting).decode('utf-8')
            print("Print Status: ", line)
            status, _ = try_decoding(line)
            if status == StatusCode.OK_STATUS:
                state = StateCode.WRITE_TO_FILE
            else:
                state = StateCode.WRITE_BLOB
    if (state == StateCode.WRITE_TO_FILE):
        if (pop != None and aes != None and id != None):
            print("Print to file")
            # Ensure the directory exists before writing the file
            os.makedirs(os.path.dirname(esp_deploy_file_path), exist_ok=True)
            with open(esp_deploy_file_path, "w") as file:
                file.write(f"DEPLOY_ID={id}\n")
                file.write(f"DEPLOY_TYPE={thing_type}\n")
                file.write(f"DEPLOY_POP={pop}\n")
                file.write(f"DEPLOY_AES={aes}\n")
            print(f"Id {id}")
            print(f"Type {thing_type}")
            print(f"Hw version {thing_hw_version}")
            print(f"Pop {pop}")
            print(f"Aes {aes}")
            print("Done")
            break
        else:
            print(f"Error: id {id}")
            print(f"Error: type {thing_type}")
            print(f"Error: hw version {thing_hw_version}")
            print(f"Error: pop {pop}")
            print(f"Error: aes {aes}")
            break
