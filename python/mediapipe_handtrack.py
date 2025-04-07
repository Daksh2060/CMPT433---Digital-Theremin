import copy
import argparse
import time

import mediapipe as mp
import numpy as np
import cv2
from udp_module import send_data

#ADD HERE, GITHUB LINK FOR REPO THIS WAS BUILT WITH

def get_args():
    
    parser = argparse.ArgumentParser()

    parser.add_argument("--device", type=int, default=0)
    parser.add_argument("--width", help='cap width', type=int, default=640)
    parser.add_argument("--height", help='cap height', type=int, default=480)
    parser.add_argument('--use_static_image_mode', action='store_true')
    parser.add_argument("--min_detection_confidence",
                        help='min_detection_confidence',
                        type=float,
                        default=0.7)
    parser.add_argument("--min_tracking_confidence",
                        help='min_tracking_confidence',
                        type=int,
                        default=0.5)

    args = parser.parse_args()
    return args


def main():
  # Get initialization arguments
  gesture = "UNKNOWN"

  args = get_args()
  cap_device = args.device
  cap_width = args.width
  cap_height = args.height
  use_static_image_mode = args.use_static_image_mode
  min_detection_confidence = args.min_detection_confidence
  min_tracking_confidence = args.min_tracking_confidence

  use_brect = True
  
  # Camera preparation 
  cap = cv2.VideoCapture(cap_device)
  cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G')) #comment out if there are any issues
  cap.set(cv2.CAP_PROP_FRAME_WIDTH, cap_width)
  cap.set(cv2.CAP_PROP_FRAME_HEIGHT, cap_height)
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
    start_latency = time.time()
    key = cv2.waitKey(10)
    if key == 27:
        break

    # Camera capture 
    ret, frame = cap.read()
    if not ret:
        break
    frame = cv2.flip(frame, 1)  # Mirror display
    debug_frame= copy.deepcopy(frame)

    # Detection implementation 
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    frame.flags.writeable = False
    results = hands.process(frame)
    frame.flags.writeable = True

    previous_gesture = None

    if results.multi_hand_landmarks is not None:
      for hand_landmarks, handedness in zip(results.multi_hand_landmarks,
                                                results.multi_handedness):
        # Extract landmarks and draw (Comment out when using on board)
        brect = calc_bounding_rect(debug_frame, hand_landmarks)
        landmark_list = calc_landmark_list(debug_frame, hand_landmarks)
        # Gestures determined manually for now
        gesture, bit_value = extract_gesture(hand_landmarks.landmark)
        #draw bounding box and hand landmarks
        debug_frame = draw_bounding_rect(use_brect, debug_frame, brect)
        debug_frame = draw_landmarks(debug_frame, landmark_list)
        
        if previous_gesture != gesture and gesture != "UNKNOWN":
          print(gesture)
          send_data(bit_value)
          previous_gesture = gesture

    # COMMENT OUT WHEN RUNNING ON BOARD!
    #latency = time.time() - start_latency
    #debug_frame = draw_debug_info(debug_frame, latency, gesture)
    #cv2.imshow('Gesture-Detection', debug_frame)
      

  cap.release()
  cv2.destroyAllWindows()


def calc_bounding_rect(image, landmarks):
    image_width, image_height = image.shape[1], image.shape[0]

    landmark_array = np.empty((0, 2), int)

    for _, landmark in enumerate(landmarks.landmark):
        landmark_x = min(int(landmark.x * image_width), image_width - 1)
        landmark_y = min(int(landmark.y * image_height), image_height - 1)

        landmark_point = [np.array((landmark_x, landmark_y))]

        landmark_array = np.append(landmark_array, landmark_point, axis=0)

    x, y, w, h = cv2.boundingRect(landmark_array)

    return [x, y, x + w, y + h]


def calc_landmark_list(image, landmarks):
    image_width, image_height = image.shape[1], image.shape[0]

    landmark_point = []

    # Keypoint
    for _, landmark in enumerate(landmarks.landmark):
        landmark_x = min(int(landmark.x * image_width), image_width - 1)
        landmark_y = min(int(landmark.y * image_height), image_height - 1)
        # landmark_z = landmark.z

        landmark_point.append([landmark_x, landmark_y])

    return landmark_point

