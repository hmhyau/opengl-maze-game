//!Includes
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include "Shader.h"
#include "Vector.h"
#include "Matrix.h"
#include "Mesh.h"
#include "Texture.h"
#include "SphericalCameraManipulator.h"
#include <iostream>
#include <math.h>
#include <string>
#include "common.h"
#include "Windows.h"

Matrix4x4 TankMVMatrix;
float lasttime = 0.0f;
GLfloat speed = 0.25f;
float cameraposx = -0.0f, cameraposy = 10.0f, cameraposz = -15.0f;
float camerafrontx = 0.0f, camerafronty = 0.0f, camerafrontz = 0.0f;
Vector3f camerapos = Vector3f(cameraposx, cameraposy, cameraposz);
Vector3f camerafront = Vector3f(camerafrontx,camerafronty,camerafrontz);
Vector3f cameraup = Vector3f(0.0f,1.0f,0.0f);
float cameradistance = camerapos.length();

float toRadian(float degree){
    return degree * PI / 180;
}

void readMaze(){
    using namespace std;
    
    ifstream infile, level2;
    infile.open("../MazeData.txt");
    level2.open("../MazeData.txt");
    if(!infile.is_open())
    cerr << "Error : Maze data not located" << endl;
    
    for(int i = 0; i <= MAX_LENGTH; i++){
        for(int j = 0; j <= MAX_LENGTH; j++){
            MazeData[i][j] = 0;
            MazeData2[i][j] = 0;
        }
    }
    
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            infile >> MazeData[i][j];
            if(MazeData[i][j] == 2)
            TotalCoin++;
            level2 >> MazeData2[i][j];
            if(MazeData2[i][j] == 2)
            TotalCoin2++;
        }
    }
}

void glEnable(void){
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_SMOOTH);
}

void objectInit(void){
    chassis.loadOBJ("../models/chassis.obj");
    front_wheel.loadOBJ("../models/front_wheel.obj");
    back_wheel.loadOBJ("../models/back_wheel.obj");
    cube.loadOBJ("../models/cube.obj");
    coin.loadOBJ("../models/coin.obj");
    turret.loadOBJ("../models/turret.obj");
    ball.loadOBJ("../models/ball.obj");
    initTexture("../models/hamvee.bmp", texture[0]);
    initTexture("../models/Crate.bmp", texture[1]);
    initTexture("../models/coin.bmp", texture[2]);
    initTexture("../models/ball.bmp", texture[3]);
}

//! Main Program Entry
int main(int argc, char** argv)
{
    //init OpenGL
    if(!initGL(argc, argv))
    return -1;
    
    readMaze();
    
    //Init Key States to false;
    for(int i = 0 ; i < 256; i++)
    keyStates[i] = false;
    
    //Set up your program
    initShader();
    objectInit();
    
    //Init Camera Manipultor
    cameraManip.setPanTiltRadius(0.f,1.f,0.f);
    
    
    //Enter main loop
    glutMainLoop();
    
    //Delete shader program
    glDeleteProgram(shaderProgramID);
    
    return 0;
}

//! Function to Initlise OpenGL
bool initGL(int argc, char** argv)
{
    using namespace GameEngine;
    
    //Init GLUT
    glutInit(&argc, argv);
    
    //Set Display Mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    
    //Set Window Size
    Windows Windows(720,720);
    Windows.init();
    
    // Window Position
    glutInitWindowPosition(200, 200);
    
    //Create Window
    glutCreateWindow("Tank Assignment");
    
    // Init GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    //Set Display function
    glutDisplayFunc(display);
    
    //Set Keyboard Interaction Functions
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyUp);
    
    //Set Mouse Interaction Functions
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(motion);
    glutMotionFunc(motion);
    
    //Start start timer function after 100 milliseconds
    glutTimerFunc(10,Timer, 1);
    
    glEnable();
    return true;
}

