#include <iostream>
#include <sstream>

using namespace std;

// Function to convert time string to seconds
int timeStringToSeconds(const string& timeStr) {
    int hours, minutes, seconds;
    char colon;
    istringstream ss(timeStr);
    ss >> hours >> colon >> minutes >> colon >> seconds;
    return hours * 3600 + minutes * 60 + seconds;
}

// Function to calculate time difference in seconds
int timeDifferenceInSeconds(const string& timeStr1, const string& timeStr2) {
    int time1 = timeStringToSeconds(timeStr1);
    int time2 = timeStringToSeconds(timeStr2);
    return (time2 - time1);
}

int main() {
    string timeStr1 = "21:30:45";
    string timeStr2 = "22:32:10";

    int difference = timeDifferenceInSeconds(timeStr1, timeStr2);
    cout << "Time difference between " << timeStr1 << " and " << timeStr2 << " is " << difference << " seconds." << endl;

    return 0;
