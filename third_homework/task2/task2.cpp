#include <iostream>
#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <random>
#include <cmath>


using namespace std;
// Определение типа задачи
enum class TaskType { Sine, SquareRoot, Power };


mutex mut;
condition_variable cv;  
queue<pair<size_t, future<double>>> tasks;
unordered_map<size_t, double> results;


// Структура для представления задачи
struct Task {
    TaskType type;
    double argument;
};


void server_thread(const stop_token& stoken)
{
    unique_lock<mutex> lock_res(mut, defer_lock);
    size_t id_task;

    while (!stoken.stop_requested())
    {
        lock_res.lock();

        while (!tasks.empty()) {
            id_task = tasks.front().first;
            results.insert({id_task, tasks.front().second.get()});
            tasks.pop();
        }

        cv.notify_one();
        lock_res.unlock();
    }

    cout << "Server stop!\n";
}


// Шаблон класса сервера
template <typename T>
class Server {
public:
    void start() {
        // Запуск сервера
        std::cout<<"Start\n";
        server = std::jthread(server_thread);
    }

    void stop() {
        // Остановка сервера
        server.request_stop();
        server.join();
        std::cout<<"End\n";
    }

    size_t add_task(Task task) {
        // Добавление задачи в очередь
        std::unique_lock<std::mutex> lock(mutex);
        tasks.push(task);
        return tasks.size(); // Возвращает id задачи
    }

    T request_result(size_t id_res) {
        // Получение результата задачи
        T res = results[id_res];
        results.erase(id_res);
        return res;
    };

private:
    std::queue<Task> tasks;
    std::mutex mutex;
    std::condition_variable cv;
    std::unordered_map<size_t, T> results;
    std::jthread server;
};


Server<double> server;


// Функция для вычисления синуса
double compute_sine(double angle) {
    return std::sin(angle);
}

// Функция для вычисления квадратного корня
double compute_square_root(double number) {
    return std::sqrt(number);
}

// Функция для возведения числа в степень
double compute_power(double base, double exponent) {
    return std::pow(base, exponent);
}

// Клиентский код
int main() {
    // Создание сервера
    Server<double> server;
    server.start();

    // Создание клиентов и добавление задач
    std::thread client1([&server]() {
        for (int i = 0; i < 5; ++i) {
            Task task{TaskType::Sine, rand() % 360};
            size_t id = server.add_task(task);
            // Дождитесь результата и сохраните в файл
            double result = server.request_result(id);
            // std::cout << "client 1: " << result << std::endl;
            // Сохранение результата в файл
        }
    });

    std::thread client2([&server]() {
        for (int i = 0; i < 5; ++i) {
            Task task{TaskType::SquareRoot, rand() % 100};
            size_t id = server.add_task(task);
            // Дождитесь результата и сохраните в файл
            double result = server.request_result(id);
            // std::cout << "client2: " << result << std::endl;
            // Сохранение результата в файл
        }
    });

    std::thread client3([&server]() {
        for (int i = 0; i < 5; ++i) {
            Task task{TaskType::Power, rand() % 10};
            size_t id = server.add_task(task);
            // Дождитесь результата и сохраните в файл
            double result = server.request_result(id);
            // std::cout << "client3: " << result << std::endl;
            // Сохранение результата в файл
        }
    });

    client1.join();
    client2.join();
    client3.join();

    server.stop();

    return 0;
}
