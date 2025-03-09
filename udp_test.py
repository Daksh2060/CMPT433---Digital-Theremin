import udp_module
import time

def timer_loop():
    counter = 0
    while counter < 15:
        udp_module.send_data("Second: " + str(counter))
        counter += 1
        time.sleep(1)

timer_loop()
udp_module.send_data("stop")

