//!Includes
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include "Shader.h"
#include "Vector.h"
#include "Matrix.h"
#include "Mesh.h"
#include "Texture.h"
#include <iostream>
#include <math.h>
#include <string>
#include "common.h"
#include "Windows.h"
#include <vector>

// Degree to Radian Conversion
float toRadian(float degree){
    return (degree * PI / 180);
}

// Read Levels
void readMaze(){
    using namespace std;
    ifstream level1, level2;

    level1.open("../MazeData.txt");
    level2.open("../MazeData2.txt");
    if(!level1.is_open() || !level2.is_open())
        cerr << "Error : Maze data not located" << endl;
//
    for(int j = 0; j <= MAX_LENGTH; j++){
        for(int i = 0; i <= MAX_LENGTH; i++){
            MazeData[i][j] = 0;
            MazeData2[i][j] = 0;
        }
    }
    
    for(int j = 0; j < 10; j++){
        for(int i = 0; i < 10; i++){
            level1 >> MazeData[i][j];
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
    texture[0] = Texture::LoadBMP("../models/hamvee.bmp");
    texture[1] = Texture::LoadBMP("../models/Crate.bmp");
    texture[2] = Texture::LoadBMP("../models/coin.bmp");
    texture[3] = Texture::LoadBMP("../models/ball.bmp");
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
    
    //Init GLUT
    glutInit(&argc, argv);
    
    //Set Display Mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    
    //Set Window Size
    glutInitWindowSize(screenWidth, screenHeight);
    
    // Window Position
    glutInitWindowPosition(200, 200);
    
    //Create Window
    glutCreateWindow("OpenGL Tank Game");
    
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
    shaderProgramID = Shader::LoadFromFile("shader.vert","shader.frag");

    vertexPositionAttribute = glGetAttribLocation(shaderProgramID,  "aVertexPosition");
    vertexNormalAttribute = glGetAttribLocation(shaderProgramID,    "aVertexNormal");
    vertexTexcoordAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");
    
    MVMatrixUniformLocation         = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform");
    ProjectionUniformLocation       = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform");
    LightPositionUniformLocation    = glGetUniformLocation(shaderProgramID, "LightPosition_uniform");
    AmbientUniformLocation          = glGetUniformLocation(shaderProgramID, "Ambient_uniform");
    SpecularUniformLocation         = glGetUniformLocation(shaderProgramID, "Specular_uniform");
    SpecularPowerUniformLocation    = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");
    TextureMapUniformLocation       = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
}

//! Display Loop
void display(void)
{
    timer = glutGet(GLUT_ELAPSED_TIME)/1000.0f - TimeAtReset;

    //Set Viewport
    glViewport(0,0, screenWidth, screenHeight);
    
    //Clear the screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glUseProgram(shaderProgramID);
    glActiveTexture(GL_TEXTURE0);
    handleKeys();

    glUniformMatrix4fv(ProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());
    glClearColor(0.2f, 0.0f, 0.65f, 0.0f);

    glUniform3f(LightPositionUniformLocation, lightPosition.x, lightPosition.y, lightPosition.z);
    glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
    glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
    glUniform1f(SpecularPowerUniformLocation, specularPower);
    
    if(Map == 1){
    MapRenderer(MazeData);
    TankRenderer();
    CheckLocation(TankInfo.PosX, TankInfo.PosY, TankInfo.PosZ, MazeData);
    CheckProjectile(ProjectileInfo.PosX, ProjectileInfo.PosY, ProjectileInfo.PosZ, MazeData);
    }
    
    if(Map == 2){
        MapRenderer(MazeData2);
        TankRenderer();
        CheckLocation(TankInfo.PosX, TankInfo.PosY, TankInfo.PosZ, MazeData2);
        CheckProjectile(ProjectileInfo.PosX, ProjectileInfo.PosY, ProjectileInfo.PosZ, MazeData2);
    }
    
    if(isShooting == 1)
        Projectile();
    
    Camera();

    glUseProgram(0);
    
    TextRenderer();

    glFlush();
    glutSwapBuffers();
}

// Map Related Rendering
void MapRenderer(int Maze[MAX_LENGTH][MAX_LENGTH]){
    for(int j = 0; j < MAX_LENGTH; j++){
        for(int i = 0; i < MAX_LENGTH; i++){
            switch (Maze[i][j]){
                case 0: break;
                case 1:
                    CubeRenderer(i, j);
                    break;
                case 2:
                    CubeRenderer(i, j);
                    CoinRenderer(i, j);
                    break;
                case 3:
                    CubeRenderer(i, j);
            }
        }
    }
}

void CubeRenderer(int x, int z){
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glUniform1i(TextureMapUniformLocation, 0);
    Matrix4x4 CubeMVMatrix;
    CubeMVMatrix.toIdentity();
    CubeMVMatrix.scale(SCALEFACTOR, SCALEFACTOR, SCALEFACTOR);
    CubeMVMatrix.translate(2 * x, -1.0f, 2 * z);
    CubeMVMatrix = ModelViewMatrix * CubeMVMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, CubeMVMatrix.getPtr());
    cube.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
    
}

void CoinRenderer(int x, int z){
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glUniform1i(TextureMapUniformLocation, 0);
    Matrix4x4 CoinMVMatrix;
    CoinMVMatrix.toIdentity();
    CoinMVMatrix.scale(SCALEFACTOR, SCALEFACTOR, SCALEFACTOR);
    CoinMVMatrix.translate(2 * x, 1.0f, 2 * z);
    CoinMVMatrix.rotate(coinRotation, 0.0f, 1.0f, 0.0f);
    CoinMVMatrix.scale(-0.4f, -0.4f, -0.4f);
    CoinMVMatrix = ModelViewMatrix * CoinMVMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, CoinMVMatrix.getPtr());
    coin.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

// Humvee Rendering
void TankRenderer(){
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glUniform1i(TextureMapUniformLocation, 0);
    Matrix4x4 TankMVMatrix;
    TankMVMatrix.toIdentity();
    TankMVMatrix.translate(TankInfo.PosX, TankInfo.PosY, TankInfo.PosZ);
    TankMVMatrix.rotate(TankInfo.Rotation, 0.0f, 1.0f, 0.0f);
    TankMVMatrix = ModelViewMatrix * TankMVMatrix;
    TurretRenderer();
    FrontWheelRenderer();
    BackWheelRenderer();
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, TankMVMatrix.getPtr());
    chassis.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}


