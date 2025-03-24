Dependencies:
  - python3.9, numpy, opencv-python, mediapipe
To run:
  python mediapipe_handtrack.py 
  Optional flags:
	 --device int,                              specifies the camera device number       (default 0)
	 --width  int,                              specifies camera width you want          (default 640)
	 --height int,                              specifies camera height                  (default 480)
	 --use_static_image_mode,                   specifies video or image processing mode (calling flag evals True)
	 --min_detection_confidence float[0.0-1.0], specifies detection threshold            (default 0.7)
	 --minh_tracking_confidence float[0.0-1.0], specifies tracking threshold             (default 0.5)
