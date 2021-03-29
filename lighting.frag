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
uniform sampler2D worldPosMap, normalVecMap, KdMap, KsMap;

uniform bool reflective;


void LightingFrag()
{
    
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);
    vec3 V = normalize(eyeVec);
    vec3 H = normalize(L+V);

    vec3 Kd = diffuse;   

    vec2 shadowIndex = shadowCoord.xy/shadowCoord.w;

    bool inShadow  = false;
    if(shadowCoord.w > 0.0 && shadowIndex.x >= 0.0 && shadowIndex.x <= 1.0 && shadowIndex.y >= 0.0 && shadowIndex.y <= 1.0){
        if(shadowCoord.w > texture2D(shadowMap, shadowIndex).w + 0.01){
            inShadow = true;
        }
    }
    
    /*
    // A checkerboard pattern to break up larte flat expanses.  Remove when using textures.
    if (objectId==groundId || objectId==floorId || objectId==seaId) {
        ivec2 uv = ivec2(floor(100.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd *= 0.9; }
    */

    if(mode < 3){
        
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
        
    }

    float LN = max(dot(L,N), 0.0);
    float HN = max(dot(H,N), 0.0);
    float HL = max(dot(H,L), 0.0);

    
    if (objectId == skyId && mode <= 2) {
        vec2 uv = vec2(-atan(V.y/V.x)/(2*3.14159), acos(V.z)/3.14159);
        FragColor.xyz = texture(skyTex, uv).xyz;
    }
    
    else if (mode == 1 || mode == 2) {        // BRDF lighting
        
        
        if(inShadow){
            FragColor.xyz = Ambient*Kd;
        }
        else{
            vec3 F = specular + ((1,1,1) - specular) * pow(1 - HL, 5);
            if(reflective){
            
            }
            float G = 1 / pow(HL, 2);
            float D = ((shininess + 2)/6.28318) * pow(HN, shininess);
            FragColor.xyz = Ambient*Kd + Light * LN * (Kd/3.14159 + ((F*G*D)/4));
        }

        
        if(reflective){
            vec3 R = 2*dot(V,N) * N - V;
            R = R / length(R);
            float a, b, c;
            a = R.x;
            b = R.y;
            c = R.z;

            vec2 uv;
            if(c > 0){
                uv = vec2(a/(1+c), b/(1+c)) * 0.5 + vec2(0.5, 0.5);
                FragColor.xyz += max(0.1 * texture2D(upperReflect, uv).xyz, 0.0);
                if(mode == 2){
                    FragColor.xyz = texture2D(upperReflect, uv).xyz;
                }
            }
            else{
                uv = vec2(a/(1-c), b/(1-c)) * 0.5 + vec2(0.5, 0.5);
                FragColor.xyz += max(0.1 * texture2D(lowerReflect, uv).xyz, 0.0);
                if(mode == 2){
                    FragColor.xyz = texture2D(lowerReflect, uv).xyz;
                }
            }
            // FragColor.xyz += texture2D(upperReflect, uv).xyz;
        }
        
        

    }
    else if(mode == 3){
        vec2 uv = gl_FragCoord.xy/vec2(1000, 1000);
        FragColor = texture(worldPosMap, uv);
    }
    else if(mode == 4){
        vec2 uv = gl_FragCoord.xy/vec2(1000, 1000);
        FragColor = texture(normalVecMap, uv);
    }
    else if(mode == 5){
        vec2 uv = gl_FragCoord.xy/vec2(1000, 1000);
        FragColor.rgb = texture(KdMap, uv).rgb;
    }
    else if(mode == 6){
        vec2 uv = gl_FragCoord.xy/vec2(1000, 1000);
        FragColor = texture(KsMap, uv);
    }

    /* else if (mode == 2) {   // Phong lighting (I am cheating a bit here by dividing the Light by 3 and multiplying Specular by 16.6, for the benefit of BRDF which is my actual shader)
        FragColor.xyz = Ambient*Kd + Light/3*Kd*LN + Light/3*(specular*16.6)*pow(HN, shininess);
        if(inShadow){
            FragColor.xyz = Ambient*Kd;
        }
    } */
    else { // if (mode == 3) {                  // No lighting
        FragColor.xyz = vec3(0.5,0.5,0.5)*Kd + Kd*max(dot(L,N),0.0);
    }
    
    
}