//Init Shader
void initShader()
{
    //Create shader
    shaderProgramID = Shader::LoadFromFile("shader.vert","shader.frag");
    
    // Get a handle for our vertex position buffer
    vertexPositionAttribute = glGetAttribLocation(shaderProgramID,  "aVertexPosition");
    vertexNormalAttribute = glGetAttribLocation(shaderProgramID,    "aVertexNormal");
    vertexTexcoordAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");
    
    //!
    MVMatrixUniformLocation         = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform");
    ProjectionUniformLocation       = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform");
    TextureMapUniformLocation       = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
}


void initTexture(std::string filename, GLuint & textureID)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    
    //Get texture Data
    int width, height;
    char *data;
    Texture::LoadBMP(filename, width, height, data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //Cleanup data as copied to GPU
    delete[] data;
}

float xtank = 0.0f, ytank = 0.0f, ztank = 0.0f;
float oldtime = 0.0f;
float deltatime = 0.0f;
//! Display Loop
void display(void)
{
    
    timer = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
    
    //Set Viewport
    glViewport(0,0, screenWidth, screenHeight);
    
    //Clear the screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    //Use shader
    glUseProgram(shaderProgramID);
    handleKeys();
    //Projection Matrix - Perspective Projection
    ProjectionMatrix.perspective(90, 1.0, 0.0001, 100.0);
    glUniformMatrix4fv(ProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());
    
    glClearColor(0.2f, 0.0f, 0.65f, 0.0f);
    //Set Colour after program is in use
    
    TankRenderer();
    
    CheckLocation(xtank, ztank);
    
    MapRenderer();
    Camera();
    
    //Unuse Shader
    glUseProgram(0);
    TextRenderer();
    
    // Flush the OpenGL buffers to the window
    glFlush();
    //Swap Buffers and post redisplay
    glutSwapBuffers();
}

void MapRenderer(){
    glPushMatrix();
    for(int i = 0; i < MAX_LENGTH; i++){
        for(int j = 0; j < MAX_LENGTH; j++){
            switch (MazeData[i][j]){
                case 0: break;
                case 1:
                CubeRenderer(i, j);
                break;
                case 2:
                CubeRenderer(i, j);
                CoinRenderer(i, j);
                break;
            }
        }
    }
    glPopMatrix();
}

void CubeRenderer(int i, int j){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glUniform1i(TextureMapUniformLocation, 0);
    Matrix4x4 CubeMVMatrix;
    CubeMVMatrix.toIdentity();
    CubeMVMatrix.scale(SCALEFACTOR, SCALEFACTOR, SCALEFACTOR);
    CubeMVMatrix.translate(2*j+0.0f, -1.0f, 2*i+0.0f);
    CubeMVMatrix = ModelViewMatrix * CubeMVMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, CubeMVMatrix.getPtr());
    cube.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

void CoinRenderer(int i, int j){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glUniform1i(TextureMapUniformLocation, 0);
    Matrix4x4 CoinMVMatrix;
    CoinMVMatrix.toIdentity();
    CoinMVMatrix.scale(SCALEFACTOR, SCALEFACTOR, SCALEFACTOR);
    CoinMVMatrix.translate(2*j+0.0f, 1.0f, 2*i+0.0f);
    CoinMVMatrix.rotate(90+coinRotation, 0.0f, 1.0f, 0.0f);
    CoinMVMatrix.scale(-0.4f, -0.4f, -0.4f);
    CoinMVMatrix = ModelViewMatrix * CoinMVMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, CoinMVMatrix.getPtr());
    coin.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

void TankRenderer(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glUniform1i(TextureMapUniformLocation, 0);
    glPushMatrix();
    TankMVMatrix.toIdentity();
    TankMVMatrix.translate(xtank, -0.0f, ztank);
    TankMVMatrix.rotate(tankRotation, 0.0f, 1.0f, 0.0f);
    TankMVMatrix = ModelViewMatrix * TankMVMatrix;
    TurretRenderer();
    WheelRenderer();
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, TankMVMatrix.getPtr());
    back_wheel.Draw(vertexPositionAttribute, -1, vertexTexcoordAttribute);
    chassis.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
    glPopMatrix();
}

