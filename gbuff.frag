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

in vec3 normalVec, lightVec, eyeVec, tanVec;
in vec2 texCoord;
in vec4 shadowCoord;
in vec3 worldPos;

uniform int objectId;
uniform vec3 diffuse;    // Kd
uniform vec3 specular;   // Ks
uniform float shininess; // alpha exponent
uniform float time;

uniform vec3 Light;    // Ii
uniform vec3 Ambient;  // Ia

uniform int mode;
uniform mat4 shadowMatrix;
uniform sampler2D shadowMap, upperReflect, lowerReflect;
uniform sampler2D skyTex, groundTex, wallTex, floorTex, teapotTex, frameTex, lFrameTex, rFrameTex;
uniform sampler2D wallNormal, floorNormal, frameNormal, seaNormal;

uniform bool reflective;


void main()
{
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);
    vec3 V = normalize(eyeVec);
    vec3 H = normalize(L+V);

    vec3 Kd = diffuse;   
    float a = shininess;
    
    vec2 shadowIndex = shadowCoord.xy/shadowCoord.w;
    
    bool inShadow = false;
    
    if(shadowCoord.w > 0.0 && shadowIndex.x >= 0.0 && shadowIndex.x <= 1.0 && shadowIndex.y >= 0.0 && shadowIndex.y <= 1.0){
        if(shadowCoord.w > texture2D(shadowMap, shadowIndex).w + 0.01){
            inShadow = true;
        }
    }

    
    if(objectId == groundId) {
        Kd = texture(groundTex, texCoord * 100.0).xyz;
    }
    else if(objectId == seaId) {
        vec3 R = -(2*dot(V,N) * N - V);
        vec2 uv = vec2(-atan(R.y/R.x)/(2*3.14159), acos(R.z)/3.14159);
        Kd = texture(skyTex, uv).xyz;

        uv.x -= time * 0.001;
        uv.y += time * 0.001;
        vec3 delta = texture(seaNormal, uv * 10.0).xyz;
        delta = delta*2.0 - vec3(1,1,1);
        vec3 T = normalize(tanVec);
        vec3 B = normalize(cross(T,N));
        N = delta.x*T + delta.y*B + delta.z*N;
    }
    else if(objectId == roomId) {
        vec2 uv = vec2(texCoord.y, -texCoord.x);
        Kd = texture(wallTex, uv * 20.0).xyz * 0.9;
        
        vec3 delta = texture(wallNormal, uv * 20.0).xyz;
        delta = delta*2.0 - vec3(1,1,1);
        vec3 T = normalize(tanVec);
        vec3 B = normalize(cross(T,N));
        N = delta.x*T + delta.y*B + delta.z*N;
    }
    else if(objectId == floorId) {
        Kd = texture(floorTex, texCoord * 4.0).xyz;

        
        vec3 delta = texture(floorNormal, texCoord * 4.0).xyz;
        delta = delta*2.0 - vec3(1,1,1);
        vec3 T = normalize(tanVec);
        vec3 B = normalize(cross(T,N));
        N = delta.x*T + delta.y*B + delta.z*N;
        
    }
    else if(objectId == teapotId) {
        Kd = texture(teapotTex, texCoord * 8.0).xyz;
    }
    else if(objectId == boxId) {
        Kd = texture(frameTex, texCoord * 1.0).xyz;

        vec3 delta = texture(frameNormal, texCoord * 1.0).xyz;
        delta = delta*2.0 - vec3(1,1,1);
        vec3 T = normalize(tanVec);
        vec3 B = normalize(cross(T,N));
        N = delta.x*T + delta.y*B + delta.z*N;
    }
    else if(objectId == frameId) {
        Kd = texture(frameTex, texCoord * 1.0).xyz;
        
        vec3 delta = texture(frameNormal, texCoord * 1.0).xyz;
        delta = delta*2.0 - vec3(1,1,1);
        vec3 T = normalize(tanVec);
        vec3 B = normalize(cross(T,N));
        N = delta.x*T + delta.y*B + delta.z*N;
        
    }
    else if(objectId == lPicId) {
        Kd = texture(lFrameTex, texCoord * 1.0).xyz;
    }
    else if(objectId == rPicId) {
        Kd = texture(rFrameTex, texCoord * 1.0).xyz;
    }
    

    float LN = max(dot(L,N), 0.0);
    float HN = max(dot(H,N), 0.0);
    float HL = max(dot(H,L), 0.0);

    
    if (objectId == skyId) {
        vec2 uv = vec2(-atan(V.y/V.x)/(2*3.14159), acos(V.z)/3.14159);
        Kd.xyz = texture(skyTex, uv).xyz;
        a = -1;
    }
    else if(mode >=7) {        // BRDF lighting
        if(inShadow){
            Kd.xyz = Ambient*Kd;
        }
        else{
            vec3 F = specular + ((1,1,1) - specular) * pow(1 - HL, 5);
            if(reflective){
            
            }
            float G = 1 / pow(HL, 2);
            float D = ((shininess + 2)/6.28318) * pow(HN, shininess);
            Kd.xyz = Ambient*Kd + Light * LN * (Kd/3.14159 + ((F*G*D)/4));
        }
        
    }
    
    
    
    gl_FragData[0].xyz = worldPos.xyz;
    gl_FragData[1].xyz = N.xyz;
    gl_FragData[2].rgb = Kd.rgb;
    gl_FragData[3].rgb = specular.rgb;
    gl_FragData[3].w = a;
    

    // FragColor.xyz = vec3(1.0, 0.0, 0.0);
}
