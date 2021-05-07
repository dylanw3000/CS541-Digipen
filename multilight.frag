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
in vec4 shadowCoord;

uniform int objectId;
uniform vec3 diffuse;    // Kd
uniform vec3 specular;   // Ks
uniform float shininess; // alpha exponent
uniform float time;

uniform vec3 Light;    // Ii
uniform vec3 Ambient;  // Ia
uniform vec3 lightPos;

uniform mat4 WorldInverse;

uniform int mode;
// uniform mat4 ShadowMatrix;
uniform sampler2D shadowMap, upperReflect, lowerReflect, choleskyMap;
uniform sampler2D skyTex, groundTex, wallTex, floorTex, teapotTex, frameTex, lFrameTex, rFrameTex;
uniform sampler2D skyIrr, skyIrr2;
uniform sampler2D wallNormal, floorNormal, frameNormal, seaNormal;
uniform sampler2D worldPosMap, normalVecMap, KdMap, KsMap;

uniform bool reflective;

uniform HammersleyBlock {
    float NN;
    float hammersley[2*100]; 
};


vec3 cholesky(float m11, float m12, float m13, float m22, float m23, float m33, float z1, float z2, float z3){
    float a = sqrt(m11);
    float b = m12/a;
    float c = m13/a;
    float d = sqrt(m22-pow(b,2));
    float e = (m23 - b * c) / d;
    float f = sqrt(m33 - pow(c,2) - pow(e,2));

    float c1H = z1 / a;
    float c2H = (z2 - b*c1H) / d;
    float c3H = (z3 - c*c1H - e*c2H) / f;

    float c3 = c3H / f;
    float c2 = (c2H-e * c3) / d;
    float c1 = (c1H - b*c2 - c*c3) / a;
    return vec3(c1,c2,c3);
}