void WheelRenderer(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glUniform1i(TextureMapUniformLocation, 0);
    glPushMatrix();
    Matrix4x4 WheelMatrix;
    WheelMatrix.toIdentity();
    WheelMatrix.translate(xtank, 0.0f, ztank);
    WheelMatrix.rotate(tankRotation, 0.0f, 1.0f, 0.0f);
    WheelMatrix = ModelViewMatrix * WheelMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, WheelMatrix.getPtr());
    front_wheel.Draw(vertexPositionAttribute, -1, vertexTexcoordAttribute);
    glPopMatrix();
}

void TurretRenderer(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glUniform1i(TextureMapUniformLocation, 0);
    glPushMatrix();
    Matrix4x4 TurretMVMatrix, CameraTurret;
    TurretMVMatrix.toIdentity();
    TurretMVMatrix.translate(xtank, 0.0f, ztank);
    TurretMVMatrix.rotate(tankRotation, 0.0f, 1.0f, 0.0f);
    //    CameraTurret = cameraManip.apply(TurretMVMatrix);
    TurretMVMatrix = ModelViewMatrix * TurretMVMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, TurretMVMatrix.getPtr());
    turret.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
    glPopMatrix();
}

void Camera(){
    glUniformMatrix4fv(ProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());
    ModelViewMatrix.lookAt(camerapos, camerafront, cameraup);
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, ModelViewMatrix.getPtr());
    
}

void Projectile(){
    glPushMatrix();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glUniform1i(TextureMapUniformLocation, 0);
    
    glPushMatrix();
    Matrix4x4 ProjectileMatrix;
    ProjectileMatrix.toIdentity();
    ProjectileMatrix.translate(xtank, 2.0, ztank);
    ProjectileMatrix = ModelViewMatrix * ProjectileMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, ProjectileMatrix.getPtr());
    ball.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
    glPopMatrix();
}


