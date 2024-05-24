import cv2
import torch
import argparse
import time
import threading
from ultralytics import YOLO
from queue import Queue

model = YOLO('yolov8s-pose')
lock = threading.Lock()
out_queue = Queue()

def process_frame(frame):
    frame = cv2.resize(frame, (640, 480))

    with torch.no_grad():
        frame_tensor = torch.from_numpy(frame).permute(2, 0, 1).float().unsqueeze(0) / 255.0  # Приведение к формату тензора и нормализация
        predictions = model.predict(source=frame_tensor, device='cpu')[0].plot()

    processed_frame = predictions

    return processed_frame

def process_frames(frames, start_frame, end_frame):
    
    for i in range(start_frame, end_frame):
        processed_frame = process_frame(frames[i])
        out_queue.put((processed_frame, i))
        print(i)
    # print("processed")
        # processed_frames.append(processed_frame)
    # result.extend(processed_frames)

def process_video(video_path, output_file, mode):

    cap = cv2.VideoCapture(video_path)
    fps = cap.get(cv2.CAP_PROP_FPS)

    if not cap.isOpened():
        print("Error opening video file")
        return

    start_time = time.time()
    frames = []
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break
        frames.append(frame)
    
    # result = [None]*len(frames)
    print(f"len frames: {len(frames)}")
    if mode == "multi":
        threads = []
        processed_frames = [None]*len(frames)
        num_threads = 2  

        for i in range(num_threads):
            start_index = i * (len(frames) // num_threads)
            end_index = start_index + (len(frames) // num_threads)
            # print(f"Start index {i}: {start_index}")
            # print(f"end index {i}: {end_index}")
            # thread = threading.Thread(target=process_frames, args=(frames, start_index, end_index, processed_frames))
            threads.append(threading.Thread(target=process_frames, args=(frames, start_index, end_index, )))   
        start_time = time.time()
        exit(0)
        # Ожидание завершения всех потоков
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        # print("kuku")
        while not out_queue.empty():
            # print("In while queue")
            fr, i = out_queue.get()
            processed_frames[i] = fr
            # print(f"res len: {len(processed_frames)}")
    else:
        processed_frames = []
        for frame in frames:
            processed_frame = process_frame(frame)
            processed_frames.append(processed_frame)

    cap.release()
    end_time = time.time()
    process_time = end_time - start_time
    print(f"Processing time: {process_time} seconds")

    # Сохранение обработанного видео
    out = cv2.VideoWriter(output_file, cv2.VideoWriter_fourcc(*'mp4v'), fps, (640, 480))
    for frame in processed_frames:#processed_frames
        out.write(frame)
    out.release()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process video with YOLOv8sPose model')
    parser.add_argument('video', type=str, help='Path to the input video')
    parser.add_argument('--mode', type=str, default='single', help='Execution mode: single/multi')
    parser.add_argument('output', type=str, help='Path to the output video file')

    args = parser.parse_args()

    process_video(args.video, args.output, args.mode)