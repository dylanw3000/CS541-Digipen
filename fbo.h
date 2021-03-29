///////////////////////////////////////////////////////////////////////
// A slight encapsulation of a Frame Buffer Object (i'e' Render
// Target) and its associated texture.  When the FBO is "Bound", the
// output of the graphics pipeline is captured into the texture.  When
// it is "Unbound", the texture is available for use as any normal
// texture.
////////////////////////////////////////////////////////////////////////

class FBO {
public:
    unsigned int fboID;
    unsigned int textureID[4];
    int width, height;  // Size of the texture.
    bool isGBuffer = false;
    unsigned int depthBuffer;

    void CreateFBO(const int w, const int h, bool isGBuffer);
    void Bind();
    void Unbind();
};
