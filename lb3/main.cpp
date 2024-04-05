#include <windows.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <windowsx.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb-master/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb-master/stb_image_write.h"

HWND hwnd;
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

void EnableOpenGL(HWND hwnd, HDC *, HGLRC *);
void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC);
GLuint loadTexture(const char *filename, int *width, int *height);

int imageWidth, imageHeight;
GLuint textureID;

void renderTexture(GLuint textureID, int spriteRowIndex);
bool isRenderTexture = false;

class Character {
public:
    float x, y; // Position of the character
    float velocityX, velocityY; // Velocity of the character
    float acceleration = 500.f; // Acceleration due to key presses
    float damping = 0.98f; // Damping factor for inertia

    // Animation variables
    int spriteRowCount = 3;
    int spriteColumnCount = 5;
    int currentFrameIndex = 0;
    float frameDuration = 0.1f; // Time in seconds for each frame
    float frameTimer = 0.0f; // Timer to track frame duration
    int currentAnimation = 1; // 1: Idle, 2: Walking, 3: Idle rare

    float idleRareTimer = 0.0f; // Timer for rare idle animation
    float idleRareDuration = 15.0f; // Duration in seconds before rare idle animation triggers


    Character(float _x, float _y) : x(_x), y(_y), velocityX(0.0f), velocityY(0.0f) {}

    void update(float deltaTime) {
        /*if (abs(velocityX) > 5 || abs(velocityY) > 5) {
            currentAnimation = 2;
        } else {
            currentAnimation = 1;
        }*/

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
            // else if (idleRareTimer >= idleRareDuration) {
            //     currentAnimation = 1; // Switch to idle animation if timer expired
            // }
        }

        updateAnimation(deltaTime);
        updatePosition(deltaTime);
    }

    // Update character's position based on keyboard input and inertia
    void updatePosition(double elapsedTime) {
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

        // std::cout << x << ' ' << y << '\n';
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
        renderTexture(textureID, spriteRowIndex);
    }
};

Character character(400.0f, 300.0f); // Create character at initial position

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

    if (!RegisterClassEx(&wcex))
        return 0;

    hwnd = CreateWindowEx(0, "GLSample", "OpenGL Sample", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                          NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    textureID = loadTexture("spritelist.png", &imageWidth, &imageHeight);

    if (textureID == 0) {
        std::cerr << "Failed to load texture" << std::endl;
        return 1; // Exit with error code
    }

    // Variables for timing
    DWORD prevTime = timeGetTime();
    const float targetFrameTime = 1.0f / 60.0f; // Target frame time for 60 FPS

    while (!bQuit) {
        // Calculate delta time
        DWORD currentTime = timeGetTime();
        float deltaTime = (currentTime - prevTime) / 1000.0f; // Convert milliseconds to seconds
        prevTime = currentTime;

        // Another way
        /*
        auto currentTime = std::chrono::high_resolution_clock::now();
        double deltaTime = std::chrono::duration<double>(currentTime - previousTime).count(); // Time since last frame
        previousTime = currentTime;
        */
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

            if (isRenderTexture)
                renderTexture(textureID, 1);

            character.render();

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
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

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

        std::cout << "Texture loaded successfully: " << filename << std::endl;
        std::cout << "Texture width: " << *width << ", height: " << *height << std::endl;
        std::cout << "Texture format: " << (nrChannels == 3 ? "RGB" : "RGBA") << std::endl;

        std::string outputFilename = "test.png";
        stbi_write_png(outputFilename.c_str(), *width, *height, nrChannels, data, 0);

        stbi_image_free(data);
        return textureID;
    } else {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }
}

void renderTexture(GLuint textureID, int spriteRowIndex) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Calculate texture aspect ratio and screen aspect ratio
    float textureAspect = (float)imageWidth / (float)imageHeight;
    RECT rect;
    GetClientRect(hwnd, &rect);
    float screenAspect = (float)(rect.right - rect.left) / (float)(rect.bottom - rect.top);

    // Calculate texture width and height to maintain aspect ratio
    float texWidth, texHeight;
    if (textureAspect > screenAspect) {
        texWidth = (rect.right - rect.left);
        texHeight = texWidth / textureAspect;
    } else {
        texHeight = (rect.bottom - rect.top);
        texWidth = texHeight * textureAspect;
    }

    // Calculate sprite row height and column width
    float spriteRowHeight = 1.0f / character.spriteRowCount;
    float spriteColumnWidth = 1.0f / character.spriteColumnCount;

    // Calculate top, bottom, left, and right texture coordinates for the selected sprite
    float top = 1.0f - spriteRowHeight * spriteRowIndex;
    float bottom = top - spriteRowHeight;
    float left = spriteColumnWidth * character.currentFrameIndex;
    float right = left + spriteColumnWidth;

    glBegin(GL_QUADS);
    glTexCoord2f(left, bottom);
    glVertex2f(character.x, character.y);
    glTexCoord2f(right, bottom);
    glVertex2f(character.x + texWidth / character.spriteColumnCount, character.y);
    glTexCoord2f(right, top);
    glVertex2f(character.x + texWidth / character.spriteColumnCount, character.y + texHeight * spriteRowHeight);
    glTexCoord2f(left, top);
    glVertex2f(character.x, character.y + texHeight * spriteRowHeight);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
    }
}
