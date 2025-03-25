import socket

def send_data(data):
    UDP_IP = "127.0.0.1"
    UDP_PORT = 12345

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(data.encode(), (UDP_IP, UDP_PORT))
    sock.close()

