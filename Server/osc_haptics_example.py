#!/usr/bin/env python3
import socket
from struct import pack
from pythonosc import dispatcher, osc_server


hostname = socket.gethostbyname("esp32-XXXXXX") # resolve one = efficient
hostport = 1234
if hostname is None:
    raise Exception("Device not found")

motors = {
        0: 'face0',
        1: 'face1',
        2: 'face2',
        3: 'face3',
        4: 'face4',
        5: 'face5',
        }

# invert dict (name -> address) and add prefix
motors = { f'haptics-{name}': addr
        for addr, name in motors.items() }

print("start esp32 socket")
sout = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def on_haptics(path, proximity):
    # path ex: /avatar/parameters/haptics-face1
    name = path.split('/', 3)[-1]
    motor = motors.get(name)
    if motor is None: return # unregistered = ignored

    strength = 255 * (1 - proximity) # proximity 0 = max brrr 255
    print(f'sent_raw ESP: {motor} <- {strength}')
    data = b"m" + pack("BB", motor, strength)
    sout.sendto(data, (hostname, hostport))


print('start osc server')
dispatcher = dispatcher.Dispatcher()
dispatcher.map("/avatar/parameters/haptics-*", on_haptics)

server = osc_server.ThreadingOSCUDPServer(('127.0.0.1', 9001), dispatcher)
server.serve_forever()
