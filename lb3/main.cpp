#include <windows.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <windowsx.h>
#include <vector>

#include "Button.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb-master/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb-master/stb_image_write.h"

// #define STB_TRUETYPE_IMPLEMENTATION
// #include "lib/stb-master/stb_truetype.h"

#define STB_EASY_FONT_IMPLEMENTATION
#include <chrono>

#include "lib/stb-master/stb_easy_font.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif


// #include "text_renderer.h"

HWND hwnd;
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

void EnableOpenGL(HWND hwnd, HDC *, HGLRC *);
void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC);
GLuint loadTexture(const char *filename, int *width, int *height);
// GLuint loadFontTexture(const char* fontPath, float fontSize, int textureWidth, int textureHeight);

// void renderPauseMenu();

int imageWidth, imageHeight;

void renderTexture(GLuint textureID, int spriteRowIndex);
bool isRenderTexture = false;

// Pause state variable
bool isPaused = false;

// GLuint fontTextureID;

class Character {
public:
    // Dimensions
    float x, y; // Position of the character
    float scale; // Scale of the character

    // Inertia
    float velocityX, velocityY; // Velocity of the character
    float acceleration = 500.f; // Acceleration due to key presses
    float damping = 0.98f; // Damping factor for inertia

    // Animation variables
    int spriteRowCount = 3;
    int spriteColumnCount = 5;
    int currentFrameIndex = 0;
    // Increase this value to slow down the animation
    float frameDuration = 0.15f; // Time in seconds for each frame
    float frameTimer = 0.0f; // Timer to track frame duration
    int currentAnimation = 1; // 1: Idle, 2: Walking, 3: Idle rare

    float idleRareTimer = 0.0f; // Timer for rare idle animation
    float idleRareDuration = 15.0f; // Duration in seconds before rare idle animation triggers

    // Textures
    int texWidth;
    int texHeight;
    GLuint characterTextureID;

    Character(float _x, float _y, float _scale, GLuint _textureID)
        : x(_x), y(_y), velocityX(0.0f), velocityY(0.0f), scale(_scale), characterTextureID(_textureID) {}

    void setTextureID(GLuint _textureID) {
        characterTextureID = _textureID;
    }

    void update(float deltaTime) {
        if (isPaused) return;

        // Check for rare idle animation
        if (currentAnimation != 3) { // If not already in rare idle animation
            if (abs(velocityX) < 5 && abs(velocityY) < 5) {
                currentAnimation = 1; // Idle animation
                // Increment idle rare timer with random addition
                idleRareTimer += deltaTime + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / deltaTime));

                // Check if it's time to switch to rare idle animation
                if (idleRareTimer >= idleRareDuration) {
                    currentAnimation = 3; // Switch to rare idle animation
                }
            } else {
                currentAnimation = 2; // Walking animation
                idleRareTimer = 0.0f; // Reset idle timer
            }
        } else { // If in rare idle animation
            if (abs(velocityX) >= 5 || abs(velocityY) >= 5) {
                currentAnimation = 2; // Switch to walking animation if moving
                idleRareTimer = 0.0f; // Reset idle timer
            }
        }

        updateAnimation(deltaTime);
        updatePosition(deltaTime);
    }

    void render() {
        // Calculate sprite row based on current animation
        int spriteRowIndex;
        if (currentAnimation == 1) {
            spriteRowIndex = 1; // Walking animation
        } else if (currentAnimation == 2) {
            spriteRowIndex = 2; // Idle animation
        } else {
            spriteRowIndex = 3; // Idle rare animation
        }

        // Render the character sprite
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, characterTextureID);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        // Calculate sprite row height and column width
        float spriteRowHeight = 1.0f / spriteRowCount;
        float spriteColumnWidth = 1.0f / spriteColumnCount;

        // Calculate top, bottom, left, and right texture coordinates for the selected sprite
        float top = 1.0f - spriteRowHeight * spriteRowIndex;
        float bottom = top - spriteRowHeight;

        // Calculate texture coordinates based on direction
        float left, right;
        if (velocityX < 0) { // If moving left, flip horizontally
            left = spriteColumnWidth * (currentFrameIndex + 1); // Reverse order
            right = spriteColumnWidth * currentFrameIndex;
        } else {
            left = spriteColumnWidth * currentFrameIndex;
            right = left + spriteColumnWidth;
        }

        // Apply scale to character dimensions
        float charWidth = texWidth / spriteColumnCount * scale;
        float charHeight = texHeight / spriteRowCount * scale;

        // Apply texture according to character size
        glBegin(GL_QUADS);
        glTexCoord2f(left, bottom);
        glVertex2f(x, y);
        glTexCoord2f(right, bottom);
        glVertex2f(x + charWidth, y);
        glTexCoord2f(right, top);
        glVertex2f(x + charWidth, y + charHeight);
        glTexCoord2f(left, top);
        glVertex2f(x, y + charHeight);
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
        }
    }


