//!Function Prototypes

#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

// Parameter Definitions
#define MAX_LENGTH 30
#define GRAVITY 1
#define PI 3.14159265
#define ACCELERATION 0.005
#define SCALEFACTOR 8
#define OFFSET 8
#define BLOCKLENGTH 16
#define TIMELIMIT 90
#define PROJ_VEL 50
#ifndef common_h
#define common_h

float lasttime = 0.0f;
float speed = 0.4f, VelocityX = 0.0f, VelocityZ = 0.0f;
float CameraPosX = 0.0f, CameraPosY = 8.0f, CameraPosZ = 20.0f;
float CameraFrontX = 0.0f, CameraFrontY = 0.0f, CameraFrontZ = 0.0f;
Vector3f CameraPos = Vector3f(CameraPosX, CameraPosY, CameraPosZ);
Vector3f CameraFront = Vector3f(CameraFrontX,CameraFrontY,CameraFrontZ);
Vector3f CameraUp = Vector3f(0.0f,1.0f,0.0f);
float CameraDistance = CameraPos.length();
Vector3f lightPosition= Vector3f(20.0,20.0,20.0);   // Light Position
Vector3f ambient    = Vector3f(0.1,0.1,0.1);
Vector3f specular   = Vector3f(1.0,1.0,1.0);
float specularPower = 15.0;

// Function Prototypes
float toRaddian(float angle);
bool initGL(int argc, char** argv);
void display(void);
void keyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void handleKeys();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void Timer(int value);
void initShader();
void glEnable(void);
void ObjectInit(void);
void readMaze(std::string filename);
void MapRenderer(int Maze[MAX_LENGTH][MAX_LENGTH]);
void CubeRenderer(int x, int z);
void CoinRenderer(int x, int z);
void TankRenderer(void);
void TurretRenderer(void);
void Camera(void);
void Projectile(void);
void render2dText(std::string text, float r, float g, float b, float x, float y);
void CheckLocation(float x, float y, float z, int Maze[MAX_LENGTH][MAX_LENGTH]);
void CheckProjectile(float x, float y, float z, int Maze[MAX_LENGTH][MAX_LENGTH]);
void FrontWheelRenderer(void);
void BackWheelRenderer(void);
void Respawn(void);
void TextRenderer(void);
void Update(void);
void UpdateCamera(void);
void SetPosition(void);
void ResetPosition(void);

typedef struct EntityParameter Entity;

struct EntityParameter{
    float PosX = 0.0f;
    float PosY = 0.0f;
    float PosZ = 0.0f;
    float VelX = 0.0f;
    float VelY = 0.0f;
    float VelZ = 0.0f;
    float Rotation = 0.0f;
};

Entity TankInfo;
Entity ProjectileInfo;
Entity TurretInfo;
Entity BackWheelInfo;
Entity FrontWheelInfo;

//! Screen size
int screenWidth   	        = 720;
int screenHeight   	        = 720;

//! Array of key states
bool keyStates[256];

//Global Variables
GLuint shaderProgramID;			                    // Shader Program ID
GLuint vertexPositionAttribute;		                // Vertex Position Attribute Location
int OutOfBounds = 0, isForward = 0, isBackward = 0, isShooting = 0, isHit = 0, GameOver = 0;
GLuint vertexNormalAttribute;
GLfloat lastFrame = 0.0f;
float deltaTime = 0.0f, currentTime = 0.0f, startTime = 0.0f;
int MazeData[MAX_LENGTH][MAX_LENGTH], MazeData2[MAX_LENGTH][MAX_LENGTH];
float elapsedTime = 0.0f;
float displacement = 0.0f;
float coinRotation = 0.0f;
int Map = 1;
float timet = 0.0f, timer = 0;
int score = 0, TotalScore = 0;
int life = 3;
int TotalCoin = 0, TotalCoin2 = 0;
float WheelRotation = 0.0f;
float dt = 0, dt2 = 0;
float TimeAtReset = 0, TimeAtPause = 0;

//Camera

//Material Properties
GLuint LightPositionUniformLocation;                // Light Position Uniform
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;

//Mesh
Mesh cube;
Mesh chassis;
Mesh FrontWheel;
Mesh BackWheel;
Mesh coin;
Mesh turret;
Mesh ball;

//Texture
GLuint vertexTexcoordAttribute;                     // Vertex Texcoord Attribute Location
GLuint TextureMapUniformLocation; // Texture Map Location
GLuint texture[4];

//Viewing
Matrix4x4 ModelViewMatrix;          // ModelView Matrix
GLuint MVMatrixUniformLocation;		// ModelView Matrix Uniform
Matrix4x4 ProjectionMatrix;         // Projection Matrix
GLuint ProjectionUniformLocation;	// Projection Matrix Uniform Location


#endif /* common_h */
