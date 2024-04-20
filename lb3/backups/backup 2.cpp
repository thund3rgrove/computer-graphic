//
// Created by alex0 on 05.04.2024.
//
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
void DisableOpenGL(HWND, HDC, HGLRC);
GLuint loadTexture(const char *filename, int *width, int *height);

int imageWidth, imageHeight;
GLuint textureID;

void renderTexture(GLuint textureID, int rowIndex, int elementIndex, float offsetX, float offsetY);
bool isRenderTexture = false;

void renderRectangle();

class Button {
public:
    float x, y, width, height;
    GLuint textureID;

    Button(float _x, float _y, float _width, float _height, GLuint _textureID) : x(_x), y(_y), width(_width),
        height(_height), textureID(_textureID) {
    }

    bool isClicked(float clickX, float clickY) {
        return (clickX >= x && clickX <= x + width && clickY >= y && clickY <= y + height);
    }

    void onClick() {
        std::cout << "Button does not have onClick method set up\n";
    }

    void render() {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(x, y);
            glTexCoord2f(.5f, 0.5f);
            glVertex2f(x + width, y);
            glTexCoord2f(.5f, .5f);
            glVertex2f(x + width, y + height);
            glTexCoord2f(0.0f, .5f);
            glVertex2f(x, y + height);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
};

class RenderTextureButton : Button {
public:
    void onClick() {
    }
};

Button button1(100.0f, 0.0f, 100.0f, 50.0f, 0); // Button 1
Button button2(210.0f, 0.0f, 100.0f, 50.0f, 0); // Button 2
Button button3(320.0f, 0.0f, 100.0f, 50.0f, 0); // Button 3

float characterX = 400.0f; // Initial character position
float characterY = 300.0f;

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

    while (!bQuit) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                bQuit = TRUE;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            if (isRenderTexture)
                renderTexture(textureID, 1, 2, characterX, characterY); // Render character at updated position

            button1.render();
            button2.render();
            button3.render();

            SwapBuffers(hDC);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
            }
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
            } else if (wParam == VK_LEFT) {
                characterX -= 10.0f; // Move character left
            } else if (wParam == VK_RIGHT) {
                characterX += 10.0f; // Move character right
            } else if (wParam == VK_UP) {
                characterY -= 10.0f; // Move character up
            } else if (wParam == VK_DOWN) {
                characterY += 10.0f; // Move character down
            }
            break;

        case WM_LBUTTONDOWN:
        {
            if (button1.isClicked((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam))) {
                isRenderTexture = !isRenderTexture;
            }

            if (button2.isClicked((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam))) {
                std::cout << "Second btn was pressed\n";
            }

            if (button3.isClicked((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam))) {
                std::cout << "Third btn was pressed\n";
            }

            break;
        }

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

void renderTexture(GLuint textureID, int rowIndex, int elementIndex, float offsetX, float offsetY) {
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    float textureAspect = (float)imageWidth / (float)imageHeight;

    RECT rect;
    GetClientRect(hwnd, &rect);
    float screenAspect = (float)(rect.right - rect.left) / (float)(rect.bottom - rect.top);

    float texWidth, texHeight;
    if (textureAspect > screenAspect) {
        texWidth = (rect.right - rect.left);
        texHeight = texWidth / textureAspect;
    } else {
        texHeight = (rect.bottom - rect.top);
        texWidth = texHeight * textureAspect;
    }

    offsetX = offsetX - texWidth / 2.0f; // Adjust character position based on its width and height
    offsetY = offsetY - texHeight / 2.0f;

    // Устанавливаем количество строк и столбцов в спрайтлисте
    int spriteRowCount = 3;
    int spriteColumnCount = 5;

    // Вычисляем размер одной строки и столбца спрайтлиста
    float spriteRowHeight = 1.0f / spriteRowCount;
    float spriteColumnWidth = 1.0f / spriteColumnCount;

    // Вычисляем верхнюю и нижнюю границы выбранной строки спрайтлиста
    float top = 1.0f - spriteRowHeight * rowIndex;
    float bottom = top - spriteRowHeight;

    // Вычисляем левую и правую границы выбранного элемента в строке спрайтлиста
    float left = spriteColumnWidth * elementIndex;
    float right = left + spriteColumnWidth;

    glBegin(GL_QUADS);
        glTexCoord2f(left, bottom);             // Нижняя левая текстурная координата
        glVertex2f(offsetX, offsetY);           // Верхний левый угол экрана
        glTexCoord2f(right, bottom);            // Нижняя правая текстурная координата
        glVertex2f(offsetX + texWidth / spriteColumnCount, offsetY);  // Верхний правый угол экрана
        glTexCoord2f(right, top);               // Верхняя правая текстурная координата
        glVertex2f(offsetX + texWidth / spriteColumnCount, offsetY + texHeight * spriteRowHeight); // Нижний правый угол экрана
        glTexCoord2f(left, top);                // Верхняя левая текстурная координата
        glVertex2f(offsetX, offsetY + texHeight * spriteRowHeight); // Нижний левый угол экрана
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
    }
}
