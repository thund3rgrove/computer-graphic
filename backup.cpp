#include <windows.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb-master/stb_image.h"

HWND hwnd;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

void EnableOpenGL(HWND hwnd, HDC *, HGLRC *);

void DisableOpenGL(HWND, HDC, HGLRC);

GLuint loadTexture(const char *filename, int *width, int *height);

void renderTexture(GLuint textureID, int width, int height);

void renderRectangle();

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

    int imageWidth = 10, imageHeight = 10;
    GLuint textureID = loadTexture("spritelist.png", &imageWidth, &imageHeight);

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
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            int imageWidth = 800; // Provide width of your image
            int imageHeight = 600; // Provide height of your image
            renderTexture(textureID, imageWidth, imageHeight);
            renderRectangle();

            SwapBuffers(hDC);

            // Print any OpenGL errors
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

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load texture image
    int nrChannels;
    unsigned char *data = stbi_load(filename, width, height, &nrChannels, 0);
    if (data) {
        if (nrChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *width, *height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else if (nrChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }

        std::cout << "Texture loaded successfully: " << filename << std::endl;

        stbi_image_free(data);
        return textureID;
    } else {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }
}


void renderTexture(GLuint textureID, int textureWidth, int textureHeight) {
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
    glEnd();
}




void renderRectangle() {
    glColor3f(1.0f, 0.0f, 0.0f); // Set color to red
    glBegin(GL_QUADS);
    glVertex2f(-0.5f, -0.5f); // Bottom-left corner
    glVertex2f(0.5f, -0.5f); // Bottom-right corner
    glVertex2f(0.5f, 0.5f); // Top-right corner
    glVertex2f(-0.5f, 0.5f); // Top-left corner
    glEnd();
}