
### CMPT433 - Final Project Writeup

Dave Williams	(dmw13)
Daksh Patel (dpa45)
Bobby Poulin (rrp5)

Group Name: Digital Theremin

GitHub Repository: https://github.com/Daksh2060/CMPT433---Digital-Theremin

Video Demonstration: https://drive.google.com/file/d/1KrkdGQSwTzkzPl4MDH4WxfZlJGkigXv6/view?usp=sharing 


## System Explanation:
The theremin is a unique, contactless musical instrument that allows musicians to control pitch and volume simply by moving their hands near two metal antennas, without any physical contact. Inspired by this concept, our project aims to replicate the theremin’s functionality by using computer vision and distance sensing. Instead of relying on physical rods, we track the position and movement of a player's hands to control different aspects of the sound output. The core flow of our projects is as follows:
A camera connected to the target device captures a live video feed, using the OpenCV library in Python to handle video capture and frame processing.

The live video feed is then passed into the Google MediaPipe framework. MediaPipe processes the video and identifies key landmarks on the hands, such as fingertips and joints. It outputs a bitmap that contains both the configuration of the landmarks and the relative positions of the fingers within the camera’s frame of reference. This data is sent via UDP to our separate Linux program and is split into two parts:

- Hand commands are extracted and used to control aspects of the audio output.

- Hand landmark positions are separately processed and displayed on the LCD screen to visualize the user's hand.

The hand commands allow us to control different sound parameters such as ####. In parallel, a distance sensor measures the distance of the user’s other hand from the target. This distance is used to control articulation of the sound, mimicking how a physical theremin musician modifies tone by moving their hand closer or farther from the rods. The following is the planned diagram of our system from the project proposal compared to the final product:

The user interface also includes four adjustable dials. These dials allow users to tune certain audio parameters of the audio by pushing the joystick in a specific direction and turning the rotary encoder knob. The user can set maximum values or ranges for parameters such as volume, octave, waveform, and distortion.
Our Digital Theremin attempts to reimagine the theremin by utilizing hand-tracking vision technology in conjunction with distance sensor based sound control to prove a similar musical experience to that which one can have while playing a physical themerin.


## Known issues:

Latency between hand commands and audio feedback:
- There was noticeable latency (~1 second delay) between issuing a hand command via the camera and hearing the audio feedback. To address this, we moved video processing from the target to the host device, reduced the frame rate, and lowered the video resolution to decrease image processing time. Through this we were able to noticeably decrease latency to where it felt more natural to play. 

Distance sensor inaccuracies at large distances:
- When moving our hand more than ~80 cm away from the distance sensor, the sensor's audio detection cone becomes wide enough to pick up nearby objects, causing inaccuracies. To mitigate this, we adjusted the system so that moving the hand closer to the sensor increases the volume (opposite to a traditional theremin) and scaled the distance response logarithmically. This way, users can access the full audio range without needing to move their hands very far.

Audio Creating Noise In Distance Sensor:
- We found a peculiar issue where the distance sensors readings were impacted by the noise created by the theremins audio output. Since the distance sensor uses sound waves to function, we believe the loud sound of the theremin was creating noise in the readings of the sensor, causing the articulation of our audio to sound off. We tried to mitigate this issue by instead taking the rolling average of samples of the distance sensors and throwing away nonsensical readings to help both combat the noise and smooth out the changes in the audio output itself. While this turned the audio transitions smooth, the distance of the sensor causes playing to still feel a bit awkward.


## Additional Hardware and Software:

Logitech C270 USB Camera:
Used for live video capture, connected directly to the target device via USB. It provides the real-time video feed necessary for hand tracking.

Adafruit Mini External USB Stereo Speaker:
Outputs the digital theremin sounds generated by the system. It connects directly to the target device through a USB port.

Ultrasonic Distance Sensor - 3V or 5V - HC-SR04 compatible - RCWL-1601:
Measures the distance of the user's hand from the board to control sound articulation (similar to controlling volume on a traditional theremin). The sensor is wired to the target device using a breadboard and PYMNL connections.

Google MediaPipe:
Processes the live video feed to detect and track hand landmarks. It converts the positions and movements of the hand and fingers into usable data for controlling various audio parameters.

OpenCV (Python):
Handles video capture from the USB camera and passes the video frames to the MediaPipe framework for processing.