void BackWheelRenderer(){
    Matrix4x4 BackWheelMatrix;
    BackWheelInfo.PosX = TankInfo.PosX;
    BackWheelInfo.PosY = TankInfo.PosY;
    BackWheelInfo.PosZ = TankInfo.PosZ;
    BackWheelMatrix.toIdentity();
    BackWheelMatrix.translate(BackWheelInfo.PosX, BackWheelInfo.PosY, BackWheelInfo.PosZ);
    BackWheelMatrix.rotate(TankInfo.Rotation, 0.0f, 1.0f, 0.0f);
    BackWheelMatrix.translate(-0.112653f, 1.012463f, -1.266819f);
    BackWheelMatrix.rotate(BackWheelInfo.Rotation, 1.0f, 0.0f, 0.0f);
    BackWheelMatrix.translate(0.112653f, -1.012463f, 1.266819f);
    BackWheelMatrix = ModelViewMatrix * BackWheelMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, BackWheelMatrix.getPtr());
    BackWheel.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
    
}
void FrontWheelRenderer(){
    Matrix4x4 FrontWheelMatrix;
    FrontWheelInfo.PosX = TankInfo.PosX;
    FrontWheelInfo.PosY = TankInfo.PosY;
    FrontWheelInfo.PosZ = TankInfo.PosZ;
    FrontWheelMatrix.toIdentity();
    FrontWheelMatrix.translate(TankInfo.PosX, TankInfo.PosY, TankInfo.PosZ);
    FrontWheelMatrix.rotate(TankInfo.Rotation, 0.0f, 1.0f, 0.0f);
    FrontWheelMatrix.translate(-0.112653f, 0.9631225f, 2.091036f);
    FrontWheelMatrix.rotate(FrontWheelInfo.Rotation, 1.0f, 0.0f, 0.0f);
    FrontWheelMatrix.translate(0.112653f, -0.9631225f, -2.091036f);
    FrontWheelMatrix = ModelViewMatrix * FrontWheelMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, FrontWheelMatrix.getPtr());
    FrontWheel.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

void TurretRenderer(){
    Matrix4x4 TurretMVMatrix, CameraTurret;
    TurretMVMatrix.toIdentity();
    TurretInfo.PosX = TankInfo.PosX;
    TurretInfo.PosY = TankInfo.PosY;
    TurretInfo.PosZ = TankInfo.PosZ;
    TurretMVMatrix.translate(TurretInfo.PosX, TurretInfo.PosY, TurretInfo.PosZ);
    TurretMVMatrix.rotate(TurretInfo.Rotation, 0.0f, 1.0f, 0.0f);
    TurretMVMatrix = ModelViewMatrix * TurretMVMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, TurretMVMatrix.getPtr());
    turret.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}

// Camera related
void Camera(){
    ProjectionMatrix.perspective(90, 1.0, 0.0001, 150.0);
    glUniformMatrix4fv(ProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());
    ModelViewMatrix.lookAt(CameraPos, CameraFront, CameraUp);
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, ModelViewMatrix.getPtr());
    
}

