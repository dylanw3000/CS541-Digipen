/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

out vec4 FragColor;

in vec4 shadowCoord;

uniform int mode;
uniform float w;
// uniform mat4 ShadowMatrix;
uniform sampler2D shadowMap;



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
    vec2 shadowIndex = shadowCoord.xy/shadowCoord.w;

    
    float s = w / 2;
    float weights[101];
    float result = 0.0;
    
    for (int i = 0; i < w*2 + 1; i++) {
        weights[i] = pow(2.718, -.5 * pow((i - w) / s, 2));
        result += weights[i];
    }
    for (int i = 0; i < w*2 + 1; i++) {
        weights[i] /= result;
    }

    
    vec4 v[128+101];
    
    for(int i=0; i<128; i++){
        v[i] = texture2D(shadowMap, shadowIndex);
        if(i < 2*w){
            v[i+128] = texture2D(shadowMap, shadowIndex);
        }
    }

    /*
    for(int i=0; i<128; i++){
        vec4 sum = vec4(0,0,0,0);
        for(int j=0; j<2*w; j++){
            sum += weights[j] * v[i+j];
        }
        break;
    }
    */
    
    
    
    
    vec4 b = texture2D(shadowMap, shadowIndex);
    float alpha = .0001;
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
    
    gl_FragData[0].x = Gs;
}
