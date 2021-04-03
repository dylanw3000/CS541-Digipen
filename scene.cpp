////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

#define REFL

const bool fullPolyCount = true; // Use false when emulating the graphics pipeline in software
#ifdef REFL
const bool showSpheres = true;  // Use true for shadows and reflections test scenes
#else
const bool showSpheres = false;  // Use true for shadows and reflections test scenes
#endif

#include "math.h"
#include <iostream>
#include <stdlib.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include <glu.h>                // For gluErrorString

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>          // For printing GLM objects with to_string

#include "framework.h"
#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "transform.h"
// #include "scene.h"

const float PI = 3.14159f;
const float rad = PI/180.0f;    // Convert degrees to radians

glm::mat4 Identity;

const float grndSize = 100.0;    // Island radius;  Minimum about 20;  Maximum 1000 or so
const float grndOctaves = 4.0;  // Number of levels of detail to compute
const float grndFreq = 0.03;    // Number of hills per (approx) 50m
const float grndPersistence = 0.03; // Terrain roughness: Slight:0.01  rough:0.05
const float grndLow = -3.0;         // Lowest extent below sea level
const float grndHigh = 5.0;        // Highest extent above sea level

////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// throughout your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as after every OpenGL call.  At the very least, once per
// refresh will tell you if something is going wrong.
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line scene.cpp:%d): %s\n", __LINE__, gluErrorString(err)); exit(-1);} }

// Create an RGB color from human friendly parameters: hue, saturation, value
glm::vec3 HSV2RGB(const float h, const float s, const float v)
{
    if (s == 0.0)
        return glm::vec3(v,v,v);

    int i = (int)(h*6.0) % 6;
    float f = (h*6.0f) - i;
    float p = v*(1.0f - s);
    float q = v*(1.0f - s*f);
    float t = v*(1.0f - s*(1.0f-f));
    if      (i == 0)     return glm::vec3(v,t,p);
    else if (i == 1)  return glm::vec3(q,v,p);
    else if (i == 2)  return glm::vec3(p,v,t);
    else if (i == 3)  return glm::vec3(p,q,v);
    else if (i == 4)  return glm::vec3(t,p,v);
    else   /*i == 5*/ return glm::vec3(v,p,q);
}

////////////////////////////////////////////////////////////////////////
// Constructs a hemisphere of spheres of varying hues
Object* SphereOfSpheres(Shape* SpherePolygons)
{
    Object* ob = new Object(NULL, nullId);
    
    for (float angle=0.0;  angle<360.0;  angle+= 18.0)
        for (float row=0.075;  row<PI/2.0;  row += PI/2.0/6.0) {   
            glm::vec3 hue = HSV2RGB(angle/360.0, 1.0f-2.0f*row/PI, 1.0f);

            Object* sp = new Object(SpherePolygons, spheresId,
                                    hue, glm::vec3(1.0, 1.0, 1.0), 120.0);
            float s = sin(row);
            float c = cos(row);
            ob->add(sp, Rotate(2,angle)*Translate(c,0,s)*Scale(0.075*c,0.075*c,0.075*c));
        }
    return ob;
}

