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
#include <fstream>

#include "Character.h"
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
std::vector<std::string> loadLevel(const std::string& filename);
void renderLevel(const std::vector<std::string>& levelData);
// GLuint loadFontTexture(const char* fontPath, float fontSize, int textureWidth, int textureHeight);

// void renderPauseMenu();

float LEVEL_TILE_SIZE = 37.5f;

int imageWidth, imageHeight;

void renderTexture(GLuint textureID, int spriteRowIndex);
bool isRenderTexture = false;

// Pause state variable
bool isPaused = false;

// GLuint fontTextureID;
// TODO: move to the class
GLuint woodTextureID;
int woodTextureWidth;
int woodTextureHeight;

Button show_pause_menu;
Button pause_button;
Button exit_button;

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
    wcex.lpszClassName = "SimpleGame";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // loadFont("Roboto-Regular.ttf", 24);

    if (!RegisterClassEx(&wcex))
        return 0;

    hwnd = CreateWindowEx(0, "SimpleGame", "Simple Game", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                          NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    // textureID = loadTexture("spritelist.png", &imageWidth, &imageHeight);
    // character.setTextureID(loadTexture("spritelist.png", &imageWidth, &imageHeight));
    Character character(400.0f, 300.0f, 0.2f,
        loadTexture("spritelist.png", &character.texWidth, &character.texHeight));

    pause_button = Button(210.0f, 0.0f, 100.0f, 50.0f,
        loadTexture("holographic-stream-package-o.png", &pause_button.texWidth, &pause_button.texHeight),
        []() {
            isPaused = !isPaused;
        }); // Button 2

    exit_button = Button(320.0f, 0.0f, 100.0f, 50.0f,
        loadTexture("emergency-exit-sign-right-adobe.jpg", &exit_button.texWidth, &exit_button.texHeight),
        []() {
            PostQuitMessage(0);
        }); // Button 3

    show_pause_menu = Button(100.0f, 0.0f, 100.0f, 50.0f,
        // loadTexture("grunge-texture-png-03-1536x1024.png", &show_pause_menu.texWidth, &show_pause_menu.texHeight),
        loadTexture("Lever (1).png", &show_pause_menu.texWidth, &show_pause_menu.texHeight),
        nullptr, true);

    woodTextureID = loadTexture("istockphoto-466135044-612x612.jpg", &woodTextureWidth, &woodTextureHeight);
    std::vector<std::string> levelData = loadLevel("level.txt");


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
            character.update(deltaTime, isPaused, levelData, LEVEL_TILE_SIZE);

            // Render frame
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            renderLevel(levelData);
            character.render();

            show_pause_menu.render();
            pause_button.render();
            exit_button.render();

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
        {
            int mouseX = LOWORD(lParam);
            int mouseY = HIWORD(lParam);

            if (exit_button.isClicked(mouseX, mouseY)) {
                exit_button.onClick();
            }

            if (pause_button.isClicked(mouseX, mouseY)) {
                pause_button.onClick();
            }

            if (show_pause_menu.isClicked(mouseX, mouseY)) {
                exit_button.switchRenderState();
                pause_button.switchRenderState();
            }
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

// Function to load a level from a file
std::vector<std::string> loadLevel(const std::string& filename) {
    std::vector<std::string> levelData;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return levelData;
    }

    std::string line;
    while (std::getline(file, line)) {
        levelData.push_back(line);
    }

    file.close();

    return levelData;
}

void renderLevel(const std::vector<std::string>& levelData) {
    const float LEVEL_TILE_SIZE = 37.5f; // Adjust this value as needed

    // Enable necessary OpenGL states
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind the wood texture
    glBindTexture(GL_TEXTURE_2D, woodTextureID);

    // Set the texture environment mode to modulate
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Iterate over the level data
    for (size_t y = 0; y < levelData.size(); ++y) {
        for (size_t x = 0; x < levelData[y].size(); ++x) {
            if (levelData[y][x] == '#') {
                // Calculate texture coordinates
                float texCoordX = static_cast<float>(x) / static_cast<float>(woodTextureWidth);
                float texCoordY = static_cast<float>(y) / static_cast<float>(woodTextureHeight);

                // Render wall with texture
                glBegin(GL_QUADS);
                glTexCoord2f(texCoordX, texCoordY);
                glVertex2f(x * LEVEL_TILE_SIZE, y * LEVEL_TILE_SIZE);

                glTexCoord2f(texCoordX + 1.0f / woodTextureWidth, texCoordY);
                glVertex2f((x + 1) * LEVEL_TILE_SIZE, y * LEVEL_TILE_SIZE);

                glTexCoord2f(texCoordX + 1.0f / woodTextureWidth, texCoordY + 1.0f / woodTextureHeight);
                glVertex2f((x + 1) * LEVEL_TILE_SIZE, (y + 1) * LEVEL_TILE_SIZE);

                glTexCoord2f(texCoordX, texCoordY + 1.0f / woodTextureHeight);
                glVertex2f(x * LEVEL_TILE_SIZE, (y + 1) * LEVEL_TILE_SIZE);
                glEnd();
            }
            // Add more conditions to render other elements of the level
            // For example, you can use different characters to represent different elements
        }
    }

    // Disable unnecessary OpenGL states
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}
