#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <sys/types.h>
#include <unistd.h>
# if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
    #include <sys/syscall.h>
    #define gettid() syscall(SYS_gettid)
#endif
#include <signal.h>

static constexpr uint32_t NumInstrDestinations = 2;
static constexpr uint32_t NumInstrSources = 4;

struct Inst {
    unsigned long long int ip;

    unsigned char is_branch;
    unsigned char branch_taken;

    unsigned char destination_registers[NumInstrDestinations];
    unsigned char source_registers[NumInstrSources];

    unsigned long long int destination_memory[NumInstrDestinations];
    unsigned long long int source_memory[NumInstrSources];
};

class ARQ
{
public:
    ARQ(void) : gen{static_cast<long unsigned int>(gettid())},
                current(0),
                time_left(0)
    {}

    // currently does not inspect the candidates
    // (the pre-buffering of the instructions was removed for now, anyway)
    uint32_t schedule(std::vector<Inst> const & candidates)
    {
        if (time_left == 0) {
            current = (current+1) % rand.size();
            time_left = rand[current](gen);
            std::cerr << "thread "<<current<<" for "<<time_left<<" instructions\n";
        }

        ++inst_count[current];
        --time_left;
        return current;
    }
    void new_thread(double average_sojourn /* instructions */) {
        rand.push_back(std::exponential_distribution<>(1 / average_sojourn));
        inst_count.push_back(0);
        ++current;
    }
    void report() {
        std::cerr << "\n";
        for (unsigned i=0; i<inst_count.size(); ++i) {
            std::cerr << "thread "<<i<<": "<<inst_count[i]<<" instructions\n";
        }
    }

private:
    std::default_random_engine gen;
    std::vector< std::exponential_distribution<> > rand;
    std::vector< uint64_t > inst_count;

    unsigned current;
    uint64_t time_left;
};

int main(int argc, char ** argv)
{
    if(argc < 3) {
        std::cout << "Usage: " << argv[0] << " output trace [[trace avg-inst-length] ...]\n";
        std::cout << "  Traces should be uncompressed.\n";
        return 0;
    }

    ARQ scheduler;

    std::vector<std::shared_ptr<std::ifstream>> handles;
    for(int i = 2; i < argc; ++i) {
        auto handle = std::make_shared<std::ifstream>(argv[i], std::ios::binary);
        if(handle->is_open()) {
            handles.push_back(handle);
        } else {
            std::cerr << "WARN: Couldn't open " << argv[i] << " for reading\n";
        }
        ++i;
        scheduler.new_thread(atof(argv[i]));
    }

    std::ofstream output(argv[1], std::ios::binary);
    if(! output.is_open()) {
        std::cerr << "ERROR: Couldn't open " << argv[1] << " for writing\n";
        return 0;
    }

    // let us detect ChampSim terminating so we can display our own stats
    signal(SIGPIPE, SIG_IGN);

    std::vector<Inst> thread_head;
#if 0
    for(auto & handle : handles) {
        Inst inst;
        handle->read(reinterpret_cast<char *>(&inst), sizeof(Inst));
        thread_head.push_back(inst);
    }
#endif

    for(;;) {
        Inst inst;

        uint32_t thread_idx = scheduler.schedule(thread_head);
        if (handles[thread_idx]->read(reinterpret_cast<char *>(&inst), sizeof(Inst))) {
            inst.is_branch |= (thread_idx & 0x7F) << 1;
            output.write(reinterpret_cast<char *>(&inst), sizeof(Inst));
            if (output.bad()) {
                output.close();
                break;
            }
        } else {
            std::cerr << "input exhausted for thread " << thread_idx << "\n";
            output.close();
            break;
        }
    }
    scheduler.report();
}
