import copy
import argparse
import time

import mediapipe as mp
import numpy as np
import cv2
from udp_module import send_data


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--device", type=int, default=0)
    args = parser.parse_args()
    return args


def main():
  # Get initialization arguments
  gesture = "UNKNOWN"

  args = get_args()
  cap_device = args.device
  cap_width = 240
  cap_height = 240
  use_static_image_mode = False
  min_detection_confidence = 0.5
  min_tracking_confidence = 0.5
  
  # Camera preparation 
  cap = cv2.VideoCapture(cap_device)
  cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G')) #comment out if there are any issues
  cap.set(cv2.CAP_PROP_FRAME_WIDTH, cap_width)
  cap.set(cv2.CAP_PROP_FRAME_HEIGHT, cap_height)
  cap.set(cv2.CAP_PROP_FPS,10) #limit fps, maybe better performance?
  time.sleep(1)

  # Load Model 
  mp_hands = mp.solutions.hands
  hands = mp_hands.Hands(
      static_image_mode=use_static_image_mode,
      max_num_hands=1,
      min_detection_confidence=min_detection_confidence,
     min_tracking_confidence=min_tracking_confidence,
  )

  while True:
    key = cv2.waitKey(10)
    if key == 27:
        break

    # Camera capture 
    ret, frame = cap.read()
    if not ret:
        break
    frame = cv2.flip(frame, 1) 

    # Detection implementation 
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    frame.flags.writeable = False
    results = hands.process(frame)
    frame.flags.writeable = True

    previous_gesture = None

    if results.multi_hand_landmarks is not None:
      for hand_landmarks, _ in zip(results.multi_hand_landmarks,
                                                results.multi_handedness):
    
        # Gestures determined manually for now
        gesture, bit_value = extract_gesture(hand_landmarks.landmark)
        landmark_list = rescale_landmarks(240, 240, hand_landmarks)
        flattened_list = landmark_list_to_string(landmark_list)
        # If its a new gesture we send data via UDP module
        if previous_gesture != gesture and gesture != "UNKNOWN":
          combined = f"{bit_value} {flattened_list}"
          send_data(combined)
          previous_gesture = gesture

  cap.release()
  cv2.destroyAllWindows()

def landmark_list_to_string(landmark_list):
    if isinstance(landmark_list, list):
        flattened = [str(num) for pair in landmark_list for num in pair]
        return " ".join(flattened)

#Extract x,y values rescaled to desired domensions
def rescale_landmarks(image_width, image_height, landmarks):
    landmark_list = []
    for _, landmark in enumerate(landmarks.landmark):
        landmark_x = min(int(landmark.x * image_width), image_width - 1)
        landmark_y = min(int(landmark.y * image_height), image_height - 1)
        landmark_list.append([landmark_x, landmark_y])

    return landmark_list

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

def extract_gesture(landmarks):

  #helper function, cal
  def distance(landmark1, landmark2):
    return ((landmark1.x - landmark2.x) ** 2 + (landmark1.y - landmark2.y) ** 2) ** 0.5
  
  #extracting key joints we're going to use
  thumb_tip = landmarks[4]
  index_tip = landmarks[8]
  middle_tip = landmarks[12]
  ring_tip = landmarks[16]
  pinky_tip = landmarks[20]


  # Check for index and thumb touching
  thumb_index = (
    # Checks that index and thumb are touching
    distance(thumb_tip, index_tip) < 0.15
    # checks that other fingers are open
  )
  # Middle finger and thumb touching
  thumb_middle = (
    distance(thumb_tip, middle_tip) < 0.15
  )
  # Ring finger and thumb touching 
  thumb_ring = (
    distance(thumb_tip, ring_tip) < 0.15
  )

  # Pinky and thumb touching
  thumb_pinky = (
    distance(thumb_tip, pinky_tip) < 0.15
  )

  bit_index = 0b1000 if thumb_index else 0b0000
  bit_middle = 0b0100 if thumb_middle else 0b0000
  bit_ring = 0b0010 if thumb_ring else 0b0000
  bit_pinky = 0b0001 if thumb_pinky else 0b0000

  touching_fingers = bit_index | bit_middle | bit_pinky | bit_ring

  #for testing overlapping conditions. If overlap found, adjust the thresholds
  print("{0:b}".format(touching_fingers))
  if(touching_fingers == 0):
      return "OPEN_HAND", "0000"
  else:
      return "FINGER_THUMB", str(touching_fingers)


if __name__=="__main__":
  main()


