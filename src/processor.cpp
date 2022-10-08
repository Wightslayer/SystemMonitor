#include "processor.h"

#include "linux_parser.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

float Processor::Utilization() {
    // Read the CPU stats aggregated over all CPU lines (the first row)
    vector<string> utilization = LinuxParser::CpuUtilization();

    // Extract and parse the individual numbers
    long user = std::stol(utilization[0]);
    long nice = std::stol(utilization[1]);
    long system = std::stol(utilization[2]);
    long idle = std::stol(utilization[3]);
    long iowait = std::stol(utilization[4]);
    long irq = std::stol(utilization[5]);
    long softirq = std::stol(utilization[6]);
    long steal = std::stol(utilization[7]);
    // long guest = std::stol(utilization[8]);  // Not used in linked guide
    // long guest_nice = std::stol(utilization[9]); // Also not used in linked guide

    // Compute aggregated CPU utilization according to:
    // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
    // as provided in the course.
    long all_idle, non_idle, total;

    all_idle = idle + iowait;
    non_idle = user + nice + system + irq + softirq + steal;
    total = all_idle + non_idle;

    float CPU_utilization = (float)(total - all_idle) / total;

    return CPU_utilization;
}