#ifndef BUTTON_H
#define BUTTON_H

#include <functional>
#include <iostream>
#include <GL/gl.h>

#ifndef STB_TRUETYPE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb-master/stb_truetype.h"
#endif
// #include "lib/stb-master/stb_image.h"

class Button {
public:
    float x, y, width, height;
    bool isRendering;
    int texWidth, texHeight;
    GLuint textureID;
    GLuint fontTextureID; // Texture ID for font
    stbtt_bakedchar charData[96]; // ASCII 32..126 is 95 glyphs

    using ClickHandler = std::function<void()>;

    Button() : x(0.0f), y(0.0f), width(0.0f), height(0.0f), textureID(0), texWidth(0), texHeight(0), isRendering(false) {}
    Button(float _x, float _y, float _width, float _height, GLuint _textureID = NULL, ClickHandler _onclick = nullptr, bool _isRendering = false) : x(_x), y(_y), width(_width),
        height(_height), textureID(_textureID), onClickHandler(_onclick), isRendering(_isRendering) {
    }

    bool isClicked(float clickX, float clickY) {
        return isRendering && (clickX >= x && clickX <= x + width && clickY >= y && clickY <= y + height);
    }

    void onClick() {
        if (onClickHandler) {
            onClickHandler();
        } else {
            std::cout << "Button does not have onClick method set up\n";
        }
    }

    void onClick(Button& btn) {

    }

    void renderText(const char* text, float posX, float posY, float scale) {
        auto fontTextureID = generateFontTexture("Roboto-Regular.ttf", 24);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, fontTextureID);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glColor3f(0.0f, 1.0f, 0.0f);

        glBegin(GL_QUADS);
        const float invWidth = 1.0f / width;
        const float invHeight = 1.0f / height;
        float x = posX;
        float y = posY;
        for (const char* p = text; *p; ++p) {
            if (*p >= 32 && *p < 128) {
                stbtt_aligned_quad q;
                stbtt_GetBakedQuad(charData, 512, 512, *p - 32, &x, &y, &q, 1); // 512 is atlas width and height
                glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0 * invWidth, q.y0 * invHeight);
                glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1 * invWidth, q.y0 * invHeight);
                glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1 * invWidth, q.y1 * invHeight);
                glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0 * invWidth, q.y1 * invHeight);
            }
        }
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
        }
    }

    void switchRenderState() {
        isRendering = !isRendering;
    }

    void render() {
        if (!isRendering) return;
        // std::cout << "Button position: (" << x << ", " << y << "), size: (" << width << ", " << height << "), textureID: " << textureID << std::endl;

        // Ensure that the texture is bound
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set up the texture environment mode
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        // Enable texturing and blending
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Render the button quad
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(x, y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(x + width, y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(x + width, y + height);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(x, y + height);
        glEnd();

        // Disable texturing and blending
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
        }
    }


    // TODO: extract
    GLuint generateFontTexture(const char* fontFilePath, float fontSize) {
        // Load font file
        FILE* fontFile = fopen(fontFilePath, "rb");
        if (!fontFile) {
            std::cerr << "Failed to open font file\n";
            return 0;
        }

        fseek(fontFile, 0, SEEK_END);
        size_t fontFileSize = ftell(fontFile);
        fseek(fontFile, 0, SEEK_SET);

        std::vector<unsigned char> fontData(fontFileSize);
        fread(fontData.data(), 1, fontFileSize, fontFile);
        fclose(fontFile);

        // Initialize stb_truetype
        stbtt_fontinfo fontInfo;
        if (!stbtt_InitFont(&fontInfo, fontData.data(), 0)) {
            std::cerr << "Failed to initialize font\n";
            return 0;
        }

        // Set character size
        int bitmapWidth = 512; // Adjust as needed
        int bitmapHeight = 512; // Adjust as needed
        float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);
        std::vector<unsigned char> bitmap(bitmapWidth * bitmapHeight);

        // Bake font into bitmap
        stbtt_BakeFontBitmap(fontData.data(), 0, fontSize, bitmap.data(), bitmapWidth, bitmapHeight, 32, 96, Button::charData);

        // Generate OpenGL texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmapWidth, bitmapHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }

private:
    ClickHandler onClickHandler; // Function pointer or function object for onClick behavior
};


class RenderTextureButton : Button {
public:
    void onClick() {

    }
};

#endif //BUTTON_H