def draw_debug_info(image, latency, gesture):
  cv2.putText(image, f"Latency: {latency*1000:.2f} ms",
              (10,30), cv2.FONT_HERSHEY_SIMPLEX, 0.7,
              (255,255,255), 2)
  cv2.putText(image, f"Gesture: {gesture}",
              (10,60), cv2.FONT_HERSHEY_SIMPLEX, 0.7,
              (255,255,255), 2)
  
  return image

def draw_landmarks(image, landmark_point):
    if len(landmark_point) > 0:
        # Thumb
        cv2.line(image, tuple(landmark_point[2]), tuple(landmark_point[3]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[2]), tuple(landmark_point[3]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[3]), tuple(landmark_point[4]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[3]), tuple(landmark_point[4]),
                (255, 255, 255), 2)

        # Index finger
        cv2.line(image, tuple(landmark_point[5]), tuple(landmark_point[6]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[5]), tuple(landmark_point[6]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[6]), tuple(landmark_point[7]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[6]), tuple(landmark_point[7]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[7]), tuple(landmark_point[8]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[7]), tuple(landmark_point[8]),
                (255, 255, 255), 2)

        # Middle finger
        cv2.line(image, tuple(landmark_point[9]), tuple(landmark_point[10]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[9]), tuple(landmark_point[10]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[10]), tuple(landmark_point[11]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[10]), tuple(landmark_point[11]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[11]), tuple(landmark_point[12]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[11]), tuple(landmark_point[12]),
                (255, 255, 255), 2)

        # Ring finger
        cv2.line(image, tuple(landmark_point[13]), tuple(landmark_point[14]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[13]), tuple(landmark_point[14]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[14]), tuple(landmark_point[15]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[14]), tuple(landmark_point[15]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[15]), tuple(landmark_point[16]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[15]), tuple(landmark_point[16]),
                (255, 255, 255), 2)

        # Little finger
        cv2.line(image, tuple(landmark_point[17]), tuple(landmark_point[18]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[17]), tuple(landmark_point[18]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[18]), tuple(landmark_point[19]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[18]), tuple(landmark_point[19]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[19]), tuple(landmark_point[20]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[19]), tuple(landmark_point[20]),
                (255, 255, 255), 2)

        # Palm
        cv2.line(image, tuple(landmark_point[0]), tuple(landmark_point[1]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[0]), tuple(landmark_point[1]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[1]), tuple(landmark_point[2]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[1]), tuple(landmark_point[2]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[2]), tuple(landmark_point[5]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[2]), tuple(landmark_point[5]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[5]), tuple(landmark_point[9]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[5]), tuple(landmark_point[9]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[9]), tuple(landmark_point[13]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[9]), tuple(landmark_point[13]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[13]), tuple(landmark_point[17]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[13]), tuple(landmark_point[17]),
                (255, 255, 255), 2)
        cv2.line(image, tuple(landmark_point[17]), tuple(landmark_point[0]),
                (0, 0, 0), 6)
        cv2.line(image, tuple(landmark_point[17]), tuple(landmark_point[0]),
                (255, 255, 255), 2)

    # Key Points
    for index, landmark in enumerate(landmark_point):
        if index == 0:  
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 1:
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 2: 
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 3:  
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 4:  
            cv2.circle(image, (landmark[0], landmark[1]), 8, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 8, (0, 0, 0), 1)
        if index == 5: 
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 6: 
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 7:  
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 8: 
            cv2.circle(image, (landmark[0], landmark[1]), 8, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 8, (0, 0, 0), 1)
        if index == 9: 
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 10: 
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 11: 
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 12: 
            cv2.circle(image, (landmark[0], landmark[1]), 8, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 8, (0, 0, 0), 1)
        if index == 13:  
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 14: 
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 15:  
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 16:  
            cv2.circle(image, (landmark[0], landmark[1]), 8, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 8, (0, 0, 0), 1)
        if index == 17:  
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 18:  
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 19:  
            cv2.circle(image, (landmark[0], landmark[1]), 5, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 5, (0, 0, 0), 1)
        if index == 20:  
            cv2.circle(image, (landmark[0], landmark[1]), 8, (255, 255, 255),
                      -1)
            cv2.circle(image, (landmark[0], landmark[1]), 8, (0, 0, 0), 1)

    return image


