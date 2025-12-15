import socket
import struct
import time

HOST = "10.74.94.237"
PORT = 8888

response = None

def build_msg_struct(id):
    ang = 90.0
    dist = 1.0
    out = True
    msg_bytes = struct.pack('Iff?', id, ang, dist, out)
    return msg_bytes


if __name__ == "__main__":
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((HOST, PORT))
    
    while True:
        for id in range(2):
            try:
                msg_bytes = build_msg_struct(id)
                client.send(msg_bytes)
    
                response = client.recv(1024)
                print(f"Response -> {response.decode()}")

            except Exception as e:
                print(f"Error envío al robot {id}: {e}")
        
        time.sleep(0.5)