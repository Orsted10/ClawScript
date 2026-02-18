 #include "observability/profiler.h"
 #include "interpreter/interpreter.h"
 #include "interpreter/stack_trace.h"
 #include <fstream>
 #include <chrono>
 #include <sstream>
 #include <iomanip>
 namespace volt {
 static Profiler* g_profiler = nullptr;
 Profiler& Profiler::instance() {
     if (!g_profiler) g_profiler = new Profiler();
     return *g_profiler;
 }
 Profiler::Profiler() : enabled_(false), running_(false), periodMs_(10), interp_(nullptr) {}
 Profiler::~Profiler() { stop(); }
 void Profiler::setInterpreter(Interpreter* p) { std::lock_guard<std::mutex> lk(mu_); interp_ = p; }
 void Profiler::setOutputPath(const std::string& p) { std::lock_guard<std::mutex> lk(mu_); outPath_ = p; }
 void Profiler::start(int hz) {
     std::lock_guard<std::mutex> lk(mu_);
     if (running_) return;
     periodMs_ = hz > 0 ? std::max(1, 1000 / hz) : 10;
     enabled_.store(true, std::memory_order_relaxed);
     running_.store(true, std::memory_order_relaxed);
     th_ = std::thread([this]{ run(); });
 }
 void Profiler::stop() {
     {
         std::lock_guard<std::mutex> lk(mu_);
         if (!running_) return;
         running_.store(false, std::memory_order_relaxed);
     }
     if (th_.joinable()) th_.join();
 }
 void Profiler::pause() { enabled_.store(false, std::memory_order_relaxed); }
 void Profiler::resume() { enabled_.store(true, std::memory_order_relaxed); }
 bool Profiler::isEnabled() const { return enabled_.load(std::memory_order_relaxed); }
 void Profiler::run() {
     while (running_.load(std::memory_order_relaxed)) {
         sampleOnce();
         std::this_thread::sleep_for(std::chrono::milliseconds(periodMs_));
     }
 }
 void Profiler::sampleOnce() {
     if (!enabled_.load(std::memory_order_relaxed)) return;
     Interpreter* cur = nullptr;
     {
         std::lock_guard<std::mutex> lk(mu_);
         cur = interp_;
     }
     if (!cur) return;
     const auto& frames = cur->getCallStack().get_frames();
     std::ostringstream oss;
     if (!frames.empty()) {
         for (size_t i = 0; i < frames.size(); ++i) {
             if (i) oss << ';';
             oss << frames[i].function_name;
         }
     } else {
         oss << "<idle>";
     }
     std::string key = oss.str();
     std::lock_guard<std::mutex> lk(mu_);
     cpuStacks_[key] += 1;
 }
 void Profiler::recordAlloc(size_t bytes, const char* kind) {
     if (!isEnabled()) return;
     Interpreter* cur = nullptr;
     {
         std::lock_guard<std::mutex> lk(mu_);
         cur = interp_;
     }
     std::ostringstream oss;
     if (cur) {
         const auto& frames = cur->getCallStack().get_frames();
         if (!frames.empty()) {
             for (size_t i = 0; i < frames.size(); ++i) {
                 if (i) oss << ';';
                 oss << frames[i].function_name;
             }
         } else {
             oss << "<top>";
         }
     } else {
         oss << "<unknown>";
     }
     oss << ';' << (kind ? kind : "alloc");
     std::string key = oss.str();
     std::lock_guard<std::mutex> lk(mu_);
     heapStacks_[key] += static_cast<uint64_t>(bytes);
 }
 void Profiler::buildTreeHtml(const std::unordered_map<std::string, uint64_t>& data, const char* title, const char* unit, std::string& out) const {
     struct Node { std::string name; uint64_t count = 0; std::vector<std::shared_ptr<Node>> children; };
     auto ensureChild = [](std::shared_ptr<Node>& n, const std::string& name) {
         for (auto& c : n->children) if (c->name == name) return c;
         auto c = std::make_shared<Node>(); c->name = name; n->children.push_back(c); return c;
     };
     auto root = std::make_shared<Node>(); root->name = "<root>";
     uint64_t total = 0;
     for (auto& kv : data) {
         total += kv.second;
         std::shared_ptr<Node> cur = root;
         std::stringstream ss(kv.first);
         std::string seg;
         while (std::getline(ss, seg, ';')) {
             cur->count += kv.second;
             cur = ensureChild(cur, seg);
         }
         cur->count += kv.second;
     }
     out.append("<div class=\"section\"><h2>");
     out.append(title);
     out.append("</h2><div class=\"flame\">");
     std::function<void(const std::shared_ptr<Node>&, double, double, int)> emit = [&](const std::shared_ptr<Node>& n, double left, double width, int depth) {
         if (n->children.empty()) return;
         double x = left;
         for (auto& c : n->children) {
             double w = width * (total ? static_cast<double>(c->count) / static_cast<double>(n->count) : 0.0);
             out.append("<div class=\"frame\" style=\"left:");
             out.append(std::to_string(x));
             out.append("%;width:");
             out.append(std::to_string(w));
             out.append("%;top:");
             out.append(std::to_string(depth * 24));
             out.append("px\">");
             out.append(c->name);
             out.append(" (");
             out.append(std::to_string(c->count));
             out.append(" ");
             out.append(unit);
             out.append(")</div>");
             emit(c, x, w, depth + 1);
             x += w;
         }
     };
     emit(root, 0.0, 100.0, 0);
     out.append("</div></div>");
 }
 void Profiler::writeHtml(const std::string& path) {
     std::unordered_map<std::string, uint64_t> cpu;
     std::unordered_map<std::string, uint64_t> heap;
     {
         std::lock_guard<std::mutex> lk(mu_);
         cpu = cpuStacks_;
         heap = heapStacks_;
     }
     std::string html;
     html.reserve(1 << 16);
     html.append("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>VoltScript Profile</title><style>");
     html.append("body{font-family:Segoe UI,Arial,sans-serif;margin:16px}h1{margin:0 0 8px}h2{margin:16px 0 8px;font-size:16px} .flame{position:relative;border:1px solid #ccc;height:400px;background:#fafafa} .frame{position:absolute;height:22px;overflow:hidden;white-space:nowrap;text-overflow:ellipsis;border:1px solid #fff;background:#fd7;padding:2px;font-size:12px}");
     html.append("</style></head><body><h1>VoltScript Profile</h1>");
     buildTreeHtml(cpu, "CPU samples", "samples", html);
     buildTreeHtml(heap, "Heap allocations", "bytes", html);
     html.append("</body></html>");
     std::string outFile = path.empty() ? outPath_ : path;
     if (outFile.empty()) outFile = "volt_profile.html";
     std::ofstream f(outFile, std::ios::binary);
     f.write(html.data(), static_cast<std::streamsize>(html.size()));
     // Also emit folded stacks for perf/FlameGraph integration
     auto writeFolded = [](const std::string& base, const char* suffix, const std::unordered_map<std::string, uint64_t>& m) {
         std::string foldedPath = base;
         foldedPath += suffix;
         std::ofstream fo(foldedPath, std::ios::binary);
         for (auto& kv : m) {
             fo << kv.first << ' ' << kv.second << "\n";
         }
     };
     // Derive base path without extension for folded outputs
     std::string base = outFile;
     auto pos = base.find_last_of('.');
     if (pos != std::string::npos) base.erase(pos);
     writeFolded(base, ".cpu.folded", cpu);
     writeFolded(base, ".heap.folded", heap);
 }
 static std::string jsonEscape(const std::string& s) {
     std::string o;
     o.reserve(s.size() + 8);
     for (char c : s) {
         if (c == '\"') o += "\\\"";
         else if (c == '\\') o += "\\\\";
         else if (c == '\n') o += "\\n";
         else o += c;
     }
     return o;
 }
 void Profiler::writeSpeedscope(const std::string& path) {
     std::unordered_map<std::string, uint64_t> cpu;
     std::unordered_map<std::string, uint64_t> heap;
     {
         std::lock_guard<std::mutex> lk(mu_);
         cpu = cpuStacks_;
         heap = heapStacks_;
     }
     std::unordered_map<std::string, int> frameIndex;
     std::vector<std::string> frames;
     auto indexFrame = [&](const std::string& name) {
         auto it = frameIndex.find(name);
         if (it != frameIndex.end()) return it->second;
         int idx = static_cast<int>(frames.size());
         frames.push_back(name);
         frameIndex[name] = idx;
         return idx;
     };
     struct ProfileData { std::vector<std::vector<int>> samples; std::vector<uint64_t> weights; uint64_t total = 0; };
     ProfileData cpuProf, heapProf;
     auto addStacks = [&](const std::unordered_map<std::string, uint64_t>& data, ProfileData& out) {
         for (auto& kv : data) {
             std::vector<int> sample;
             std::stringstream ss(kv.first);
             std::string seg;
             while (std::getline(ss, seg, ';')) {
                 sample.push_back(indexFrame(seg));
             }
             out.samples.push_back(std::move(sample));
             out.weights.push_back(kv.second);
             out.total += kv.second;
         }
     };
     addStacks(cpu, cpuProf);
     addStacks(heap, heapProf);
     std::string base = path;
     std::string outFile = base.empty() ? outPath_ : base;
     if (outFile.empty()) outFile = "volt_profile.json";
     auto pos = outFile.find_last_of('.');
     if (pos != std::string::npos) outFile.erase(pos);
     outFile += ".speedscope.json";
     std::ofstream f(outFile, std::ios::binary);
     f << "{\n";
     f << "  \"$schema\": \"https://www.speedscope.app/file-format-schema.json\",\n";
     f << "  \"shared\": { \"frames\": [";
     for (size_t i = 0; i < frames.size(); ++i) {
         if (i) f << ",";
         f << "{\"name\":\"" << jsonEscape(frames[i]) << "\"}";
     }
     f << "] },\n";
     f << "  \"profiles\": [\n";
     f << "    {\"type\":\"sampled\",\"name\":\"CPU samples\",\"unit\":\"samples\",\"startValue\":0,\"endValue\":" << cpuProf.total << ",\"samples\":[";
     for (size_t i = 0; i < cpuProf.samples.size(); ++i) {
         if (i) f << ",";
         f << "[";
         for (size_t j = 0; j < cpuProf.samples[i].size(); ++j) {
             if (j) f << ",";
             f << cpuProf.samples[i][j];
         }
         f << "]";
     }
     f << "],\"weights\":[";
     for (size_t i = 0; i < cpuProf.weights.size(); ++i) {
         if (i) f << ",";
         f << cpuProf.weights[i];
     }
     f << "]},\n";
     f << "    {\"type\":\"sampled\",\"name\":\"Heap allocations\",\"unit\":\"bytes\",\"startValue\":0,\"endValue\":" << heapProf.total << ",\"samples\":[";
     for (size_t i = 0; i < heapProf.samples.size(); ++i) {
         if (i) f << ",";
         f << "[";
         for (size_t j = 0; j < heapProf.samples[i].size(); ++j) {
             if (j) f << ",";
             f << heapProf.samples[i][j];
         }
         f << "]";
     }
     f << "],\"weights\":[";
     for (size_t i = 0; i < heapProf.weights.size(); ++i) {
         if (i) f << ",";
         f << heapProf.weights[i];
     }
     f << "]}\n";
     f << "  ]\n";
     f << "}\n";
 }
 void profilerSetCurrentInterpreter(Interpreter* p) { Profiler::instance().setInterpreter(p); }
 void profilerStart(int hz) { Profiler::instance().start(hz); }
 void profilerStop() { Profiler::instance().stop(); }
 void profilerPause() { Profiler::instance().pause(); }
 void profilerResume() { Profiler::instance().resume(); }
 void profilerRecordAlloc(size_t bytes, const char* kind) { Profiler::instance().recordAlloc(bytes, kind); }
 bool profilerEnabled() { return Profiler::instance().isEnabled(); }
 }