void UpdateCamera(){
    CameraPos = Vector3f(TankInfo.PosX - CameraDistance * sinf(toRadian(TurretInfo.Rotation)), TankInfo.PosY + CameraPosY, TankInfo.PosZ - CameraDistance * cosf(toRadian(TurretInfo.Rotation)));
    CameraFront = Vector3f(TankInfo.PosX + CameraFrontX, TankInfo.PosY + CameraFrontY, TankInfo.PosZ + CameraFrontZ);
}

// Projectile
void Projectile(){
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glUniform1i(TextureMapUniformLocation, 0);
    Matrix4x4 ProjectileMatrix;
    ProjectileMatrix.toIdentity();
    ProjectileInfo.PosX = TankInfo.PosX + PROJ_VEL * dt2 * sinf(toRadian(ProjectileInfo.Rotation));
    ProjectileInfo.PosY = (TankInfo.PosY + 4.0f) - dt2 * GRAVITY * 6;
    ProjectileInfo.PosZ = TankInfo.PosZ + PROJ_VEL * dt2 * cosf(toRadian(ProjectileInfo.Rotation));
    ProjectileMatrix.translate(ProjectileInfo.PosX, ProjectileInfo.PosY, ProjectileInfo.PosZ);
    ProjectileMatrix.scale(0.4f, 0.4f, 0.4f);
    ProjectileMatrix = ModelViewMatrix * ProjectileMatrix;
    glUniformMatrix4fv(MVMatrixUniformLocation, 1, false, ProjectileMatrix.getPtr());
    ball.Draw(vertexPositionAttribute, vertexNormalAttribute, vertexTexcoordAttribute);
}


// 2D Text HUD Rendering
void render2dText(std::string text, float r, float g, float b, float x, float y)
{
    glColor4f(r,g,b,1.0);
    glRasterPos2f(x, y); // window coordinates
    for(unsigned int i = 0; i < text.size(); i++)
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);
}

void TextRenderer(){
    glBindTexture(GL_TEXTURE_2D, 0);
    
    char TimeString[20];
    char ScoreString[20];
    char LifeString[20];
    char LevelString[20];
    char TotalScoreString[20];
    
    sprintf(TotalScoreString, "Total Score: %d", TotalScore);
    sprintf(LevelString, "Level: %d", Map);
    sprintf(TimeString, "Time: %.3fs", (TIMELIMIT-timer));
    sprintf(LifeString, "Life: %d", life);
    if(Map == 1)
        sprintf(ScoreString, "Score: %d/%d", score, TotalCoin);
    if(Map == 2)
        sprintf(ScoreString, "Score: %d/%d", score, TotalCoin2);
    
    render2dText(LevelString, 1.0f, 1.0f, 1.0f, -0.95f, 0.95f);
    if(timer < TIMELIMIT && life >= 1)
        render2dText(TimeString, 1.0f, 1.0f, 1.0f, -0.95f, 0.9f);
    
    render2dText(ScoreString, 1.0f, 1.0f, 1.0f, -0.95f, 0.85f);
    render2dText(TotalScoreString, 1.0f, 1.0f, 1.0f, -0.95f, 0.8f);
    
    if(life >= 1 && timer < TIMELIMIT)
        render2dText(LifeString, 0.0f, 1.0f, 0.0f, -0.95f, 0.75f);
    if(life == 0 || timer >= TIMELIMIT){
        render2dText("Game Over", 1.0f, 0.0f, 0.0f, -0.95f, 0.75f);
        render2dText("Press ESC to exit the game", 1.0f, 1.0f, 1.0f, -0.95f, 0.7f);
    }
}

// Collision Detection
void CheckLocation(float x, float y, float z, int Maze[MAX_LENGTH][MAX_LENGTH]){
    if(Maze[int((x+OFFSET)/BLOCKLENGTH)][int((z+OFFSET)/BLOCKLENGTH)] == 2 && y == 0){
        Maze[int((x+OFFSET)/BLOCKLENGTH)][int((z+OFFSET)/BLOCKLENGTH)] = 1;
        TotalScore++;
        score++;
    }
    if(Maze[int((x+OFFSET)/BLOCKLENGTH)][int((z+OFFSET)/BLOCKLENGTH)] == 0 || x < -OFFSET || z < -OFFSET){
        OutOfBounds = 1;
    }
    if (Maze[int((x+OFFSET)/BLOCKLENGTH)][int((z+OFFSET)/BLOCKLENGTH)] == 3){
        OutOfBounds = 1;
        Maze[int((x+OFFSET)/BLOCKLENGTH)][int((x+OFFSET)/BLOCKLENGTH)] = 0;
    }
}

