#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) {

    // Constants used to extract and remove hours/minutes from seconds
    const long seconds_in_hour = 3600;
    const long seconds_in_minute = 60;

    // In C++, dividing a long by a long yields another long
    // The floating point 'remainder' is automatically discarded
    const long hours = seconds / seconds_in_hour;
    seconds = seconds - hours * seconds_in_hour;

    const long minutes = seconds / seconds_in_minute;
    seconds = seconds - minutes * seconds_in_minute;

    // Convert the numbers to string
    string strhours = std::to_string(hours);
    string strminutes = std::to_string(minutes);
    string strseconds = std::to_string(seconds);

    // Convert H:M:S to HH:MM:SS
    if (hours < 10){
        strhours = "0" + strhours;
    }
    if (minutes < 10){
        strminutes = "0" + strminutes;
    }
    if (seconds < 10){
        strseconds = "0" + strseconds;
    }

    // Return the time as HH:MM:SS
    return strhours + ":" + strminutes + ":" + strseconds;
}