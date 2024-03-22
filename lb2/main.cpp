#include <windows.h>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <windowsx.h>

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

void renderTexture(GLuint textureID);
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
/*
        std::cout <<
            "BUTTON (" << x << ", " << y << ") :: " <<
            clickX << ' ' << clickY << '\n';
*/

        return (clickX >= x && clickX <= x + width && clickY >= y && clickY <= y + height);
    }

    void render() {
        glColor3f(0.0f, 1.0f, 1.0f); // Set color to red

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
    }
};

Button button1(100.0f, 0.0f, 100.0f, 50.0f, 0); // Button 1
Button button2(210.0f, 0.0f, 100.0f, 50.0f, 0); // Button 2
Button button3(320.0f, 0.0f, 100.0f, 50.0f, 0); // Button 3

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


    // NOTE : CONSTRUCTION ZONE
    // glEnable(GL_DEPTH_TEST); // Enable depth testing
    // glDisable(GL_BLEND);

    // Render the red rectangle outside of the loop
    // renderRectangle();

    while (!bQuit) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                bQuit = TRUE;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Set clear color to dark gray
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear both color and depth buffers
            glDisable(GL_DEPTH_TEST);

//            std::cout << "Clearing screen..." << std::endl;
//            std::cout << "Rendering texture..." << std::endl;

            // Reset color to default (white) before rendering the texture
            // glColor3f(0.0f, 1.0f, 1.0f);
            if (isRenderTexture)
                renderTexture(textureID); // Render the texture

            // Reset color to default (white) before rendering the texture
            // glColor3f(1.0f, 1.0f, 1.0f);
//            renderRectangle();

             button1.render(); // Render button 1
             button2.render(); // Render button 2
             button3.render(); // Render button 3

            SwapBuffers(hDC);

            // Print any OpenGL errors
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
            }
        }
    }


    // =============

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
        {
//            std::cout << "LMB pressed at " + GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) + '\n';
//            std::cout << "Is clicked the button? ";
            if (button1.isClicked(
                    (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)
            )) {
//                std::cout << "YES\n";
                isRenderTexture = !isRenderTexture;
            }/* else {
                std::cout << "NO\n";
            }*/

            break;
        }
        default:
            // std::cout << "Key pressed: " + wParam + '\n';
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
    glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top); // Adjust viewport size

// Make coordinates start at the top left corner
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, rect.right - rect.left, rect.bottom - rect.top, 0, -1, 1); // Set orthographic projection
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
//
    std::cout << "Viewport size: " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << std::endl;
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
        std::cout << "Texture width: " << *width << ", height: " << *height << std::endl;
        std::cout << "Texture format: " << (nrChannels == 3 ? "RGB" : "RGBA") << std::endl;

        // Save texture as an image file
        std::string outputFilename = "test.png";
        stbi_write_png(outputFilename.c_str(), *width, *height, nrChannels, data, 0);


        stbi_image_free(data);
        return textureID;
    } else {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }
}

// Render the texture
/*
void renderTexture(GLuint textureID) {
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textureID); // Bind the texture

    // glColor3f(1.0f, 1.0f, 1.0f);
    // Set the texture environment mode to GL_REPLACE
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Check if texture is bound successfully
    GLint boundTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
    if (boundTexture == textureID) {
//        std::cout << "Texture bound successfully: " << textureID << std::endl;
    } else {
        std::cerr << "Failed to bind texture: " << textureID << std::endl;
    }


    glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);  // Top-left corner
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(imageWidth, 0.0f);  // Top-right corner
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(imageWidth, imageHeight);  // Bottom-right corner
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, imageHeight);  // Bottom-left corner

    glEnd();



    glDisable(GL_TEXTURE_2D);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
    }
}
*/
void renderTexture(GLuint textureID) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID); // Bind the texture

    // glColor3f(1.0f, 1.0f, 1.0f);
    // Set the texture environment mode to GL_REPLACE
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Calculate the aspect ratio of the texture
    float textureAspect = (float)imageWidth / (float)imageHeight;

    // Calculate the aspect ratio of the screen
    RECT rect;
    GetClientRect(hwnd, &rect);
    float screenAspect = (float)(rect.right - rect.left) / (float)(rect.bottom - rect.top);

    // Determine the portion of the screen to cover with the texture
    float texWidth, texHeight;
    if (textureAspect > screenAspect) {
        // Texture is wider than the screen
        texWidth = (rect.right - rect.left);
        texHeight = texWidth / textureAspect;
    } else {
        // Texture is taller than or equal to the screen
        texHeight = (rect.bottom - rect.top);
        texWidth = texHeight * textureAspect;
    }

    float offsetX = ((rect.right - rect.left) - texWidth) / 2.0f;
    float offsetY = ((rect.bottom - rect.top) - texHeight) / 2.0f;

    // Render the texture within the calculated portion of the screen
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(offsetX, offsetY);  // Top-left corner
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(offsetX + texWidth, offsetY);  // Top-right corner
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(offsetX + texWidth, offsetY + texHeight);  // Bottom-right corner
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(offsetX, offsetY + texHeight);  // Bottom-left corner
    glEnd();

    glDisable(GL_TEXTURE_2D);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
    }
}

//


void renderRectangle() {
    glColor3f(1.0f, 0.0f, 0.0f); // Set color to red


    glBegin(GL_QUADS);

        glVertex2f(-0.5f, -0.5f); // Bottom-left corner
        glVertex2f(0.5f, -0.5f); // Bottom-right corner
        glVertex2f(0.5f, 0.5f); // Top-right corner
        glVertex2f(-0.5f, 0.5f); // Top-left corner

    glEnd();
}
