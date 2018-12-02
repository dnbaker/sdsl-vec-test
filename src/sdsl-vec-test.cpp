#include "../include/sdsl/coder_elias_delta.hpp"
#include "../include/sdsl/coder_elias_gamma.hpp"
#include "../include/sdsl/coder_fibonacci.hpp"
#include "../include/sdsl/dac_vector.hpp"
#include "../include/sdsl/enc_vector.hpp"
#include "../include/sdsl/vlc_vector.hpp"
#include <cstdint>
#include <random>
#include <chrono>
#include <string>

class Timer {
    using TpType = std::chrono::system_clock::time_point;
    std::string name_;
    TpType start_, stop_;
public:
    Timer(std::string &&name=""): name_{std::move(name)}, start_(std::chrono::system_clock::now()) {
        ::std::cerr << "Constructed with name " << name_ << '\n';
    }
    void stop() {stop_ = std::chrono::system_clock::now();}
    void restart() {start_ = std::chrono::system_clock::now();}
    double report() {
        const double nns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_ - start_).count();
        std::cerr << "Took " << nns << "ns for task '" << name_ << "'\n";
        return nns;
    }
    ~Timer() {stop(); /* hammertime */ report();}
    void rename(const char *name) {name_ = name;}
};

template<typename T> size_t get_nbytes(const T &c) {
    std::ofstream ofs("/dev/null");
    return c.serialize(ofs);
}
template<> size_t get_nbytes(const std::vector<uint64_t> &c) {
    return c.size() * sizeof(c[0]);
}

template<typename CompressedVectorType>
size_t perform_comparison(const std::vector<std::uint64_t> &vec, std::string name, bool is_second_pass, size_t unc=0) {
    CompressedVectorType yo(vec);
    size_t nb = get_nbytes(yo), vmem = sizeof(vec[0]) * vec.size();
    std::mt19937_64 mt(137);
    std::vector<std::uint64_t> idxs(50000);
    for(auto &e: idxs) e = mt() % yo.size();
    using Tp = std::chrono::system_clock::time_point;
    Tp start = std::chrono::system_clock::now();
    Timer t(std::string(name));
    size_t sum = 0;
    for(size_t i = 0; i < idxs.size(); ++i) {
        sum += yo[idxs[i]];
    }
    Tp stop = std::chrono::system_clock::now();
    mt.seed(sum);
    const uint64_t ret = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    name += is_second_pass ? ".sorted": ".random";
    name += (mt() & 0xFFFFFFFFFFFFuL) ? "": "."; // Force evaluation
    std::cout << name << '\t' << nb << '\t' << ret << '\t' << (nb * 100. / vmem) << "%\t" << (100. * ret / (unc ? unc: ret)) << "%\n";
    return ret;
}

int main(int argc, char *argv[]) {
    auto nelem = argc == 1 ? static_cast<unsigned long long>(5000000): std::strtoull(argv[1], nullptr, 10);
    std::mt19937_64 mt(1337);
    std::vector<std::uint64_t> data(nelem);
    for(auto &el: data) el = mt();
    std::cout << "#method\tnum_bytes\tns_required\t%%memory_vs_unc\t%%ns_vs_unc\n";
    for(int i = 0; i < 2; ++i) {
        size_t unc = perform_comparison<std::vector<uint64_t>>(data, "uncompressed", i);
        perform_comparison<sdsl::enc_vector<>>(data, "encdefault", i, unc);
        perform_comparison<sdsl::enc_vector<sdsl::coder::elias_delta>>(data, "enced", i, unc);
        perform_comparison<sdsl::enc_vector<sdsl::coder::elias_gamma>>(data, "enceg", i, unc);
        perform_comparison<sdsl::enc_vector<sdsl::coder::fibonacci>>(data, "fib", i, unc);
        perform_comparison<sdsl::vlc_vector<>>(data, "vlcdefault", i, unc);
        perform_comparison<sdsl::vlc_vector<sdsl::coder::fibonacci>>(data, "vlcfib", i, unc);
        perform_comparison<sdsl::vlc_vector<sdsl::coder::elias_delta>>(data, "vlced", i, unc);
        perform_comparison<sdsl::vlc_vector<sdsl::coder::elias_gamma>>(data, "vlceg", i, unc);
        perform_comparison<sdsl::dac_vector<>>(data, "dacdefault", i, unc);
        perform_comparison<sdsl::dac_vector<8>>(data, "dac8", i, unc);
        perform_comparison<sdsl::dac_vector<16>>(data, "dac16", i, unc);
        perform_comparison<sdsl::dac_vector<32>>(data, "dac32", i, unc);
        perform_comparison<sdsl::dac_vector<2>>(data, "dac2", i, unc);
        std::iota(data.begin(), data.end(), 0);
    }
}
