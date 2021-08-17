#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <random>

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
    ARQ(void) : gen(rd()), prev_size(0) { }

    uint32_t schedule(std::vector<Inst> const & candidates)
    {
        if(candidates.size() != prev_size) {
            rand = std::uniform_int_distribution<>(0, candidates.size() - 1);
            prev_size = candidates.size();
        }

        return rand(gen);
    }

private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> rand;

    uint32_t prev_size;
};

int main(int argc, char ** argv)
{
    if(argc < 3) {
        std::cout << "Usage: " << argv[0] << " output trace [trace ...]\n";
        std::cout << "  Traces should be uncompressed.\n";
        return 0;
    }

    std::vector<std::shared_ptr<std::ifstream>> handles;
    for(int i = 2; i < argc; ++i) {
        auto handle = std::make_shared<std::ifstream>(argv[i], std::ios::binary);
        if(handle->is_open()) {
            handles.push_back(handle);
        } else {
            std::cerr << "WARN: Couldn't open " << argv[i] << " for reading\n";
        }
    }

    std::ofstream output(argv[1], std::ios::binary);
    if(! output.is_open()) {
        std::cerr << "ERROR: Couldn't open " << argv[1] << " for writing\n";
        return 0;
    }

    std::vector<Inst> thread_head;
    for(auto & handle : handles) {
        Inst inst;
        handle->read(reinterpret_cast<char *>(&inst), sizeof(Inst));
        thread_head.push_back(inst);
    }

    ARQ scheduler;

    while(! handles.empty()) {
        uint32_t thread_idx = scheduler.schedule(thread_head);
        output.write(reinterpret_cast<char *>(&thread_head[thread_idx]), sizeof(Inst));

        Inst inst;
        if(handles[thread_idx]->read(reinterpret_cast<char *>(&inst), sizeof(Inst))) {
            thread_head[thread_idx] = inst;
        } else {
            handles[thread_idx]->close();
            handles.erase(handles.begin() + thread_idx);
            thread_head.erase(thread_head.begin() + thread_idx);
        }
    }

    output.close();
}