protected:
    // Update character's position based on keyboard input and inertia
    void updatePosition(float elapsedTime) {
        if (GetKeyState(VK_LEFT) & 0x8000) {
            velocityX -= acceleration * elapsedTime;
        }
        if (GetKeyState(VK_RIGHT) & 0x8000) {
            velocityX += acceleration * elapsedTime;
        }
        if (GetKeyState(VK_UP) & 0x8000) {
            velocityY -= acceleration * elapsedTime;
        }
        if (GetKeyState(VK_DOWN) & 0x8000) {
            velocityY += acceleration * elapsedTime;
        }

        // Apply damping to simulate inertia
        velocityX *= damping;
        velocityY *= damping;

        // Update character's position based on velocity
        x += velocityX * elapsedTime;
        y += velocityY * elapsedTime;
    }

    void updateAnimation(float deltaTime) {
        // Update frame timer
        frameTimer += deltaTime;

        // Check if it's time to advance to the next frame
        if (frameTimer >= frameDuration) {
            // Reset frame timer
            frameTimer = 0.0f;

            // Advance to the next frame
            currentFrameIndex = (currentFrameIndex + 1) % spriteColumnCount;
        }
    }
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // loadFont("Roboto-Regular.ttf", 24);

    if (!RegisterClassEx(&wcex))
        return 0;

    hwnd = CreateWindowEx(0, "GLSample", "OpenGL Sample", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                          NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    // textureID = loadTexture("spritelist.png", &imageWidth, &imageHeight);
    // character.setTextureID(loadTexture("spritelist.png", &imageWidth, &imageHeight));
    Character character(400.0f, 300.0f, 0.2f, loadTexture("spritelist.png", &character.texWidth, &character.texHeight));
    Button button1(100.0f, 0.0f, 100.0f, 50.0f, loadTexture("grunge-texture-png-03-1536x1024.png", &button1.texWidth, &button1.texHeight)); // Button 1
    Button button2(210.0f, 0.0f, 100.0f, 50.0f, loadTexture("grunge-texture-png-03-1536x1024.png", &button2.texWidth, &button2.texHeight)); // Button 2
    Button button3(320.0f, 0.0f, 100.0f, 50.0f, loadTexture("grunge-texture-png-03-1536x1024.png", &button3.texWidth, &button3.texHeight)); // Button 3

    // fontTextureID = loadFontTexture('Roboto-Regular.ttf', 24, 512, 512);

    // Variables for timing
    // DWORD prevTime = timeGetTime();
    // const float targetFrameTime = 1.0f / 60.0f; // Target frame time for 60 FPS, no need in it for now
    auto startTime = std::chrono::high_resolution_clock::now(); // Get start time

    while (!bQuit) {
        // Calculate delta time
        // DWORD currentTime = timeGetTime();
        // float deltaTime = (currentTime - prevTime) / 1000.0f; // Convert milliseconds to seconds
        // prevTime = currentTime;

        // Another way
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - startTime).count(); // Calculate delta time
        startTime = currentTime; // Update start time
        //


        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                bQuit = TRUE;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            // Update game logic
            character.update(deltaTime);

            // Render frame
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            // if (isRenderTexture)
            //     renderTexture(textureID, 1);

            character.render();
            button1.render();
            button2.render();
            button3.render();

            if (GetKeyState('P') & 0x8000) {
                isPaused = !isPaused; // Toggle pause state
            }

            // Render pause menu if game is paused
            if (isPaused) {
                // TODO: Make a pause menu, at least with the buttons
            }



            // Always at the end
            SwapBuffers(hDC);
        }
    }

    DisableOpenGL(hwnd, hDC, hRC);
    DestroyWindow(hwnd);

    return msg.wParam;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }

            // if (wParam == VK_F9) {
            //     isPaused = !isPaused;
            // }

            break;

        case WM_LBUTTONDOWN:
            if (HIWORD(lParam) < 50) {
                isRenderTexture = !isRenderTexture;
            }
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


