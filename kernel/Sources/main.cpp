#include <memory_management.hpp>
#include <debug.hpp>

extern "C" void kernel_main(void) {
    Debug::Initialize();
    Debug::SetPosition(0 , 0);
    
    Memory::Initialize();
    
    while(1) {
        ;
    }
}