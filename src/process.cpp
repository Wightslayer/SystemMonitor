#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid){
    pid_ = pid;

    // Some variables that dont change, so better to initialize once instead of calling
    // LinuxParser everytime we need them.
    uid_ = LinuxParser::Uid(pid_);
    user_ = LinuxParser::User(pid_);
    command_ = LinuxParser::Command(pid_);
    starttime_ = LinuxParser::StartTime(pid_);
}

int Process::Pid() {
    return pid_; 
}

// Using the in the course provided stackoverflow post:
// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
float Process::CpuUtilization() {
    // Returns the CPU Utilization of the process as a value between 0. and 1.

    vector<string> utilization = LinuxParser::CpuUtilization(pid_);
    float uptime = std::stof(utilization[0]);
    float utime = std::stof(utilization[1]);
    float stime = std::stof(utilization[2]);
    float cutime = std::stof(utilization[3]);
    float cstime = std::stof(utilization[4]);
    float starttime = std::stof(utilization[5]);

    float Hertz = (float)sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime / Hertz);

    // No need to multiply by 100 because:
    // "You need to take care of the fact that the cpu utilization has already been multiplied by 100." in ncurses_display.cpp
    // And also answered in this Knowledge question: https://knowledge.udacity.com/questions/502811
    float cpu_usage = (total_time / Hertz) / seconds;
    
    // Store it as a class variable so that we can use it with the overloaded '<' operator.
    cpu_usage_ = cpu_usage;

    return cpu_usage_;
}

string Process::Command() {
    return command_;
}

string Process::Ram() {
    string ram_str = LinuxParser::Ram(pid_);
    int ram_int = std::stoi(ram_str);
    ram_int /= 1000;  // Convert from KB to MB
    
    return std::to_string(ram_int);
}

string Process::User() {
    return user_; 
}

long int Process::UpTime() {
    // Get the system uptime
    long system_uptime = LinuxParser::UpTime();

    // Return process uptime
    return system_uptime - starttime_;
}

bool Process::operator<(Process const& a) const {
    // Using the process CPU utilization for the comparison
    return a.cpu_usage_ < cpu_usage_;
}