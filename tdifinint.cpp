#include <iostream>

// Function to calculate time difference in seconds
int timeDifferenceInSeconds(int h1, int m1, int s1, int h2, int m2, int s2) {
    // Convert both times into seconds
    int totalSeconds1 = h1 * 3600 + m1 * 60 + s1;
    int totalSeconds2 = h2 * 3600 + m2 * 60 + s2;

    // Calculate the time difference in seconds
    int timeDifference = totalSeconds2 - totalSeconds1;

    return timeDifference;
}

int main() {
    int h1, m1, s1, h2, m2, s2;

    // Input first time
    std::cout << "Enter first time (hours minutes seconds): ";
    std::cin >> h1 >> m1 >> s1;

    // Input second time
    std::cout << "Enter second time (hours minutes seconds): ";
    std::cin >> h2 >> m2 >> s2;

    // Calculate time difference
    int differenceInSeconds = timeDifferenceInSeconds(h1, m1, s1, h2, m2, s2);

    std::cout << "Time difference in seconds: " << differenceInSeconds << std::endl;

    return 0;
}
