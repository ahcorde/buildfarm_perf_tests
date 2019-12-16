#include "linux_memory_measurement.hpp"
#include "linux_cpu_measurement.hpp"
#include "utilities/utilities.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <boost/program_options.hpp>

int main(int argc, char* argv[])
{

  float timeout = 60;
  std::ofstream m_os;
  std::string pid_sub;
  std::string process_name;
  std::string process_arguments;

  try {
    boost::program_options::options_description desc{"Options"};

    desc.add_options()("help,h", "Help screen")("timeout",
      boost::program_options::value<float>()->default_value(30),"Test duration")
      ("log",boost::program_options::value<std::string>()->default_value("out.csv"), "Log filename")
      ("process_name", boost::program_options::value<std::string>()->default_value(""), "process_name")
      ("process_arguments", boost::program_options::value<std::string>()->default_value(""), "process_arguments");

    boost::program_options::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
      std::cout << desc << '\n';
      return 0;
    }
    if (vm.count("timeout")) {
      timeout = vm["timeout"].as<float>();
      printf("Duration of the test %5f\n", timeout);
    }

    if (vm.count("log")) {
      m_os.open(vm["log"].as<std::string>(), std::ofstream::out);
      std::cout << "file_name " << vm["log"].as<std::string>() << std::endl;
      if (m_os.is_open()) {
        m_os << "T_experiment" << ",cpu_usage (%)" << ",virtual memory (Mb)" <<
          ",physical memory (Mb)" << ",resident anonymous memory (Mb)" << std::endl;
      }
    }

    if (vm.count("process_name")) {
      process_name = vm["process_name"].as<std::string>();
      std::cout << "process_name " << process_name << std::endl;

    }
    if (vm.count("process_arguments")) {
      process_arguments = vm["process_arguments"].as<std::string>();
      std::cout << "process_arguments " << process_arguments << std::endl;
    }

  } catch (const boost::program_options::error & ex) {
    std::cerr << ex.what() << '\n';
  }

  auto start = std::chrono::high_resolution_clock::now();

  while(true) {
    pid_sub = getPIDByName(process_name, argv[0], process_arguments);
    std::cout << "pid_sub " << pid_sub << std::endl;

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = finish - start;
    float seconds_running = elapsed.count() / 1000;

    if (seconds_running > timeout) {
      return -1;
    }


    if(pid_sub.empty() || atoi(pid_sub.c_str()) < 1024) {
      std::cerr << "No process found" << std::endl;
      continue;
    }else{
      break;
    }
  }


  LinuxMemoryMeasurement linux_memory_measurement(pid_sub);
  LinuxCPUMeasurement linux_cpu_measurement(pid_sub);


  while(true) {

    linux_memory_measurement.makeReading();

    double cpu_percentage = linux_cpu_measurement.getCPUCurrentlyUsedByCurrentProcess();
    double virtual_mem_usage = linux_memory_measurement.getVirtualMemUsedProcess();
    double phy_mem_usage = linux_memory_measurement.getPhysMemUsedProcess();
    double resident_anonymousMemory_usage = linux_memory_measurement.getResidentAnonymousMemory();

    // std::cout << "totalVirtualMem: " << linux_memory_measurement.getTotalVirtualMem() << " Mb" << std::endl;
    // std::cout << "virtualMemUsed: " << linux_memory_measurement.getVirtualMemUsed() << " Mb" << std::endl;
    std::cout << "virtualMemUsed Process: " << virtual_mem_usage << " Mb" << std::endl;
    // std::cout << "TotalPhysMem: " << linux_memory_measurement.getTotalPhysMem() << " Mb" << std::endl;
    // std::cout << "PhysMemUsed: " << linux_memory_measurement.getPhysMemUsed() << " Mb" << std::endl;
    std::cout << "ResidentAnonymousMemory Process: " << resident_anonymousMemory_usage << " Mb" << std::endl;

    std::cout << "PhysMemUsedProcess: " << phy_mem_usage << " Mb" << std::endl;
    std::cout << "CPU Usage: " << cpu_percentage << " (%)" << std::endl;

    std::this_thread::sleep_for (std::chrono::seconds(1));
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = finish - start;
    float seconds_running = elapsed.count() / 1000;
    std::cout << "------------------------- " << std::endl;

    if (seconds_running > timeout) {
      break;
    }

    if (m_os.is_open()) {
      m_os << seconds_running << ", " << cpu_percentage << ", " <<
        virtual_mem_usage << ", " << phy_mem_usage << ", " << resident_anonymousMemory_usage <<
        std::endl;
    }

  }
}