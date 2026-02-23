 #pragma once
 #include <atomic>
 #include <thread>
 #include <mutex>
 #include <unordered_map>
 #include <string>
 #include <vector>
 #include <memory>
 namespace claw {
 class Interpreter;
 class Profiler {
 public:
     static Profiler& instance();
     void setInterpreter(Interpreter* p);
     void setOutputPath(const std::string& p);
     void start(int hz);
     void stop();
     void pause();
     void resume();
     void recordAlloc(size_t bytes, const char* kind);
     bool isEnabled() const;
     void writeHtml(const std::string& path);
     void writeSpeedscope(const std::string& path);
 private:
     Profiler();
     ~Profiler();
     void run();
     void sampleOnce();
     std::string collapseStack() const;
     void buildTreeHtml(const std::unordered_map<std::string, uint64_t>& data, const char* title, const char* unit, std::string& out) const;
     std::atomic<bool> enabled_;
     std::atomic<bool> running_;
     int periodMs_;
     std::thread th_;
     mutable std::mutex mu_;
     Interpreter* interp_;
     std::unordered_map<std::string, uint64_t> cpuStacks_;
     std::unordered_map<std::string, uint64_t> heapStacks_;
     std::string outPath_;
 };
 void profilerSetCurrentInterpreter(Interpreter* p);
 void profilerStart(int hz);
 void profilerStop();
 void profilerPause();
 void profilerResume();
 void profilerRecordAlloc(size_t bytes, const char* kind);
 bool profilerEnabled();
 }
