#include "Windows.h"
#include <GL/glew.h>
#include <GLUT/glut.h>

namespace GameEngine{
    
    Windows::Windows(int screenWidth, int screenHeight){
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;
    }
    
    Windows::~Windows(){
        
    }
    
    void Windows::update() const{
        
    }
    
    void Windows::init(){
        ;
    }
}
