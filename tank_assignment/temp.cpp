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

float lasttime = 0.0f;
float speed = 0.4f, VelocityX = 0.0f, VelocityZ = 0.0f;
float CameraPosX = 0.0f, CameraPosY = 10.0f, CameraPosZ = 15.0f;
float CameraFrontX = 0.0f, CameraFrontY = 0.0f, CameraFrontZ = 0.0f;
Vector3f CameraPos = Vector3f(CameraPosX, CameraPosY, CameraPosZ);
Vector3f CameraFront = Vector3f(CameraFrontX,CameraFrontY,CameraFrontZ);
Vector3f CameraUp = Vector3f(0.0f,1.0f,0.0f);
float CameraDistance = CameraPos.length();

// Degree to Radian Conversion
float toRadian(float degree){
    return (degree * PI / 180);
}

// Read Levels
void readMaze(){
    using namespace std;
    ifstream infile, level2;
    infile.open("../MazeData.txt");
    level2.open("../MazeData2.txt");
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

// Enable OpenGL functions
void glEnable(void){
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_SMOOTH);
}

// Init Objects and Textures
void ObjectInit(void){
    chassis.loadOBJ("../models/chassis.obj");
    FrontWheel.loadOBJ("../models/front_wheel.obj");
    BackWheel.loadOBJ("../models/back_wheel.obj");
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
    ObjectInit();
    
    //Init Camera Manipultor
    cameraManip.setPanTiltRadius(0.f,1.f,0.f);
    glutIdleFunc(Update);
    startTime = glutGet(GLUT_ELAPSED_TIME);
    
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

// Init Shader
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
    LightPositionUniformLocation    = glGetUniformLocation(shaderProgramID, "LightPosition_uniform");
    AmbientUniformLocation          = glGetUniformLocation(shaderProgramID, "Ambient_uniform");
    SpecularUniformLocation         = glGetUniformLocation(shaderProgramID, "Specular_uniform");
    SpecularPowerUniformLocation    = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");
    TextureMapUniformLocation       = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
}

// Init Texture
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
    CheckLocation(TankInfo.PosX, TankInfo.PosZ, MazeData2);
    MapRenderer(MazeData2);
    
    Camera();
    
    //Unuse Shader
    glUseProgram(0);
    
    TextRenderer();
    
    // Flush the OpenGL buffers to the window
    glFlush();
    
    // Swap Buffers and post redisplay
    glutSwapBuffers();
}

// Map Related Rendering
void MapRenderer(int Maze[MAX_LENGTH][MAX_LENGTH]){
    glPushMatrix();
    for(int i = 0; i < MAX_LENGTH; i++){
        for(int j = 0; j < MAX_LENGTH; j++){
            switch (Maze[i][j]){
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
    CoinMVMatrix.rotate(coinRotation, 0.0f, 1.0f, 0.0f);
    CoinMVMatrix.scale(-0.4f, -0.4f, -0.4f);
    CoinMVMatrix = ModelViewMatrix * CoinMVMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, CoinMVMatrix.getPtr());
    coin.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

// Humvee Rendering
void TankRenderer(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glUniform1i(TextureMapUniformLocation, 0);
    glPushMatrix();
    Matrix4x4 TankMVMatrix;
    TankMVMatrix.toIdentity();
    TankMVMatrix.translate(TankInfo.PosX, TankInfo.PosY, TankInfo.PosZ);
    TankMVMatrix.rotate(TankInfo.Rotation, 0.0f, 1.0f, 0.0f);
    TankMVMatrix = ModelViewMatrix * TankMVMatrix;
    TurretRenderer();
    WheelRenderer();
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, TankMVMatrix.getPtr());
    BackWheel.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
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
    WheelMatrix.translate(TankInfo.PosX, TankInfo.PosY, TankInfo.PosZ);
    WheelMatrix.rotate(TankInfo.Rotation, 0.0f, 1.0f, 0.0f);
    WheelMatrix = ModelViewMatrix * WheelMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, WheelMatrix.getPtr());
    FrontWheel.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
    glPopMatrix();
}

void TurretRenderer(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glUniform1i(TextureMapUniformLocation, 0);
    glPushMatrix();
    Matrix4x4 TurretMVMatrix, CameraTurret;
    TurretMVMatrix.toIdentity();
    TurretMVMatrix.translate(TankInfo.PosX, TankInfo.PosY, TankInfo.PosZ);
    TurretMVMatrix.rotate(TankInfo.Rotation, 0.0f, 1.0f, 0.0f);
    CameraTurret = cameraManip.apply(TurretMVMatrix);
    cameraManip.setFocus(Vector3f(TankInfo.PosX,TankInfo.PosY, TankInfo.PosZ));
    cameraManip.setPanTiltRadius(0.0f, 0.0f , 0.0f);
    TurretMVMatrix = ModelViewMatrix * CameraTurret;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, TurretMVMatrix.getPtr());
    turret.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
    glPopMatrix();
}

// Camera related
void Camera(){
    glUniformMatrix4fv(ProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());
    ModelViewMatrix.lookAt(CameraPos, CameraFront, CameraUp);
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, ModelViewMatrix.getPtr());
    
}

void updateCamera(){
    CameraPos = Vector3f(TankInfo.PosX - CameraDistance * sinf(toRadian(TankInfo.Rotation)), TankInfo.PosY + CameraPosY, TankInfo.PosZ - CameraDistance * cosf(toRadian(TankInfo.Rotation)));
    CameraFront = Vector3f(TankInfo.PosX + CameraFrontX, TankInfo.PosY + CameraFrontY, TankInfo.PosZ + CameraFrontZ);
}