void EnableOpenGL(HWND hwnd, HDC *hDC, HGLRC *hRC) {
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    *hDC = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);
    SetPixelFormat(*hDC, iFormat, &pfd);

    *hRC = wglCreateContext(*hDC);
    wglMakeCurrent(*hDC, *hRC);

    RECT rect;
    GetClientRect(hwnd, &rect);
    glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, rect.right - rect.left, rect.bottom - rect.top, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}


GLuint loadTexture(const char *filename, int *width, int *height) {
    GLuint _textureID;
    glGenTextures(1, &_textureID);
    glBindTexture(GL_TEXTURE_2D, _textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int nrChannels;
    unsigned char *data = stbi_load(filename, width, height, &nrChannels, 0);
    if (data) {
        if (nrChannels == 3) {
            // If the image loaded has only RGB channels, create an alpha channel and remove the white background
            unsigned char *newData = new unsigned char[(*width) * (*height) * 4];
            for (int i = 0; i < (*width) * (*height); ++i) {
                newData[i * 4] = data[i * 3];
                newData[i * 4 + 1] = data[i * 3 + 1];
                newData[i * 4 + 2] = data[i * 3 + 2];
                newData[i * 4 + 3] = (data[i * 3] == 255 && data[i * 3 + 1] == 255 && data[i * 3 + 2] == 255) ? 0 : 255;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, newData);
            delete[] newData;
        } else if (nrChannels == 4) {
            // If the image already has an alpha channel, use it as is
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }

        if (_textureID == 0) {
            std::cerr << "Failed to load texture (textureID is 0): " << filename << std::endl;
            return 1; // Exit with error code
        }

        std::cout << "Texture loaded successfully: " << filename << std::endl;
        std::cout << "Texture ID: " << _textureID << std::endl;
        std::cout << "Texture width: " << *width << ", height: " << *height << std::endl;
        std::cout << "Texture format: " << (nrChannels == 3 ? "RGB" : "RGBA") << std::endl;

        stbi_image_free(data);
        return _textureID;
    }

    // If no data on load
    std::cerr << "Failed to load texture: " << filename << std::endl;
    return 0;
}


/*void renderText(const char* text, float x, float y, float scale) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600); // Adjust if your window size is different
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fontTextureID); // Replace with your font texture ID

    glBegin(GL_QUADS);
    float offset = 0.0f;
    for (const char* c = text; *c != '\0'; ++c) {
        float cx = (*c % 16) / 16.0f;
        float cy = 1.0f - ((*c / 16) / 16.0f);

        glTexCoord2f(cx, cy);
        glVertex2f(offset, 0.0f);

        glTexCoord2f(cx + 1.0f / 16.0f, cy);
        glVertex2f(offset + 1.0f, 0.0f);

        glTexCoord2f(cx + 1.0f / 16.0f, cy - 1.0f / 16.0f);
        glVertex2f(offset + 1.0f, 1.0f);

        glTexCoord2f(cx, cy - 1.0f / 16.0f);
        glVertex2f(offset, 1.0f);

        offset += 1.0f;
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void renderPauseMenu() {
    renderText("PAUSED", 300.0f, 300.0f, 1.0f); // Render the pause text at desired position
}

GLuint loadFontTexture(const char* fontPath, float fontSize, int textureWidth, int textureHeight) {
    unsigned char* ttf_buffer = nullptr;
    FILE* file = fopen(fontPath, "rb");
    if (!file) {
        std::cerr << "Failed to open font file: " << fontPath << std::endl;
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    ttf_buffer = (unsigned char*)malloc(size);
    fread(ttf_buffer, 1, size, file);
    fclose(file);

    unsigned char* temp_bitmap = (unsigned char*)malloc(textureWidth * textureHeight);
    if (!temp_bitmap) {
        std::cerr << "Failed to allocate memory for font texture" << std::endl;
        free(ttf_buffer);
        return 0;
    }

    stbtt_bakedchar* cdata = (stbtt_bakedchar*)malloc(sizeof(stbtt_bakedchar) * 128);

    // Generate font texture
    stbtt_BakeFontBitmap(ttf_buffer, 0, fontSize, temp_bitmap, textureWidth, textureHeight, 32, 128, cdata);

    // Upload font texture to OpenGL
    GLuint fontTextureID;
    glGenTextures(1, &fontTextureID);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, textureWidth, textureHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Cleanup
    free(ttf_buffer);
    free(temp_bitmap);
    free(cdata);

    return fontTextureID;
}*/