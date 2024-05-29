#include <iostream>

int totalSeconds(int time) {
    int hours = time / 10000;
    int minutes = (time / 100) % 100;
    int seconds = time % 100;

    return hours * 3600 + minutes * 60 + seconds;
}

int main() {
    int time = 64500; // Example time in the format HHMMSS
    int seconds = totalSeconds(time);
    std::cout << "Total seconds: " << seconds << std::endl;
    return 0;
}