def draw_bounding_rect(use_brect, image, brect):
    if use_brect:
        # Outer rectangle
        cv2.rectangle(image, (brect[0], brect[1]), (brect[2], brect[3]),
                     (0, 0, 0), 1)

    return image

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y


def extract_gesture(landmarks):
  #helper function, cal
  def distance(landmark1, landmark2):
    return ((landmark1.x - landmark2.x) ** 2 + (landmark1.y - landmark2.y) ** 2) ** 0.5
  
  #extracting key joints we're going to use
  wrist = landmarks[0]
  thumb_tip = landmarks[4]
  index_tip = landmarks[8]
  middle_tip = landmarks[12]
  ring_tip = landmarks[16]
  pinky_tip = landmarks[20]

  thumb_base = landmarks[2]
  index_base = landmarks[5]
  middle_base = landmarks[9]
  ring_base = landmarks[13]
  pinky_base = landmarks[17]

  #get hand size, going to use for normalizing distances
  palm_height = distance(wrist, middle_base)
  

  #get centre of palm
  palm_center_x = (wrist.x + index_base.x + middle_base.x + ring_base.x + pinky_base.x) / 5
  palm_center_y = (wrist.y + index_base.y + middle_base.y + ring_base.y + pinky_base.y) / 5
  palm_center = Point(palm_center_x, palm_center_y)

  
  # Check for open hand
  open_hand = (
    # Check that fingers are open and straight
    distance(index_tip, wrist)  > 1.25 * palm_height and
    distance(middle_tip, wrist) > 1.25 * palm_height and
    distance(ring_tip, wrist)   > 1.25 * palm_height and
    distance(pinky_tip, wrist)  > 1.25 * palm_height and
    # Check that fingers are spread 
    distance(index_tip, middle_tip) > 0.25 * palm_height and
    distance(middle_tip, ring_tip) > 0.25 * palm_height and
    distance(ring_tip, pinky_tip) > 0.25 * palm_height

  )

  # Check for index and thumb touching
  thumb_index = (
    # Checks that index and thumb are touching
    distance(thumb_tip, index_tip) < 0.1
    # checks that other fingers are open
  )
  # Middle finger and thumb touching
  thumb_middle = (
    distance(thumb_tip, middle_tip) < 0.1 
  )
  # Ring finger and thumb touching 
  thumb_ring = (
    distance(thumb_tip, ring_tip) < 0.1
  )

  # Pinky and thumb touching
  thumb_pinky = (
    distance(thumb_tip, pinky_tip) < 0.1 
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
#   if open_hand and not thumb_index and not thumb_middle and not thumb_ring and not thumb_pinky and not index_middle_thumb and not fingers_touching:
#     return "OPEN_HAND", "0000"
#   if thumb_index and not open_hand and not thumb_middle and not thumb_ring and not thumb_pinky and not index_middle_thumb and not fingers_touching:
#     return "THUMB_INDEX", "1000"
#   if thumb_middle and not open_hand and not thumb_index and not thumb_ring and not thumb_pinky and not index_middle_thumb and not fingers_touching:
#     return "THUMB_MIDDLE", "0100"
#   if thumb_ring and not open_hand and not thumb_index and not thumb_middle and not thumb_pinky and not index_middle_thumb and not fingers_touching:
#     return "THUMB_RING", "0010"
#   if thumb_pinky and not open_hand and not thumb_index and not thumb_middle and not thumb_ring and not index_middle_thumb and not fingers_touching:
#     return "THUMB_PINKY", "0001"
#   if index_middle_thumb and not open_hand and not thumb_index and not thumb_middle and not thumb_ring and not thumb_pinky and not fingers_touching:
#     return "INDEX_MIDDLE_THUMB", "0101"
#   if fingers_touching and not open_hand and not thumb_index and not thumb_middle and not thumb_ring and not thumb_pinky and not index_middle_thumb:
#     return "FINGERS_TOUCHING", "Placeholder hehe sorry Dave ;) "
  #if no gesture can be determined
  return "UNKNOWN", "UNKNOWN"


if __name__=="__main__":
  main()