////////////////////////////////////////////////////////////////////////
// Constructs a -1...+1  quad (canvas) framed by four (elongated) boxes
Object* FramedPicture(const glm::mat4& modelTr, const int objectId, 
                      Shape* BoxPolygons, Shape* QuadPolygons)
{
    // This draws the frame as four (elongated) boxes of size +-1.0
    float w = 0.05;             // Width of frame boards.
    
    Object* frame = new Object(NULL, nullId);
    Object* ob;
    
    glm::vec3 woodColor(87.0/255.0,51.0/255.0,35.0/255.0);
    ob = new Object(BoxPolygons, frameId,
                    woodColor, glm::vec3(0.2, 0.2, 0.2), 10.0);
    frame->add(ob, Translate(0.0, 0.0, 1.0+w)*Scale(1.0, w, w));
    frame->add(ob, Translate(0.0, 0.0, -1.0-w)*Scale(1.0, w, w));
    frame->add(ob, Translate(1.0+w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));
    frame->add(ob, Translate(-1.0-w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));

    ob = new Object(QuadPolygons, objectId,
                    woodColor, glm::vec3(0.0, 0.0, 0.0), 10.0);
    frame->add(ob, Rotate(0,90));

    return frame;
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as setting a
// number of other parameters.
void Scene::InitializeScene()
{
    glEnable(GL_DEPTH_TEST);
    CHECKERROR;

    // @@ Initialize interactive viewing variables here. (spin, tilt, ry, front back, ...)
    spin = 0;
    tilt = 30;

    ry = 0.4;
    rx = ry * width / height;

    tx = 0;
    ty = 0;
    zoom = 25;

    front = 0.5;
    back = 5000;

    eye = { 0, -20, 0 };
    speed = 10;
    w_down = false;
    a_down = false;
    s_down = false;
    d_down = false;

    transformation_mode = true;
    mode = 1;
    
    // Set initial light parameters
    lightSpin = 150.0;
    lightTilt = -45.0;
    lightDist = 100.0;
    // @@ Perhaps initialize additional scene lighting values here. (lightVal, lightAmb)

    


    shadowFBO.CreateFBO(4000, 4000, false);
    lowerReflectFBO.CreateFBO(1000, 1000, false);
    upperReflectFBO.CreateFBO(1000, 1000, false);
    GBufferFBO.CreateFBO(1000, 1000, true);
    compiledShadowFBO.CreateFBO(4000, 4000, false);

    CHECKERROR;
    objectRoot = new Object(NULL, nullId);

    
    // Enable OpenGL depth-testing
    glEnable(GL_DEPTH_TEST);

    // Create the lighting shader program from source code files.
    // @@ Initialize additional shaders if necessary
    lightingProgram = new ShaderProgram();
    lightingProgram->AddShader("multilight.vert", GL_VERTEX_SHADER);
    lightingProgram->AddShader("multilight.frag", GL_FRAGMENT_SHADER);

    // lightingProgram->AddShader("final.vert", GL_VERTEX_SHADER);
    // lightingProgram->AddShader("final.frag", GL_FRAGMENT_SHADER);
    // lightingProgram->AddShader("lighting.vert", GL_VERTEX_SHADER);
    // lightingProgram->AddShader("lighting.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(lightingProgram->programId, 0, "vertex");
    glBindAttribLocation(lightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightingProgram->programId, 3, "vertexTangent");
    lightingProgram->LinkProgram();



    shadowProgram = new ShaderProgram();
    shadowProgram->AddShader("shadow.vert", GL_VERTEX_SHADER);
    shadowProgram->AddShader("shadow.frag", GL_FRAGMENT_SHADER);

    shadowProgram->LinkProgram();



    reflectionProgram = new ShaderProgram();
    reflectionProgram->AddShader("reflect.vert", GL_VERTEX_SHADER);
    reflectionProgram->AddShader("reflect.frag", GL_FRAGMENT_SHADER);
    reflectionProgram->AddShader("lighting.vert", GL_VERTEX_SHADER);
    reflectionProgram->AddShader("lighting.frag", GL_FRAGMENT_SHADER);

    reflectionProgram->LinkProgram();



    GBufferProgram = new ShaderProgram();
    GBufferProgram->AddShader("gbuff.vert", GL_VERTEX_SHADER);
    GBufferProgram->AddShader("gbuff.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(GBufferProgram->programId, 0, "vertex");
    glBindAttribLocation(GBufferProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(GBufferProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(GBufferProgram->programId, 3, "vertexTangent");
    GBufferProgram->LinkProgram();



    localLightProgram = new ShaderProgram();
    localLightProgram->AddShader("localLight.vert", GL_VERTEX_SHADER);
    localLightProgram->AddShader("localLight.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(localLightProgram->programId, 0, "vertex");
    glBindAttribLocation(localLightProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(localLightProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(localLightProgram->programId, 3, "vertexTangent");

    localLightProgram->LinkProgram();


    /*
    choelskyProgram = new ShaderProgram();
    choelskyProgram->AddShader("choelsky.vert", GL_VERTEX_SHADER);
    choelskyProgram->AddShader("choelsky.frag", GL_FRAGMENT_SHADER);

    choelskyProgram->LinkProgram();
    */



    choleskyProgram = new ShaderProgram();
    choleskyProgram->AddShader("cholesky.comp", GL_COMPUTE_SHADER);

    choleskyProgram->LinkProgram();
    CHECKERROR;



    choleskyProgramV = new ShaderProgram();
    choleskyProgramV->AddShader("cholesky-v.comp", GL_COMPUTE_SHADER);

    choleskyProgramV->LinkProgram();
    CHECKERROR;


    
    // Create all the Polygon shapes
    Shape* TeapotPolygons =  new Teapot(fullPolyCount?12:2);
    Shape* BoxPolygons = new Box();
    Shape* SpherePolygons = new Sphere(32);
    Shape* RoomPolygons = new Ply("room.ply");
    Shape* FloorPolygons = new Plane(10.0, 10);
    Shape* QuadPolygons = new Quad();
    Shape* SeaPolygons = new Plane(2000.0, 50);
    ground = new ProceduralGround(grndSize, 400,
                                     grndOctaves, grndFreq, grndPersistence,
                                     grndLow, grndHigh);
    Shape* GroundPolygons = ground;

    // Various colors used in the subsequent models
    glm::vec3 woodColor(87.0/255.0, 51.0/255.0, 35.0/255.0);
    glm::vec3 brickColor(134.0/255.0, 60.0/255.0, 56.0/255.0);
    glm::vec3 floorColor(6*16/255.0, 5.5*16/255.0, 3*16/255.0);
    glm::vec3 brassColor(0.5, 0.5, 0.1);
    glm::vec3 grassColor(62.0/255.0, 102.0/255.0, 38.0/255.0);
    glm::vec3 waterColor(0.3, 0.3, 1.0);

    glm::vec3 black(0.0, 0.0, 0.0);
    glm::vec3 brightSpec(0.03, 0.03, 0.03);
    glm::vec3 polishedSpec(0.01, 0.01, 0.01);
 
    // Creates all the models from which the scene is composed.  Each
    // is created with a polygon shape (possibly NULL), a
    // transformation, and the surface lighting parameters Kd, Ks, and
    // alpha.

    // @@ This is where you could read in all the textures and
    // associate them with the various objects being created in the
    // next dozen lines of code.

    // @@ To change an object's surface parameters (Kd, Ks, or alpha),
    // modify the following lines.
    
    Object* central    = new Object(NULL, nullId);
    Object* anim       = new Object(NULL, nullId);
    Object* room       = new Object(RoomPolygons, roomId, brickColor, black, 1);
    Object* floor      = new Object(FloorPolygons, floorId, floorColor, black, 1);
    Object* teapot     = new Object(TeapotPolygons, teapotId, brassColor, brightSpec, 120, true);
    Object* podium     = new Object(BoxPolygons, boxId, glm::vec3(woodColor), polishedSpec, 10); 
    Object* sky        = new Object(SpherePolygons, skyId, black, black, 0);
    Object* ground     = new Object(GroundPolygons, groundId, grassColor, black, 1);
    Object* sea        = new Object(SeaPolygons, seaId, waterColor, brightSpec, 120);
    Object* spheres    = SphereOfSpheres(SpherePolygons);
    Object* leftFrame  = FramedPicture(Identity, lPicId, BoxPolygons, QuadPolygons);
    Object* rightFrame = FramedPicture(Identity, rPicId, BoxPolygons, QuadPolygons); 


    // @@ To change the scene hierarchy, examine the hierarchy created
    // by the following object->add() calls and adjust as you wish.
    // The objects being manipulated and their polygon shapes are
    // created above here.

    // Scene is composed of sky, ground, sea, room and some central models
    if (fullPolyCount) {
        objectRoot->add(sky, Scale(2000.0, 2000.0, 2000.0));
        objectRoot->add(sea); 
        objectRoot->add(ground); }
    objectRoot->add(central);
// #ifndef REFL
    objectRoot->add(room,  Translate(0.0, 0.0, 0.02));
// #endif
    objectRoot->add(floor, Translate(0.0, 0.0, 0.02));

    // Central model has a rudimentary animation (constant rotation on Z)
    animated.push_back(anim);

    // Central contains a teapot on a podium and an external sphere of spheres
    central->add(podium, Translate(0.0, 0,0));
    central->add(anim, Translate(0.0, 0,0));
    anim->add(teapot, Translate(0.1, 0.0, 1.5)*TeapotPolygons->modelTr);
    if (showSpheres && fullPolyCount)
        anim->add(spheres, Translate(0.0, 0.0, 0.0)*Scale(30.0, 30.0, 30.0));
    
    // Room contains two framed pictures
    if (fullPolyCount) {
        room->add(leftFrame, Translate(-1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));
        room->add(rightFrame, Translate( 1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8)); }

    CHECKERROR;

    // Load in sky texture
    skyTex = new Texture("./skys/sky.jpg");
    seaNormal = new Texture("./textures/ripples_normalmap.png");
    groundTex = new Texture("./textures/grass.jpg");
    wallTex = new Texture("./textures/Standard_red_pxr128.png");
    wallNormal = new Texture("./textures/Standard_red_pxr128_normal.png");
    floorTex = new Texture("./textures/6670-diffuse.jpg");
    floorNormal = new Texture("./textures/6670-normal.jpg");
    teapotTex = new Texture("./textures/cracks.png");
    frameTex = new Texture("./textures/Brazilian_rosewood_pxr128.png");
    frameNormal = new Texture("./textures/Brazilian_rosewood_pxr128_normal.png");
    lFrameTex = new Texture("./textures/angry.png");
    rFrameTex = new Texture("./textures/cow.png");

    CHECKERROR;

    total_time = 0.0;
}

void Scene::BuildTransforms()
{
    

    // @@ When you are ready to try interactive viewing, replace the
    // following hard coded values for WorldProj and WorldView with
    // transformation matrices calculated from variables such as spin,
    // tilt, tr, ry, front, and back.
    
    WorldProj = Perspective(ry*width/height, ry, front, back);

    if (transformation_mode) {
        WorldView = MatrixMult(Rotate(0, tilt - 90), Rotate(2, spin));
        WorldView = MatrixMult(Translate(tx, ty, -zoom), WorldView);
    }
    else {
        WorldView = MatrixMult(Rotate(0, tilt - 90), Rotate(2, spin));
        WorldView = MatrixMult(WorldView, Translate(-eye.x, -eye.y, eye.z));
    }
    

    // @@ Print the two matrices (in column-major order) for
    // comparison with the project document.
    //std::cout << "WorldView: " << glm::to_string(WorldView) << std::endl;
    //std::cout << "WorldProj: " << glm::to_string(WorldProj) << std::endl;
}

////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be
// drawn. (Which is often: 30 to 60 times per second are the common
// goals.)
void Scene::DrawScene()
{
    // Set the viewport
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    CHECKERROR;
    // Calculate the light's position from lightSpin, lightTilt, lightDist
    lightPos = glm::vec3(lightDist*cos(lightSpin*rad)*sin(lightTilt*rad),
                         lightDist*sin(lightSpin*rad)*sin(lightTilt*rad), 
                         lightDist*cos(lightTilt*rad));

    // Update position of any continuously animating objects
    double atime = 360.0*glfwGetTime()/36;
    for (std::vector<Object*>::iterator m=animated.begin();  m<animated.end();  m++)
        (*m)->animTr = Rotate(2, atime);

    now_time = glfwGetTime();
    time_since_last_refresh = now_time - prev_time;
    prev_time = now_time;
    float step = speed * (float)time_since_last_refresh;

    if (!transformation_mode) {
        if (w_down)
            eye += step * glm::vec3(sin(spin * 0.01745329251), cos(spin * 0.01745329251), 0.0);
        if (s_down)
            eye -= step * glm::vec3(sin(spin * 0.01745329251), cos(spin * 0.01745329251), 0.0);
        if (a_down)
            eye -= step * glm::vec3(cos(spin * 0.01745329251), -sin(spin * 0.01745329251), 0.0);
        if (d_down)
            eye += step * glm::vec3(cos(spin * 0.01745329251), -sin(spin * 0.01745329251), 0.0);

        eye.z = -1.3 - ground->HeightAt(eye.x, eye.y);
    }

    total_time += time_since_last_refresh;

    BuildTransforms();

    // The lighting algorithm needs the inverse of the WorldView matrix
    WorldInverse = glm::inverse(WorldView);
    

    ////////////////////////////////////////////////////////////////////////////////
    // Anatomy of a pass:
    //   Choose a shader  (create the shader in InitializeScene above)
    //   Choose and FBO/Render-Target (if needed; create the FBO in InitializeScene above)
    //   Set the viewport (to the pixel size of the screen or FBO)
    //   Clear the screen.
    //   Set the uniform variables required by the shader
    //   Draw the geometry
    //   Unset the FBO (if one was used)
    //   Unset the shader
    ////////////////////////////////////////////////////////////////////////////////

    int loc, programId;
    glm::vec3 Light(3, 3, 3), Ambient(0.2, 0.2, 0.2);

    glm::mat4 Vl = LookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    glm::mat4 Pl = Perspective(40/lightDist, 40/lightDist, front, back);
    

    

    // glGenTextures(1, &shadowMap);
    // glBindTexture(GL_TEXTURE_2D, shadowMap);

    {
        shadowProgram->Use();
        shadowFBO.Bind();

        glViewport(0, 0, 4000, 4000);
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        programId = shadowProgram->programId;

        loc = glGetUniformLocation(programId, "Proj");     // perspective
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(Pl));

        loc = glGetUniformLocation(programId, "View");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(Vl));

        loc = glGetUniformLocation(programId, "mode");
        glUniform1i(loc, mode);

        CHECKERROR;

        objectRoot->Draw(shadowProgram, Identity);
        CHECKERROR;

        glDisable(GL_CULL_FACE);
        shadowFBO.Unbind();
        shadowProgram->Unuse();
        CHECKERROR;
    }

    /*
    glm::mat4 ShadowMatrix = MatrixMult(Scale(.5, .5, .5), Translate(.5, .5, .5));
    ShadowMatrix = MatrixMult(ShadowMatrix, MatrixMult(Vl, Pl));
    */
    glm::mat4 ShadowMatrix;
    ShadowMatrix = Translate(.5, .5, .5) * Scale(.5, .5, .5);
    ShadowMatrix = ShadowMatrix * Pl * Vl;




    ////////////////////////////////////////////////////////////////////////////////
    // Cholesky
    ////////////////////////////////////////////////////////////////////////////////

    
    {
        int blurWidth = 10;
        if (mode == 2) {
            blurWidth = 0;
        }

        float s = (float)blurWidth / 2;
        float weights[101];
        float sum = 0;
        for (int i = 0; i < blurWidth * 2 + 1; i++)
        {
            weights[i] = (float)exp(-.5 * pow((i - blurWidth) / s, 2));
            sum += weights[i];
        }
        for (int i = 0; i < blurWidth * 2 + 1; i++)
        {
            weights[i] /= sum;
        }


        choleskyProgram->Use();
        programId = choleskyProgram->programId;


        GLuint blockID;
        glGenBuffers(1, &blockID); // Generates block
        int bindpoint = 0; // Start at zero, increment for other blocks

        loc = glGetUniformBlockIndex(programId, "blurKernel");
        glUniformBlockBinding(programId, loc, bindpoint);

        glBindBuffer(GL_UNIFORM_BUFFER, blockID);
        glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
        glBufferData(GL_UNIFORM_BUFFER, (blurWidth * 2 + 1) * sizeof(float), &weights, GL_STATIC_DRAW);


        loc = glGetUniformLocation(programId, "w");
        glUniform1i(loc, blurWidth);


        loc = glGetUniformLocation(programId, "src"); // Perhaps "src" and "dst"
        CHECKERROR;
        glBindImageTexture(0, shadowFBO.textureID[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        CHECKERROR;
        glUniform1i(loc, 0);

        loc = glGetUniformLocation(programId, "dst"); // Perhaps "src" and "dst"
        glBindImageTexture(1, compiledShadowFBO.textureID[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glUniform1i(loc, 1);
        // Change GL_READ_ONLY to GL_WRITE_ONLY for output image
        // Note: GL_RGBA32F means 4 channels (RGBA) of 32 bit floats.



        glDispatchCompute((4000 / 128) + 1, 4000, 1); // Tiles WxH image with groups sized 128x1
        //glDispatchCompute(fboWidth, fboHeight/128, 1); // Tiles WxH image with groups sized 128x1

        choleskyProgram->Unuse();


        /// <summary>
        /// 
        /// </summary>
        choleskyProgramV->Use();
        programId = choleskyProgramV->programId;


        glGenBuffers(1, &blockID); // Generates block
        bindpoint = 1; // Start at zero, increment for other blocks

        loc = glGetUniformBlockIndex(programId, "blurKernel");
        glUniformBlockBinding(programId, loc, bindpoint);

        glBindBuffer(GL_UNIFORM_BUFFER, blockID);
        glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
        glBufferData(GL_UNIFORM_BUFFER, (blurWidth * 2 + 1) * sizeof(float), &weights, GL_STATIC_DRAW);


        loc = glGetUniformLocation(programId, "w");
        glUniform1i(loc, blurWidth);


        loc = glGetUniformLocation(programId, "src"); // Perhaps "src" and "dst"
        CHECKERROR;
        glBindImageTexture(0, shadowFBO.textureID[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        CHECKERROR;
        glUniform1i(loc, 0);

        loc = glGetUniformLocation(programId, "dst"); // Perhaps "src" and "dst"
        glBindImageTexture(1, compiledShadowFBO.textureID[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glUniform1i(loc, 1);
        // Change GL_READ_ONLY to GL_WRITE_ONLY for output image
        // Note: GL_RGBA32F means 4 channels (RGBA) of 32 bit floats.



        glDispatchCompute((4000 / 128) + 1, 4000, 1); // Tiles WxH image with groups sized 128x1
        //glDispatchCompute(fboWidth, fboHeight/128, 1); // Tiles WxH image with groups sized 128x1

        choleskyProgramV->Unuse();
    }
    

    /*
    {
        choelskyProgram->Use();
        compiledShadowFBO.Bind();

        glViewport(0, 0, 4000, 4000);
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_FRONT);

        float blurW = 15.0;

        float s = blurW / 2;
        float weights[101];
        float result = 0;
        for (int i = 0; i < blurW*2 + 1; i++) {
            weights[i] = expf(-.5 * powf((i - blurW) / s, 2));
            result += weights[i];
        }
        for (int i = 0; i < blurW*2 + 1; i++) {
            weights[i] /= result;
        }


        programId = choelskyProgram->programId;

        loc = glGetUniformLocation(programId, "Proj");     // perspective
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(Pl));

        loc = glGetUniformLocation(programId, "View");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(Vl));

        loc = glGetUniformLocation(programId, "ShadowMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(ShadowMatrix));

        int unit = 4;
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, shadowFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "shadowMap");
        glUniform1i(loc, unit);

        loc = glGetUniformLocation(programId, "mode");
        glUniform1i(loc, mode);

        loc = glGetUniformLocation(programId, "w");
        glUniform1f(loc, 8.0);

        CHECKERROR;

        // GLenum attachments[4] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
        // glDrawBuffers(4, attachments);

        objectRoot->Draw(choelskyProgram, Identity);
        CHECKERROR;

        // glDisable(GL_CULL_FACE);
        compiledShadowFBO.Unbind();
        choelskyProgram->Unuse();
        CHECKERROR;
    }
    */


    ////////////////////////////////////////////////////////////////////////////////
    // CS 562
    ////////////////////////////////////////////////////////////////////////////////
    
    {
        GBufferProgram->Use();
        CHECKERROR;
        GBufferFBO.Bind();
        CHECKERROR;

        programId = GBufferProgram->programId;

        glViewport(0, 0, width, height);
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECKERROR;

        loc = glGetUniformLocation(programId, "WorldProj");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
        CHECKERROR;

        loc = glGetUniformLocation(programId, "WorldView");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));

        loc = glGetUniformLocation(programId, "WorldInverse");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));

        loc = glGetUniformLocation(programId, "lightPos");
        glUniform3fv(loc, 1, &(lightPos[0]));

        loc = glGetUniformLocation(programId, "mode");
        glUniform1i(loc, mode);

        loc = glGetUniformLocation(programId, "time");
        glUniform1f(loc, (float)total_time);
        CHECKERROR;


        // glm::vec3 Light(3, 3, 3), Ambient(0.2, 0.2, 0.2);

        loc = glGetUniformLocation(programId, "Light");
        glUniform3fv(loc, 1, &(Light[0]));

        loc = glGetUniformLocation(programId, "Ambient");
        glUniform3fv(loc, 1, &(Ambient[0]));

        loc = glGetUniformLocation(programId, "ShadowMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(ShadowMatrix));

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, shadowFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "shadowMap");
        glUniform1i(loc, 2);


        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, upperReflectFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "upperReflect");
        glUniform1i(loc, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, lowerReflectFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "lowerReflect");
        glUniform1i(loc, 4);
        CHECKERROR;

        {
            int unit = 5;
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, skyTex->textureId);
            loc = glGetUniformLocation(programId, "skyTex");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, groundTex->textureId);
            loc = glGetUniformLocation(programId, "groundTex");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, wallTex->textureId);
            loc = glGetUniformLocation(programId, "wallTex");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, floorTex->textureId);
            loc = glGetUniformLocation(programId, "floorTex");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, teapotTex->textureId);
            loc = glGetUniformLocation(programId, "teapotTex");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, frameTex->textureId);
            loc = glGetUniformLocation(programId, "frameTex");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, lFrameTex->textureId);
            loc = glGetUniformLocation(programId, "lFrameTex");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, rFrameTex->textureId);
            loc = glGetUniformLocation(programId, "rFrameTex");
            glUniform1i(loc, unit);

            //// Normal Maps
            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, wallNormal->textureId);
            loc = glGetUniformLocation(programId, "wallNormal");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, floorNormal->textureId);
            loc = glGetUniformLocation(programId, "floorNormal");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, frameNormal->textureId);
            loc = glGetUniformLocation(programId, "frameNormal");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, seaNormal->textureId);
            loc = glGetUniformLocation(programId, "seaNormal");
            glUniform1i(loc, unit);

            
        }

        GLenum attachments[4] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
        glDrawBuffers(4, attachments);
        CHECKERROR;

        objectRoot->Draw(GBufferProgram, Identity);
        CHECKERROR;

        GBufferFBO.Unbind();
        GBufferProgram->Unuse();
        CHECKERROR;
    }
    

    

    ////////////////////////////////////////////////////////////////////////////////
    // Lighting pass
    ////////////////////////////////////////////////////////////////////////////////
    
    // Choose the lighting shader
    
    lightingProgram->Use();
    programId = lightingProgram->programId;

    // Set the viewport, and clear the screen
    glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECKERROR;

    /*
    glEnable(GL_DEPTH_TEST);
    */
    /*
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    */
    
    

    // @@ The scene specific parameters (uniform variables) used by
    // the shader are set here.  Object specific parameters are set in
    // the Draw procedure in object.cpp
    
    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));

    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));

    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));

    loc = glGetUniformLocation(programId, "lightPos");
    glUniform3fv(loc, 1, &(lightPos[0]));  

    loc = glGetUniformLocation(programId, "eyePos");
    glUniform3fv(loc, 1, &(eye[0]));

    loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);

    loc = glGetUniformLocation(programId, "time");
    glUniform1f(loc, (float)total_time);

    
    // glm::vec3 Light(3, 3, 3), Ambient(0.2, 0.2, 0.2);

    loc = glGetUniformLocation(programId, "Light");
    glUniform3fv(loc, 1, &(Light[0]));

    loc = glGetUniformLocation(programId, "Ambient");
    glUniform3fv(loc, 1, &(Ambient[0]));

    loc = glGetUniformLocation(programId, "ShadowMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(ShadowMatrix));

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, shadowFBO.textureID[0]);
    loc = glGetUniformLocation(programId, "shadowMap");
    glUniform1i(loc, 2);

    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, upperReflectFBO.textureID[0]);
    loc = glGetUniformLocation(programId, "upperReflect");
    glUniform1i(loc, 3);
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, lowerReflectFBO.textureID[0]);
    loc = glGetUniformLocation(programId, "lowerReflect");
    glUniform1i(loc, 4);

    {
        int unit = 5;

        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, GBufferFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "worldPosMap");
        glUniform1i(loc, unit);

        unit++;

        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, GBufferFBO.textureID[1]);
        loc = glGetUniformLocation(programId, "normalVecMap");
        glUniform1i(loc, unit);

        unit++;

        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, GBufferFBO.textureID[2]);
        loc = glGetUniformLocation(programId, "KdMap");
        glUniform1i(loc, unit);

        unit++;

        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, GBufferFBO.textureID[3]);
        loc = glGetUniformLocation(programId, "KsMap");
        glUniform1i(loc, unit);

        unit++;

        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, compiledShadowFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "choleskyMap");
        glUniform1i(loc, unit);
    }

    CHECKERROR;

    // Draw all objects (This recursively traverses the object hierarchy.)
    objectRoot->Draw(lightingProgram, Identity);
    CHECKERROR; 

    /*
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    /*
    glDisable(GL_DEPTH_TEST);
    */
    
    // Turn off the shader
    lightingProgram->Unuse();
    

    ////////////////////////////////////////////////////////////////////////////////
    // End of Lighting pass
    ////////////////////////////////////////////////////////////////////////////////

    if (mode <= 2) {
        /////// Local lights
        localLightProgram->Use();
        programId = localLightProgram->programId;

        // Set the viewport, and clear the screen
        /*
        glViewport(0, 0, width, height);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECKERROR;
        */

        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_BLEND);



        // @@ The scene specific parameters (uniform variables) used by
        // the shader are set here.  Object specific parameters are set in
        // the Draw procedure in object.cpp

        loc = glGetUniformLocation(programId, "WorldProj");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));

        loc = glGetUniformLocation(programId, "WorldView");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));

        loc = glGetUniformLocation(programId, "WorldInverse");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));

        // loc = glGetUniformLocation(programId, "lightPos");
        // glUniform3fv(loc, 1, &(lightPos[0]));

        loc = glGetUniformLocation(programId, "eyePos");
        glUniform3fv(loc, 1, &(eye[0]));

        loc = glGetUniformLocation(programId, "mode");
        glUniform1i(loc, mode);

        loc = glGetUniformLocation(programId, "time");
        glUniform1f(loc, (float)total_time);


        // glm::vec3 Light(3, 3, 3), Ambient(0.2, 0.2, 0.2);

        loc = glGetUniformLocation(programId, "Ambient");
        glUniform3fv(loc, 1, &(Ambient[0]));

        loc = glGetUniformLocation(programId, "ShadowMatrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(ShadowMatrix));

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, shadowFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "shadowMap");
        glUniform1i(loc, 2);


        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, upperReflectFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "upperReflect");
        glUniform1i(loc, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, lowerReflectFBO.textureID[0]);
        loc = glGetUniformLocation(programId, "lowerReflect");
        glUniform1i(loc, 4);

        {
            int unit = 5;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, GBufferFBO.textureID[0]);
            loc = glGetUniformLocation(programId, "worldPosMap");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, GBufferFBO.textureID[1]);
            loc = glGetUniformLocation(programId, "normalVecMap");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, GBufferFBO.textureID[2]);
            loc = glGetUniformLocation(programId, "KdMap");
            glUniform1i(loc, unit);

            unit++;

            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, GBufferFBO.textureID[3]);
            loc = glGetUniformLocation(programId, "KsMap");
            glUniform1i(loc, unit);
        }

        CHECKERROR;

        /*
        for (float i = 0; i < 3; i++) {
            glm::vec3 localLight(3 * (i == 0), 3 * (i == 1), 3 * (i == 2));
            glm::vec3 localLightPos(sinf(2 * i) * 35, cosf(2 * i) * 35, 4);

            loc = glGetUniformLocation(programId, "Light");
            glUniform3fv(loc, 1, &(localLight[0]));

            loc = glGetUniformLocation(programId, "lightPos");
            glUniform3fv(loc, 1, &(localLightPos[0]));

            objectRoot->Draw(localLightProgram, Identity);
            CHECKERROR;
        }
        */

        /*
        for (float i = 0; i < 24; i++) {
            glm::vec3 localLight(3,3,3);
            glm::vec3 localLightPos(sinf(i*3.14/8) * i*5, cosf(i*3.14/8) * i*5, 1);

            loc = glGetUniformLocation(programId, "Light");
            glUniform3fv(loc, 1, &(localLight[0]));

            loc = glGetUniformLocation(programId, "lightPos");
            glUniform3fv(loc, 1, &(localLightPos[0]));

            objectRoot->Draw(localLightProgram, Identity);
            CHECKERROR;
        }
        */


        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);


        // Turn off the shader
        localLightProgram->Unuse();
    }
    

}



/*
    glActiveTexture(GL_TEXTURE2);           // Activate texture unit 2
    glBindTexture(GL_TEXTURE_2D, texID);    // Load texture into it
    loc = glGetUniformLocation(programId, "shadowMap");
    glUniform1i(loc, 2);                    // Tell shader texture is in unit 2
    */
