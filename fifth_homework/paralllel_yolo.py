import cv2
import torch
import argparse
import time
import threading
from ultralytics import YOLO

model = YOLO('yolov8s-pose')
lock = threading.Lock()

# def draw_pose(frame, predictions):
    # for prediction in predictions:  # Используем .xyxy для получения ограничивающих рамок объектов
        # # print(prediction.keypoints)
        # if prediction.keypoints.conf == None:
            # break
        # keypoints = prediction.keypoints  # Получение ключевых точек из предсказаний
        # for i in range(len(keypoints.xy)):  # Проход по координатам ключевых точек
            # x, y, confidence = keypoints.xy[0][i][0], keypoints.xy[0][i][1], keypoints.conf[0][i].detach().float()
            # # print(x)
            # # if confidence > 0.5:  # Порог уверенности для отрисовки точки
            # cv2.circle(frame, (int(x), int(y)), 5, (255, 0, 0), -1)  # Отрисовка точки
# 
    # return frame



def process_frame(frame):
    print("Hello from process_frame")
    # Преобразование кадра в формат, который принимает модель YOLOv8sPose (например, RGB и размер 640x480)
    # frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    frame = cv2.resize(frame, (640, 480))

    with torch.no_grad():
        frame_tensor = torch.from_numpy(frame).permute(2, 0, 1).float().unsqueeze(0) / 255.0  # Приведение к формату тензора и нормализация
        predictions = model.predict(source=frame_tensor, device='cpu')[0].plot()

    # Распаковка предсказаний и отрисовка результатов (например, скелета pose)
    processed_frame = predictions#draw_pose(frame, predictions)

    return processed_frame


    
def process_frames(frames, start_frame, end_frame, result):
    # print("Hello from process frames")
    for i in range(start_frame, end_frame):
        processed_frame = process_frame(frames[i])
        with lock:
            result[i] = processed_frame

def process_video(video_path, output_file, mode):
    # model.eval()
    print("Hello from process video")

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

    if mode == "multi":
        print("Hello from multi")
        # Обработка кадра в отдельном потоке или процессе
        processed_frames = [None] * len(frames)
    
        num_threads = 4  # Количество потоков для обработки кадров
        chunk_size = len(frames) // num_threads
        print(chunk_size)
        threads = []
    
        for i in range(num_threads):
            start_index = i * chunk_size
            print(start_index)
            end_index = start_index + chunk_size if i < num_threads - 1 else len(frames)
            print(end_index)
            thread = threading.Thread(target=process_frames, args=(frames, start_index, end_index, processed_frames))
            threads.append(thread)
    
        start_time = time.time()
    
        # Запускаем потоки
        for thread in threads:
            thread.start()
    
        # Ждем завершения всех потоков
        for thread in threads:
            thread.join()
        
    else:
        for i in range(len(frames)):
            processed_frame = process_frame(frame)
            processed_frames.append(processed_frame)

    cap.release()

    end_time = time.time()
    process_time = end_time - start_time
    print(f"Processing time: {process_time} seconds")

    # Сохранение обработанного видео
    out = cv2.VideoWriter(output_file, cv2.VideoWriter_fourcc(*'mp4v'), fps, (640, 480))
    for frame in processed_frames:
        out.write(frame)
    out.release()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process video with YOLOv8sPose model')
    parser.add_argument('video', type=str, help='Path to the input video')
    parser.add_argument('--mode', type=str, default='single', help='Execution mode: single/multi')
    parser.add_argument('output', type=str, help='Path to the output video file')

    args = parser.parse_args()

    process_video(args.video, args.output, args.mode)