// TODO: Projectile
void Projectile(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glUniform1i(TextureMapUniformLocation, 0);
    
    glPushMatrix();
    Matrix4x4 ProjectileMatrix;
    ProjectileMatrix.toIdentity();
    ProjectileMatrix.translate(TankInfo.PosX, 2.0, TankInfo.PosZ);
    //    ProjectileMatrix.scale(0.2f, 0.2f, 0.2f);
    ProjectileMatrix = ModelViewMatrix * ProjectileMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, ProjectileMatrix.getPtr());
    ball.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
    glPopMatrix();
}


// 2D Text HUD Rendering
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
    sprintf(ScoreString, "Score: %d/%d", score, TotalCoin2);
    sprintf(LifeString, "Life: %d", life);
    if(life >= 1)
        render2dText(LifeString, 1.0f, 1.0f, 1.0f, 0.5f+1.0/32.0, 0.9f);
    else
        render2dText("Game Over", 1.0f, 1.0f, 1.0f, 0.5f+1.0f/32.0, 0.0f);
    
    if(TotalCoin == score)
        render2dText("Level 2", 1.0f, 1.0f, 1.0f, -0.5f+1.0f/32.0, 0.75f);
    render2dText(TimeString, 1.0f, 1.0f, 1.0f, -1.0f+1.0f/32.0, -1.0f);
    render2dText(ScoreString, 1.0f, 1.0f, 1.0f, 0.0f+1.0f/32.0, -1.0f);
}

// Collision Detection
void CheckLocation(float x, float z, int Maze[MAX_LENGTH][MAX_LENGTH]){
    if(Maze[int((z+OFFSET)/16)][int((x+OFFSET)/16)] == 2){
        Maze[int((z+OFFSET)/16)][int((x+OFFSET)/16)] = 1;
        score++;
    }
    if(Maze[int((z+OFFSET)/16)][int((x+OFFSET)/16)] == 0 || x < -OFFSET || z < -OFFSET){
        OutOfBounds = 1;
    }
}

void Respawn(){
    if(life >= 1){
        TankInfo.PosX = 0.0f;
        TankInfo.PosY = 0.0f;
        TankInfo.PosZ = 0.0f;
        TankInfo.Rotation = 0.0f;
        --life;
        OutOfBounds = 0;
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
    if(keyStates['w'])
        isForward = 1;
    else
        isForward = 0;
    
    if(keyStates['s'])
        isBackward = 1;
    else
        isBackward = 0;
    
    if(keyStates['a'] && (keyStates['w'] || (keyStates['s']))){
        // camera_x += 0.1;
        TankInfo.Rotation += 1.0f;
        //        Vector3f crosscamerafront;
        //        crosscamerafront = camerafront.cross(camerafront, cameraup);
        //        camerapos = camerapos - camerapos.normalise(crosscamerafront.operator*(cameraspeed));
    }
    
    if(keyStates['d'] && (keyStates['w'] || (keyStates['s']))){
        //camera_x -= 0.1;
        TankInfo.Rotation -= 1.0f;
        //        Vector3f crosscamerafront;
        //        crosscamerafront = camerafront.cross(camerafront, cameraup);
        //        camerapos = camerapos + camerapos.normalise(crosscamerafront.operator*(cameraspeed));
    }
    if(keyStates[32]){
        int SpawnTime = 0.0f;
        if(SpawnTime < 5.0f){
            Projectile();
            SpawnTime += deltaTime;
        }
        std::cout << "Spacebar is pressed" <<  std::endl;
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
float dt = 0;

//! Timer Function
void Timer(int value)
{
    updateCamera();
    printf("%f %f %f\n", dt, TankInfo.VelX, TankInfo.VelZ);
    
    if(isForward == 1){
        if (Vector3f(TankInfo.VelX, TankInfo.VelY, TankInfo.VelZ).length() > 0.5) {
            TankInfo.VelX = TankInfo.VelX*sinf(toRadian(TankInfo.Rotation));
            TankInfo.VelZ = TankInfo.VelZ*cosf(toRadian(TankInfo.Rotation));
        }
        else{
            dt += 0.005f;
            TankInfo.VelX = dt*sinf(toRadian(TankInfo.Rotation));
            TankInfo.VelZ = dt*cosf(toRadian(TankInfo.Rotation));
        }
        TankInfo.PosX += TankInfo.VelX;
        TankInfo.PosZ += TankInfo.VelZ;
        WheelRotation += 1.0f;
    }
    else if(isForward == 0 && dt > 0){
        dt -= 0.005f;
        TankInfo.PosX += dt*sinf(toRadian(TankInfo.Rotation));
        TankInfo.PosZ += dt*cosf(toRadian(TankInfo.Rotation));
        WheelRotation += 1.0f;
    }
    
    if(isBackward == 1){
        dt -= 0.005f;
        TankInfo.PosX += dt*sinf(toRadian(TankInfo.Rotation));
        TankInfo.PosZ += dt*cosf(toRadian(TankInfo.Rotation));
        WheelRotation += 1.0f;
    }
    else if(isBackward == 0 && dt < 0){
        dt += 0.005f;
        TankInfo.PosX += dt*sinf(toRadian(TankInfo.Rotation));
        TankInfo.PosZ += dt*cosf(toRadian(TankInfo.Rotation));
        WheelRotation += 1.0f;
    }
    
    if(OutOfBounds == 1){
        TankInfo.PosY -= dt;
        dt += 0.05f;
        if (life >= 1 && dt > 5){
            Respawn();
            dt = 0;
        }
    }
    
    coinRotation += 2.0f;
    glutPostRedisplay();
    //Call function again after 10 milli seconds
    glutTimerFunc(10,Timer, 1);
}

void Update(){
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = (currentTime - startTime)/1000.0f;
    startTime = currentTime;
}
