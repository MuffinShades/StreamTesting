#include "types.hpp"
#include <iostream>

class Hydra {
    template<class _T> static bool MultiExe(_T* fn, size_t nThreads);
};