void CheckProjectile(float x, float y, float z, int Maze[MAX_LENGTH][MAX_LENGTH]){
    if(Maze[int((x+OFFSET)/BLOCKLENGTH)][int((z+OFFSET)/BLOCKLENGTH)] == 2 && y >= 0){
        Maze[int((x+OFFSET)/BLOCKLENGTH)][int((z+OFFSET)/BLOCKLENGTH)] = 1;
        TotalScore++;
        score++;
        isHit = 1;
    }
    if(y < 0 && Maze[int((x+OFFSET)/BLOCKLENGTH)][int((z+OFFSET)/BLOCKLENGTH)] == 1)
        isHit = 1;
    
    if (Maze[int((x+OFFSET)/BLOCKLENGTH)][int((z+OFFSET)/BLOCKLENGTH)] == 3)
        Maze[int((x+OFFSET)/BLOCKLENGTH)][int((x+OFFSET)/BLOCKLENGTH)] = 0;
}

void Respawn(){
    if(OutOfBounds == 1){
        --life;
        OutOfBounds = 0;
        if(life >= 1)
            ResetPosition();
    }
}

void ResetPosition(){
    dt = 0;
    TankInfo.PosX = 0;
    TankInfo.PosY = 0;
    TankInfo.PosZ = 0;
    TankInfo.Rotation = 0.0f;
    TurretInfo.Rotation = 0.0f;
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
    if(!GameOver){
        if(keyStates['w'])
            isForward = 1;
        else
            isForward = 0;
        
        if(keyStates['s'])
            isBackward = 1;
        else
            isBackward = 0;
        
        if(keyStates['a'] && dt != 0){
            TankInfo.Rotation += 1.0f * fabsf(dt)*3;
        }
        
        if(keyStates['d'] && dt != 0){
            TankInfo.Rotation -= 1.0f * fabsf(dt)*3;
        }
        if(keyStates['r'])
            TurretInfo.Rotation += 2.0f;
        
        if(keyStates['t'])
            TurretInfo.Rotation -= 2.0f;
        
        if(keyStates[32]){
            if(!isShooting){
                ProjectileInfo.Rotation = TurretInfo.Rotation;
                isShooting = 1;
            }
            Projectile();
        }
    }
}

//! Mouse Interaction
void mouse(int button, int state, int x, int y)
{
}

//! Motion
void motion(int x, int y)
{
}

//! Timer Function
void Timer(int value)
{
    UpdateCamera();
    if(OutOfBounds == 0){
        SetPosition();
        
//       printf("%f %f %f %f\n", dt, dt2, TankInfo.PosX, TankInfo.PosZ);
        if(isForward == 1){
            dt += 0.005f;
            FrontWheelInfo.Rotation += 5;
            BackWheelInfo.Rotation += 5;
        }
        else if(isForward == 0 && dt > 0){
            dt -= 0.005f;
            FrontWheelInfo.Rotation += 5 * dt;
            BackWheelInfo.Rotation += 5 * dt;
        }
        
        if(isBackward == 1){
            dt -= 0.005f;
            FrontWheelInfo.Rotation -= 5;
            BackWheelInfo.Rotation -= 5;
        }
        else if(isBackward == 0 && dt < 0){
            dt += 0.005f;
            FrontWheelInfo.Rotation += 5 * dt;
            BackWheelInfo.Rotation += 5 * dt;
        }
        
        if(isShooting == 1){
            dt2 += 0.05f;
            if(dt2 > 3 || isHit == 1){
                dt2 = 0;
                isShooting = 0;
                isHit = 0;
            }
        }
    }
    
    else if(OutOfBounds == 1){
        TankInfo.PosY -= dt;
        dt += 0.05f;
        if (life >= 1 && dt > 5){
            Respawn();
            dt = 0;
        }
    }
    
    coinRotation += 2.0f;
    
    if(TotalCoin == score){
        Map++;
        TimeAtReset = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
        score = 0;
        ResetPosition();
    }
    
    if(timer >= TIMELIMIT){
        life = 0;
        OutOfBounds = 1;
    }
    if(life == 0 || timer >= TIMELIMIT)
        GameOver = 1;
    
    glutPostRedisplay();
    glutTimerFunc(10,Timer, 1);
}

void Update(){
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = (currentTime - startTime)/1000.0f;
    startTime = currentTime;
}

void SetPosition(){
    TankInfo.VelX = dt * sinf(toRadian(TankInfo.Rotation));
    TankInfo.VelZ = dt * cosf(toRadian(TankInfo.Rotation));
    TankInfo.PosX += TankInfo.VelX;
    TankInfo.PosZ += TankInfo.VelZ;
}
