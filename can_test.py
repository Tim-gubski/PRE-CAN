# import can

# # read in can packetst at 1mbps

# bus = can.interface.Bus(channel='/dev/tty.usbmodem2101', interface='slcan', bitrate=500000)

# while True:
#     message = bus.recv()
#     print(message)

# # send a can packet
# # hex_string = "3d a0 a7 01 99 18"
# hex_string = "3d a0 a7 02 c4 1c"
# byte_value = bytes.fromhex(hex_string)
# print(byte_value)
# msg = can.Message(arbitration_id=0x310, data=byte_value, is_extended_id=False)
# print(msg)
# bus.send(msg)

# for i in range(50):
#     bus.send(msg)
#     message = bus.recv()
#     print(message)



#!/usr/bin/env python3

import can
import time

def main():
    # Example: Connect to the CANable running slcan firmware at 500 kbps
    # ‘channel’ => your /dev/tty.* device
    # ‘bitrate’ => the desired CAN rate, e.g. 500000
    bus = can.Bus(
        interface='slcan',
        channel='/dev/tty.usbmodem2101',
        bitrate=500000
    )
    
    # Send a test frame
    msg = can.Message(
        arbitration_id=0x123,
        data=[0x01, 0x02, 0x03, 0x04],
        is_extended_id=False
    )
    try:
        bus.send(msg)
        print("Message sent on slcan interface!")
    except can.CanError as e:
        print("Failed to send:", e)

    # Wait for a response
    start = time.time()
    while time.time() - start < 2.0:
        rx = bus.recv(timeout=0.5)
        if rx:
            print("RX:", rx)
            break
    else:
        print("No frame received in 2 seconds.")

    bus.shutdown()

if __name__ == "__main__":
    main()

