////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "fbo.h"

enum ObjectIds {
    nullId	= 0,
    skyId	= 1,
    seaId	= 2,
    groundId	= 3,
    roomId	= 4,
    boxId	= 5,
    frameId	= 6,
    lPicId	= 7,
    rPicId	= 8,
    teapotId	= 9,
    spheresId	= 10,
    floorId     = 11
};

class Shader;


class Scene
{
public:
    GLFWwindow* window;

    // @@ Declare interactive viewing variables here. (spin, tilt, ry, front back, ...)
    float spin, tilt, tx, ty, rx, ry, front, back, zoom;

    float speed;
    glm::vec3 eye;
    bool w_down, a_down, s_down, d_down;
    bool transformation_mode;
    double prev_time, now_time, time_since_last_refresh;
    double total_time;

    FBO shadowFBO, upperReflectFBO, lowerReflectFBO, GBufferFBO, compiledShadowFBO;
    GLuint shadowMap;

    // Light parameters
    float lightSpin, lightTilt, lightDist;
    glm::vec3 lightPos;
    // @@ Perhaps declare additional scene lighting values here. (lightVal, lightAmb)
    
    ProceduralGround* ground;


    int mode; // Extra mode indicator hooked up to number keys and sent to shader
    
    // Viewport
    int width, height;

    // Transformations
    glm::mat4 WorldProj, WorldView, WorldInverse;

    // All objects in the scene are children of this single root object.
    Object* objectRoot;
    std::vector<Object*> animated;

    // Shader programs
    ShaderProgram* lightingProgram;
    ShaderProgram* shadowProgram;
    ShaderProgram* reflectionProgram;
    ShaderProgram* GBufferProgram;
    ShaderProgram* localLightProgram;
    // ShaderProgram* choelskyProgram;
    ShaderProgram* choleskyProgram;
    ShaderProgram* choleskyProgramV;
    // @@ Declare additional shaders if necessary


    // Textures
    // std::string texAddress = "skys/Tropical_Beach_8k.jpg";
    Texture *skyTex, *groundTex, *wallTex, *floorTex, *teapotTex, *frameTex, *lFrameTex, *rFrameTex;
    Texture* wallNormal, *floorNormal, *frameNormal, *seaNormal;


    void InitializeScene();
    void BuildTransforms();
    void DrawScene();

};
