/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

out vec4 FragColor;

// These definitions agree with the ObjectIds enum in scene.h
const int     nullId	= 0;
const int     skyId	= 1;
const int     seaId	= 2;
const int     groundId	= 3;
const int     roomId	= 4;
const int     boxId	= 5;
const int     frameId	= 6;
const int     lPicId	= 7;
const int     rPicId	= 8;
const int     teapotId	= 9;
const int     spheresId	= 10;
const int     floorId	= 11;

// in vec3 normalVec, lightVec, eyeVec, tanVec;
in vec2 texCoord;
// in vec4 shadowCoord;
in vec4 vertex;

uniform int objectId;
uniform vec3 diffuse;    // Kd
uniform vec3 specular;   // Ks
uniform float shininess; // alpha exponent
uniform float time;

uniform vec3 Light;    // Ii
uniform vec3 Ambient;  // Ia
uniform vec3 lightPos, eyePos;

uniform int mode;
uniform mat4 shadowMatrix;
uniform sampler2D shadowMap, upperReflect, lowerReflect;
uniform sampler2D skyTex, groundTex, wallTex, floorTex, teapotTex, frameTex, lFrameTex, rFrameTex;
uniform sampler2D wallNormal, floorNormal, frameNormal, seaNormal;
uniform sampler2D worldPosMap, normalVecMap, KdMap, KsMap;

uniform bool reflective;


void main()
{
    vec2 uv = gl_FragCoord.xy/vec2(1000, 1000);

    vec3 N = normalize(texture(normalVecMap, uv).xyz);
    vec3 worldPos = texture(worldPosMap, uv).xyz;

    vec3 L = normalize(lightPos - worldPos);
    vec3 V = normalize(eyePos - worldPos);
    vec3 H = normalize(L+V);
    
    vec3 Kd = texture(KdMap, uv).xyz;
    vec3 Ks = texture(KsMap, uv).xyz;
    float a = texture(KsMap, uv).w;
    
    
    float LN = max(dot(L,N), 0.0);
    float HN = max(dot(H,N), 0.0);
    float HL = max(dot(H,L), 0.0);

    
    float falloff = 1 - (pow(worldPos.x-lightPos.x, 2) + pow(worldPos.y-lightPos.y, 2) + pow(worldPos.z-lightPos.z, 2)) / 200.0;
    if(falloff <= 0 || mode > 2){
        FragColor.rgb = vec3(0,0,0);
        return;
    }
    
    
    vec3 F = Ks + ((1,1,1) - Ks) * pow(1 - HL, 5);
    float G = 1 / pow(HL, 2);
    float D = ((a + 2)/6.28318) * pow(HN, a);
    FragColor.xyz = (Ambient*Kd + Light * LN * (Kd/3.14159 + ((F*G*D)/4) )) * falloff;
    
    
}