void main()
{
    vec2 uv = gl_FragCoord.xy/vec2(1000, 1000);

    vec3 N = normalize(texture(normalVecMap, uv).xyz);
    vec3 worldPos = texture(worldPosMap, uv).xyz;

    vec3 eyePos = (WorldInverse*vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 eyeVec = eyePos - worldPos;

    vec3 L = normalize(lightPos - worldPos);
    vec3 V = normalize(eyeVec);
    vec3 H = normalize(L+V);
    
    vec3 Kd = texture(KdMap, uv).xyz;
    vec3 Ks = texture(KsMap, uv).xyz;
    float a = texture(KsMap, uv).w;

    if(a == -1 && (mode <= 2 || mode >= 7)){
        FragColor.xyz = Kd;
        return;
    }

    float LN = max(dot(L,N), 0.0);
    float HN = max(dot(H,N), 0.0);
    float HL = max(dot(H,L), 0.0);

    vec2 Nuv = vec2( 0.5 - atan(N.y,N.x)/(2*3.14159), acos(N.z)/3.14159 );
    // vec3 NuvInverse = vec3( cos(2*3.14159*(.5-Nuv.x))*sin(3.14159*Nuv.y), sin(2*3.14159*(.5-Nuv.x))*sin(3.14159*Nuv.y), cos(3.14159*Nuv.y) );

    
    if (mode <= 2) {        // BRDF lighting
        bool inShadow  = false;
        // vec4 shadowCoord = shadowMatrix * vec4(worldPos.x,worldPos.y,worldPos.z,1);
        vec2 shadowIndex = shadowCoord.xy/shadowCoord.w;

        vec4 b = texture2D(choleskyMap, shadowIndex);
        float alpha = 0.000009;
        vec4 bPrime = (1-alpha)*b + alpha*vec4(0.5, 0.5, 0.5, 0.5);

        float zf = shadowCoord.w / 150.0;
        float z1 = 1.0;
        float z2 = zf;
        float z3 = zf*zf;

        float m11 = 1;
        float m12 = bPrime.x;
        float m13 = bPrime.y;
        float m22 = bPrime.y;
        float m23 = bPrime.z;
        float m33 = bPrime.w;

        vec3 Cs = cholesky(m11,m12,m13,m22,m23,m33, z1,z2,z3);

        // (-b +- sqrt(b*b - 4*a*c)) / (2*a)
        z2 = (-Cs.y - sqrt(Cs.y*Cs.y - 4*Cs.x*Cs.z)) / (2*Cs.z);
        z3 = (-Cs.y + sqrt(Cs.y*Cs.y - 4*Cs.x*Cs.z)) / (2*Cs.z);

        if(z2 > z3){
            float tmp = z2;
            z2 = z3;
            z3 = z2;
        }

        float Gs = 0.0;
        if(zf <= z2){
            Gs = 0.0;
        }
        else if(zf <= z3){
            Gs = (zf*z3 - bPrime.x*(zf+z3) + bPrime.y) / ((z3-z2)*(zf-z2));
        }
        else{
            Gs = 1.0 - (z2*z3 - bPrime.x*(z2+z3) + bPrime.y) / ((zf-z2)*(zf-z3));
        }

        // Gs = texture2D(choleskyMap, shadowIndex).x;

        if(Gs > 0.01){
            inShadow = true;
            if(Gs > 1.0){
                Gs = 1.0;
            }
        }

        /*
        if(mode == 2){
            if(shadowCoord.w > 0.0 && shadowIndex.x >= 0.0 && shadowIndex.x <= 1.0 && shadowIndex.y >= 0.0 && shadowIndex.y <= 1.0){
                if(shadowCoord.w > texture2D(shadowMap, shadowIndex).w + 0.01){
                    inShadow = true;
                }
            }
        }
        */
        
        vec3 F = Ks + ((1,1,1) - Ks) * pow(1 - HL, 5);
        float G = 1 / pow(HL, 2);
        float D = ((a + 2)/6.28318) * pow(HN, a);
        vec3 BRDF = ((F*G*D)/4);

        if(inShadow){
            FragColor.xyz = Ambient*Kd + (1-Gs)*Light*LN*(Kd/3.14159 + BRDF);
            if(reflective){
                FragColor.xyz = Light*texture2D(skyIrr, Nuv).xyz*(Kd/3.14159) + (1-Gs)*Light*LN*(Kd/3.14159 + BRDF);
                if(mode == 2){
                    FragColor.xyz = Light*texture2D(skyIrr2, Nuv).xyz*(Kd/3.14159) + (1-Gs)*Light*LN*(Kd/3.14159 + BRDF);
                }
            }
        }
        else{
            FragColor.xyz = Ambient*Kd + Light * LN * (Kd/3.14159 + BRDF);
            if(reflective){
                if (mode != 2){
                    

                    float pw = 0.0;
                    vec3 R = 2*dot(N,V)*N - V;
                    vec3 A = normalize(vec3(-R.y, R.x, 0.0));
                    vec3 B = normalize(R*A);

                    for(int i=0; i<NN; i++){
                        vec2 aaa = vec2(hammersley[2*i], acos(pow(hammersley[2*i+1], 1/(i+1))) / 3.14159);
                        vec3 L = vec3( cos(2*3.14159*(.5-aaa.x))*sin(3.14159*aaa.y), sin(2*3.14159*(.5-aaa.x))*sin(3.14159*aaa.y), cos(3.14159*aaa.y) );
                        vec3 wk = normalize(L.x*A + L.y*B + L.z*R);

                        float level = .5*log2(1000*1000/NN) - .5*log2(D);

                        pw += 1 / pow(dot(vec3(0,0,0), H), 2);
                    }
                    FragColor.xyz = Light*texture2D(skyIrr, Nuv).xyz*(Kd/3.14159) + Light * LN * (Kd/3.14159 + BRDF);
                }
                else{
                    FragColor.xyz = Light*texture2D(skyIrr2, Nuv).xyz*Kd/3.14159 + Light * LN * (Kd/3.14159 + BRDF);
                }
            }
        }
    }
    /*  
    // project 2
    else if(mode == 3){
        float shade = texture(shadowMap, uv).x;
        FragColor.rgb = vec3(shade,shade,shade);
    }
    else if(mode == 4){
        float Gs = texture(choleskyMap, uv).x;
        FragColor.rgb = vec3(Gs,Gs,Gs);
    }
    */

    // project 1
    else if(mode == 3){
        FragColor = texture(worldPosMap, uv);
    }
    else if(mode == 4){
        FragColor = texture(normalVecMap, uv);
    }
    else if(mode == 5){
        FragColor.rgb = texture(KdMap, uv).rgb;
    }
    else if(mode == 6){
        FragColor = texture(KsMap, uv);
    }
    
    else{
        FragColor.rgb = texture(KdMap, uv).rgb;
    }

}
