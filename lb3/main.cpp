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

Button button1;
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

    button1 = Button(100.0f, 0.0f, 100.0f, 50.0f,
        loadTexture("grunge-texture-png-03-1536x1024.png", &button1.texWidth, &button1.texHeight)); // Button 1

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

            // if (isRenderTexture)
            //     renderTexture(textureID, 1);

            renderLevel(levelData);
            character.render();

            button1.render();
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

            // Check if the mouse click is within the boundaries of the button
            if (mouseX >= exit_button.x && mouseX <= exit_button.x + exit_button.width &&
                mouseY >= exit_button.y && mouseY <= exit_button.y + exit_button.height) {
                // Perform action when the button is clicked
                exit_button.onClick();
            }

            // Check if the mouse click is within the boundaries of the button
            if (mouseX >= pause_button.x && mouseX <= pause_button.x + pause_button.width &&
                mouseY >= pause_button.y && mouseY <= pause_button.y + pause_button.height) {
                // Perform action when the button is clicked
                pause_button.onClick();
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

    // Set color for walls
    glColor3f(0.5f, 0.5f, 0.5f); // Example color (gray)

    // Iterate over the level data
    for (size_t y = 0; y < levelData.size(); ++y) {
        for (size_t x = 0; x < levelData[y].size(); ++x) {
            if (levelData[y][x] == '#') {
                // Render wall
                glBegin(GL_QUADS);
                glVertex2f(x * LEVEL_TILE_SIZE, y * LEVEL_TILE_SIZE);
                glVertex2f((x + 1) * LEVEL_TILE_SIZE, y * LEVEL_TILE_SIZE);
                glVertex2f((x + 1) * LEVEL_TILE_SIZE, (y + 1) * LEVEL_TILE_SIZE);
                glVertex2f(x * LEVEL_TILE_SIZE, (y + 1) * LEVEL_TILE_SIZE);
                glEnd();
            }
            // Add more conditions to render other elements of the level
            // For example, you can use different characters to represent different elements
        }
    }
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