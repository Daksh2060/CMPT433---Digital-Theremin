import socket

def send_data(data):
    LOCALHOST = "127.0.0.1"
    LAPTOP_TO_BOARD = "192.168.7.2"
    UDP_IP = LOCALHOST
    UDP_PORT = 12345

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(data.encode(), (UDP_IP, UDP_PORT))
    sock.close()