void render2dText(std::string text, float r, float g, float b, float x, float y)
{
    glColor3f(r,g,b);
    glRasterPos2f(x, y); // window coordinates
    for(unsigned int i = 0; i < text.size(); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

void TextRenderer(){
    char TimeString[100];
    char ScoreString[100];
    char LifeString[100];
    
    sprintf(TimeString, "Time: %.3f", timer);
    sprintf(ScoreString, "Score: %d", score);
    sprintf(LifeString, "Life: %d", life);
    if(life > 0)
    render2dText(LifeString, 1.0f, 1.0f, 1.0f, 0.5f+1.0f/32.0, 0.0f);
    else
    render2dText("Game Over", 1.0f, 1.0f, 1.0f, 0.5f+1.0f/32.0, 0.0f);
    
    if(TotalCoin == score)
    render2dText("Level 2", 1.0f, 1.0f, 1.0f, -0.5f+1.0f/32.0, 0.75f);
    render2dText(TimeString, 1.0f, 1.0f, 1.0f, -1.0f+1.0f/32.0, -1.0f);
    render2dText(ScoreString, 1.0f, 1.0f, 1.0f, 0.0f+1.0f/32.0, -1.0f);
}

void CheckLocation(float x, float z){
    if(MazeData[int((z+OFFSET)/16)][int((x+OFFSET)/16)] == 2){
        MazeData[int((z+OFFSET)/16)][int((x+OFFSET)/16)] = 1;
        score++;
    }
    if(MazeData[int((z+OFFSET)/16)][int((x+OFFSET)/16)] == 0 || x < -OFFSET || z < -OFFSET){
        Respawn();
    }
}

void Respawn(){
    xtank = 0.0f;
    ytank = 0.0f;
    ztank = 0.0f;
    tankRotation = 0.0f;
    if(life > 0)
    life--;
    else{
        life = 0;
        ytank -= GRAVITY*timer;
    }
}

//! Keyboard Interaction
void keyboard(unsigned char key, int x, int y)
{
    //Quits program when esc is pressed
    if (key == 27)	//esc key code
    {
        exit(0);
    }
    //Set key status
    keyStates[key] = true;
}

//! Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
    keyStates[key] = false;
}

//! Handle Keys
void handleKeys()
{
    //keys should be handled here
    if(keyStates['w']){
        xtank += speed*sinf(toRadian(tankRotation));
        ztank += speed*cosf(toRadian(tankRotation));
        camerapos = Vector3f(xtank - cameradistance * sinf(toRadian(tankRotation)), ytank + cameraposy, ztank - cameradistance * cosf(toRadian(tankRotation)));
        camerafront = Vector3f(xtank + camerafrontx, ytank + camerafronty, ztank + camerafrontz);
        WheelRotation += 1.0f;
    }
    //    else{
    //        pos_x += cameraspeed*sinf(toRadian(tankRotation)) * timet;
    //        pos_z += cameraspeed*cosf(toRadian(tankRotation)) * timet;
    //        camerapos = Vector3f(pos_x + cameraposx, pos_y + cameraposy, pos_z + cameraposz);
    //        camerafront = Vector3f(pos_x + camerafrontx, pos_y + camerafronty, pos_z + camerafrontz);
    //    }
    
    if(keyStates['s']){
        xtank -= speed*sinf(toRadian(tankRotation));
        ztank -= speed*cosf(toRadian(tankRotation));
        camerapos = Vector3f(xtank - cameradistance * sinf(toRadian(tankRotation)), ytank + cameraposy, ztank - cameradistance * cosf(toRadian(tankRotation)));
        camerafront = Vector3f(xtank + camerafrontx, ytank + camerafronty, ztank + camerafrontz);
        WheelRotation -= 1.0f;
    }
    //    else{
    //        pos_x -= cameraspeed*sinf(toRadian(tankRotation)) * timet;
    //        pos_z -= cameraspeed*cosf(toRadian(tankRotation)) * timet;
    //        camerapos = Vector3f(pos_x + cameraposx, pos_y + cameraposy, pos_z + cameraposz);
    //        camerafront = Vector3f(pos_x + camerafrontx, pos_y + camerafronty, pos_z + camerafrontz);
    //    }
    
    if(keyStates['a'] && (keyStates['w'] || (keyStates['s']))){
        //camera_x += 0.1;
        tankRotation += 1.0f;
        //        Vector3f crosscamerafront;
        //        crosscamerafront = camerafront.cross(camerafront, cameraup);
        //        camerapos = camerapos - camerapos.normalise(crosscamerafront.operator*(cameraspeed));
    }
    
    if(keyStates['d'] && (keyStates['w'] || (keyStates['s']))){
        //camera_x -= 0.1;
        tankRotation -= 1.0f;
        //        Vector3f crosscamerafront;
        //        crosscamerafront = camerafront.cross(camerafront, cameraup);
        //        camerapos = camerapos + camerapos.normalise(crosscamerafront.operator*(cameraspeed));
    }
    
    if(keyStates['i']){
        Projectile();
        std::cout << "i is pressed" <<  std::endl;
    }
}

//! Mouse Interaction
void mouse(int button, int state, int x, int y)
{
    cameraManip.handleMouse(button, state, x, y);
}

//! Motion
void motion(int x, int y)
{
    cameraManip.handleMouseMotion(x, y);
}
//! Timer Function
void Timer(int value)
{
    printf("%f %f %f %f %d\n", xtank, ztank, timer, timet, TotalCoin);
    if(keyStates['w']){
        timet += 0.25f;
    }
    else if (!keyStates['w']){
        timet -= 0.25f;
    }
    else if(keyStates['s']){
        timet -= 0.25f;
    }
    else if(!keyStates['s']){
        timet += 0.25f;
    }
    else
    timet = 0.0f;
    
    coinRotation += 2.0f;
    glutPostRedisplay();
    //Call function again after 10 milli seconds
    glutTimerFunc(10,Timer, 1);
}
