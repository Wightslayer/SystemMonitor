#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

#include <iostream>

// NOTE: To convert strings to numbers, I used: 
// https://knowledge.udacity.com/questions/668887

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  // To aggregate the memory utilization, we compute the
  // percentage of used memory with respect to the total memory.

  long mem_total, mem_free;
  string line, mem_part, mem_number, kb;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  
  if (stream.is_open()) {
    // get MemTotal
    std::getline(stream, line);
    std::istringstream ls_mem_total(line);  // Cannot redeclare, so using 2 names
    ls_mem_total >> mem_part >> mem_number >> kb;
    mem_total = std::stol(mem_number);

    // get MemFree
    std::getline(stream, line);
    std::istringstream ls_mem_free(line);  // Cannot redeclare, so using 2 names
    ls_mem_free >> mem_part >> mem_number >> kb;
    mem_free = std::stol(mem_number);
  }

  // Compute the aggregated memory utilization
  float mem_utilization = (float) (mem_total - mem_free) / mem_total;

  return mem_utilization;
}

long LinuxParser::UpTime() { 

  string line, uptime_seconds, uptime_idle;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  
  // Get uptime in seconds
  if (stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime_seconds >> uptime_idle;
  }

  // Return uptime as a long
  return std::stol(uptime_seconds);
}

// I went with an approach that directly computes the utilization without the jiffies.
// This is possible according to: https://knowledge.udacity.com/questions/730750
long LinuxParser::Jiffies() { return 0; }
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }
long LinuxParser::ActiveJiffies() { return 0; }
long LinuxParser::IdleJiffies() { return 0; }

vector<string> LinuxParser::CpuUtilization() {
  
  vector<string> utilization;
  string line, utilization_part;
  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);

    // First part is just 'cpu'
    linestream >> utilization_part; 

    // Get the aggregated CPU information
    for (int i=0; i < 10; i++){
      linestream >> utilization_part;
      utilization.push_back(utilization_part);
    }
  }

  return utilization;
}

int LinuxParser::TotalProcesses() { 

  string line, key, value, processes_count;
  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.is_open()){
    while(std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if(key == "processes"){
        processes_count = value;
        break;
      }
    }
  }

 return std::stoi(processes_count);
}

int LinuxParser::RunningProcesses() {

  string line, key, value, running_count;
  std::ifstream stream(kProcDirectory + kStatFilename);

  if (stream.is_open()){
    while(std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if(key == "procs_running"){
        running_count = value;
        break;
      }
    }
  }

 return std::stoi(running_count);
}

string LinuxParser::Command(int pid[[maybe_unused]]) {

  string command;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);

  if (stream.is_open()){
    std::getline(stream, command);
  }
  
  return command;
}

string LinuxParser::Ram(int pid) {

  string line, key, value, VmSize;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);

  if (stream.is_open()){
    while(std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmSize:"){
        VmSize = value;
        break;
      }
    }
  }

  return VmSize;
}


// Using the by the course provided github link to get the required values:
// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
vector<string> LinuxParser::CpuUtilization(int pid){
  // Overload function to also retrieve the process CPU utilization.

  vector<string> utilization;
  string line, utilization_part, dummy;
  std::ifstream stat_stream(kProcDirectory + std::to_string(pid) + kStatFilename);

  // Get the system uptime
  long uptime = LinuxParser::UpTime();
  utilization.push_back(std::to_string(uptime));

  if (stat_stream.is_open()){
    std::getline(stat_stream, line);
    std::istringstream linestream(line);

    // Values that we dont need
    for (int i=0; i < 13; i++){
      linestream >> dummy;
    }
    
    // #14 to #17. Utime, stime, cutime, cstime.
    for (int i=0; i < 4; i++){
      linestream >> utilization_part;
      utilization.push_back(utilization_part);
    }

    // More values we dont need
    for (int i=0; i < 4; i++){
      linestream >> dummy;
    }

    // #22 starttime
    linestream >> utilization_part;
    utilization.push_back(utilization_part);
  }

  return utilization;
}

string LinuxParser::Uid(int pid) {

  string line, key, value, uid;
  std::ifstream status_stream(kProcDirectory + std::to_string(pid) + kStatusFilename);

  if (status_stream.is_open()){
    while(std::getline(status_stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if(key == "Uid:"){
        uid = value;
        break;
      }
    }
  }

  return uid;
}

string LinuxParser::User(int pid) { 

  string line, key, value, dummy, uid, name;
  std::ifstream passwd_stream(kPasswordPath);

  // Get the uid
  uid = LinuxParser::Uid(pid);

  // get the user name
  if (passwd_stream.is_open()){
    while(std::getline(passwd_stream, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
    
      // We use dummy to catch the 'x' that is on each line
      linestream >> key >> dummy >> value;
      if(value == uid){
        name = key;
        break;
      }
    }
  }

  // Return the name of the user that launched this process
  return name;
}

// The process class reads the start time once on initialization and then
// only needs system uptime when computing its uptime.
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return -1; }

long LinuxParser::StartTime(int pid) {
  // Computes the process start time in seconds.
  // Returns this start time.

  string line, starttime, dummy;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);

  if (stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);

    // Values that we dont need
    for (int i=0; i < 21; i++){
      linestream >> dummy;
    }
    
    // #22 starttime
    linestream >> starttime;
  }

  // Compute process start time in seconds
  long process_starttime = std::stol(starttime) / sysconf(_SC_CLK_TCK);

  // Return start time
  return process_starttime;
}
