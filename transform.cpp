////////////////////////////////////////////////////////////////////////
// A small library of 4x4 matrix operations needed for graphics
// transformations.  glm::mat4 is a 4x4 float matrix class with indexing
// and printing methods.  A small list or procedures are supplied to
// create Rotate, Scale, Translate, and Perspective matrices and to
// return the product of any two such.

#include <glm/glm.hpp>

#include "math.h"
#include "transform.h"

float* Pntr(glm::mat4& M)
{
    return &(M[0][0]);
}

//@@ The following procedures should calculate and return 4x4
//transformation matrices instead of the identity.

// Return a rotation matrix around an axis (0:X, 1:Y, 2:Z) by an angle
// measured in degrees.  NOTE: Make sure to convert degrees to radians
// before using sin and cos.  HINT: radians = degrees*PI/180
const float pi = 3.14159f;
glm::mat4 Rotate(const int i, const float theta)
{
    glm::mat4 R;
    
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (x == y) {
                R[y][x] = 1;
            }
            else {
                R[y][x] = 0;
            }
        }
    }

    
    const float ang = theta * pi / 180;
    switch (i) {
        case 0:
            R[1][1] = cos(ang);
            R[1][2] = sin(ang);
            R[2][1] = -sin(ang);
            R[2][2] = cos(ang);
            break;
        case 1:
            R[0][0] = cos(ang);
            R[0][2] = -sin(ang);
            R[2][0] = sin(ang);
            R[2][2] = cos(ang);
            break;
        case 2:
            R[0][0] = cos(ang);
            R[0][1] = sin(ang);
            R[1][0] = -sin(ang);
            R[1][1] = cos(ang);
            break;
        default:
            break;
    }
    

    return R;
}

// Return a scale matrix
glm::mat4 Scale(const float x, const float y, const float z)
{
    glm::mat4 S;
    
    for (int xx = 0; xx < 4; xx++) {
        for (int yy = 0; yy < 4; yy++) {
            S[yy][xx] = 0;
        }
    }
    
    S[0][0] = x;
    S[1][1] = y;
    S[2][2] = z;
    S[3][3] = 1;
    

    return S;
}

// Return a translation matrix
glm::mat4 Translate(const float x, const float y, const float z)
{
    glm::mat4 T;
    
    for (int xx = 0; xx < 4; xx++) {
        for (int yy = 0; yy < 4; yy++) {
            if (xx == yy) {
                T[yy][xx] = 1;
            }
            else {
                T[yy][xx] = 0;
            }
        }
    }
    
    T[3][0] = x;
    T[3][1] = y;
    T[3][2] = z;
    

    return T;
}

// Returns a perspective projection matrix
glm::mat4 Perspective(const float rx, const float ry,
             const float front, const float back)
{
    glm::mat4 P;

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            P[y][x] = 0;
        }
    }

    
    P[0][0] = 1 / rx;
    P[1][1] = 1 / ry;
    P[2][2] = -(back + front) / (back - front);
    P[3][2] = -(2 * front * back) / (back - front);
    P[2][3] = -1;
    

    return P;
}


glm::mat4 MatrixMult(glm::mat4 m1, glm::mat4 m2)
{
    glm::mat4 M;

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            M[x][y] = 0;
            for (int i = 0; i < 4; i++) {
                M[x][y] += m1[i][y] * m2[x][i];
            }
        }
    }

    return M;
}


glm::mat4 LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    glm::vec3 V = glm::normalize(center - eye);
    glm::vec3 A = glm::normalize(glm::cross(V, up));
    glm::vec3 B = glm::cross(A, V);

    glm::mat4 R;

    R[0][0] = A.x;
    R[1][0] = A.y;
    R[2][0] = A.z;
    R[3][0] = glm::dot(-A, eye);
    

    R[0][1] = B.x;
    R[1][1] = B.y;
    R[2][1] = B.z;
    R[3][1] = glm::dot(-B, eye);

    R[0][2] = -V.x;
    R[1][2] = -V.y;
    R[2][2] = -V.z;
    R[3][2] = glm::dot(V, eye);

    R[0][3] = 0;
    R[1][3] = 0;
    R[2][3] = 0;
    R[3][3] = 1;

    // mat4x4((0.503871, 0.863779, -0.000000, -0.000000), (-0.613135, 0.357662, 0.704375, 24.269911), (0.608424, 0.000000, 0.709828, -24.083456), (0.000000, 0.000000, 0.000000, 1.000000))

    return R;
}

