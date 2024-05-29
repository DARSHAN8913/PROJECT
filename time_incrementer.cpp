#include <iostream>
#include <iomanip>

int addSeconds(int hours, int mins, int secs, int ti) {
    int total_secs = hours * 3600 + mins * 60 + secs + ti;
    int new_hours = total_secs / 3600;
    if(new_hours==24)
    {
        new_hours=0;
    }
    int remaining_secs = total_secs % 3600;
    int new_mins = remaining_secs / 60;
    int new_secs = remaining_secs % 60;
    return new_hours * 10000 + new_mins * 100 + new_secs;
}

int main() {
    int hours = 23;
    int mins = 58;
    int secs = 30;
    int ti = 150;

    int new_time = addSeconds(hours, mins, secs, ti);
    std::cout << std::setw(6) << std::setfill('0') << new_time << std::endl; // Output will be an integer representing the new time in the format you described

    return 0;
}
