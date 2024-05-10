#include <stdio.h>
#include <iostream>

namespace GameEngine{
    class Windows {
        
    public:
        Windows(int screenWidth, int screenHeight);
        
        ~Windows();
        
        void update() const;
        
        void init();
        
    private:
        int screenWidth;
        int screenHeight;
    };
}
