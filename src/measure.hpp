#pragma once
#include "types.hpp"
#include <iostream>
#include <chrono>

typedef std::__1::chrono::steady_clock::time_point _time_point;

class CodeMeasure {
private:
    _time_point st = std::chrono::high_resolution_clock::now();
public:
    void start() {
        std::cout << "Start!" << std::endl;
        this->st = std::chrono::high_resolution_clock::now();
        std::cout << "ye" << std::endl;
    }
    f64 end() {
        //return 1;
        auto _rn = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(_rn - this->st).count();
    }
    f64 ms_end() {
        return end() / 1e6;
    }
};