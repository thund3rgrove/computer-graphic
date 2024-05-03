#include <cmath>
#include <iostream>
#include <cstdio>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "camera.h"

// Constaints
bool isPlaying = true;
bool isCameraLightOn = true;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

void EnableOpenGL(HWND hwnd, HDC *, HGLRC *);

void DisableOpenGL(HWND, HDC, HGLRC);

void WndResize(int x, int y);

void Init_Light();

void Init_Material();

void MoveCamera();

void DrawSpinningTriangle(float theta);

void DrawFloor(float width, float length, float tileSize);

void DrawAxes(int windowWidth, int windowHeight);

void DrawChessBoard(float startX, float startY, int n = 8);

void Draw_LightPyramid();

void Draw_Cube();

void Draw_Bulb(GLfloat bulb_color[] = nullptr, GLfloat bulb_width = 2.0f, GLfloat bulb_height = 2.0f,
               GLfloat bulb_position[] = nullptr);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
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
    wcex.lpszClassName = "ComputerGraphic";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "ComputerGraphic",
                          "Lab 5",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          1600,
                          900,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);


    ShowWindow(hwnd, nCmdShow);
    ShowCursor(FALSE);

    RECT windowRect;
    GetClientRect(hwnd, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    WndResize(windowRect.right, windowRect.bottom);


    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    Init_Light();
    Init_Material();

    /* program main loop */
    while (!bQuit) {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT) {
                bQuit = TRUE;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            // if (isPlaying) {
            // if (isPlaying) ShowCursor(FALSE);
            // else ShowCursor(TRUE);
            // Enable OpenGL states and clear buffers
            glClearColor((float) 255.f / 255.f, (float) 254.f / 255.f, (float) 200.f / 255.f, 1.0f);
            // Set background to nice slight yellow :) (255, 254, 200 rgb)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (isPlaying && GetForegroundWindow() == hwnd) {
                MoveCamera();
            }

            // Set up projection matrix
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(45.0f, (float) windowWidth / (float) windowHeight, 0.1f, 100.0f);

            // Set up modelview matrix
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            // Move the camera
            Camera_Apply();

            // Draw the floor
            glPushMatrix();
            DrawFloor(200, 200, 2);
            glPopMatrix();

            // Draw the triangle
            glPushMatrix();
            DrawSpinningTriangle(theta);
            glPopMatrix();

            // Draw the axes
            glPushMatrix();
            DrawAxes(windowWidth, windowHeight);
            glPopMatrix();

            // Render chess board
            // glPushMatrix();
            // DrawChessBoard(5.0f, 3.0f, 8); // Draw the chessboard starting from coordinates (5, 3)
            // glPopMatrix();

            glPushMatrix();
            glTranslatef(2.f, 2.f, 2.f);
            Draw_Cube();
            glPopMatrix();


            // Bulb

            // Outer matrix
            glPushMatrix();

            // Radius of rotation around the cube
            float radius = 12.0f;

            // // TODO: make theta instead of 0
            // Calculate the position of the bulb relative to the cube's center
            float bulb_x = radius * cos(theta * M_PI / 180); // Convert theta to radians
            float bulb_y = radius * sin(theta * M_PI / 180); // Convert theta to radians

            glTranslatef(2.0f + bulb_x, 2.0f + bulb_y, 8.0f);

            // Inner matrix
            glPushMatrix();

            // Angle between the x-axis and the line connecting the bulb to the center of the cube
            float angle = atan2(bulb_y, bulb_x) * 180 / M_PI + 90;

            // Rotate the bulb around its own axis
            glRotatef(angle, 0.0f, 0.0f, 1.0f);

            // Lean the bulb by 45 degrees around the X-axis
            glRotatef(45, 1.0f, 0.0f, 0.0f);

            // Color
            // GLfloat bulb_color[] = {1.f, 0.5f, 0.f}; // Orange color

            // Draw the bulb
            Draw_Bulb(new GLfloat[]{1.f, .5f, .5f, .5f}, (GLfloat)3.f, (GLfloat)3.f);

            // End Inner matrix
            glPopMatrix();

            // End Outer matrix
            glPopMatrix();

            //


            // Check for OpenGL errors
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                fprintf(stderr, "OpenGL error: %d\n", error);
            }

            // Swap buffers and update rotation
            SwapBuffers(hDC);
            theta += 1.0f;
            Sleep(1);
        }
    }


    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
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
            switch (wParam) {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                // isPlaying = !isPlaying;
                    break;
                case 'T': {
                    isCameraLightOn = !isCameraLightOn;
                    if (isCameraLightOn) {
                        glEnable(GL_LIGHT0);
                    } else {
                        glDisable(GL_LIGHT0);
                    }
                    break;
                }
            }
            break;

        case WM_SIZE:
            WndResize(LOWORD(lParam), HIWORD(lParam));
            break;


        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC *hDC, HGLRC *hRC) {
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

void MoveCamera() {
    Camera_MoveDirectional(
        GetKeyState('W') < 0
            ? 1
            : GetKeyState('S') < 0
                ? -1
                : 0,
        GetKeyState('D') < 0
                ? 1
                : GetKeyState('A') < 0
                    ? -1
                    : 0,
        0.1);
    Camera_MoveUpDown(
        GetKeyState(VK_SPACE) < 0
            ? 1
            : GetKeyState(VK_SHIFT) < 0
                ? -1
                : 0,
        0.1
        );
    Camera_AutoMoveByMouse(400, 400);
}

void WndResize(int x, int y) {
    glViewport(0, 0, x, y); // Resize the viewport to match the new window size

    glMatrixMode(GL_PROJECTION); // Switch to the projection matrix
    glLoadIdentity(); // Reset the projection matrix

    float aspectRatio = (float) x / (float) y; // Calculate the new aspect ratio

    // Set up a perspective projection matrix
    gluPerspective(45.0f, aspectRatio, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW); // Switch back to the modelview matrix
}

void DrawSpinningTriangle(float theta) {
    // Apply rotation to the entire scene
    glRotatef(theta, 0.0f, 0.0f, 1.0f);

    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.87f, -0.5f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.87f, -0.5f, 0.0f);
    glEnd();
}

void DrawAxes(int windowWidth, int windowHeight) {
    // Define axis length and padding
    float axisLength = 0.1f * std::min(windowWidth, windowHeight); // Length of each axis line
    float padding = 50.0f; // Padding from the edges of the window

    // Get camera rotation angles
    float camXRot = camera.Xrot;
    float camZRot = camera.Zrot;

    // Calculate the rotation lines' end points
    float xAxisEndX = padding + axisLength * cos(camZRot * M_PI / 180.0f);
    float xAxisEndY = padding + axisLength * sin(camZRot * M_PI / 180.0f);
    float yAxisEndX = padding - axisLength * sin(camZRot * M_PI / 180.0f);
    float yAxisEndY = padding + axisLength * cos(camZRot * M_PI / 180.0f);
    float zAxisEndX = padding + axisLength * cos(camXRot * M_PI / 180.0f);
    float zAxisEndY = padding + axisLength * sin(camXRot * M_PI / 180.0f);

    // Clear the depth buffer to ensure the axes are drawn in front of everything else
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);

    // Set up modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Draw X-axis (red)
    glColor3f(1.0f, 0.0f, 0.0f); // Red color
    glBegin(GL_LINES);
    glVertex2f(padding, padding);
    glVertex2f(xAxisEndX, xAxisEndY);
    glEnd();

    // Draw Y-axis (green)
    glColor3f(0.0f, 1.0f, 0.0f); // Green color
    glBegin(GL_LINES);
    glVertex2f(padding, padding);
    glVertex2f(yAxisEndX, yAxisEndY);
    glEnd();

    // Draw Z-axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color
    glBegin(GL_LINES);
    glVertex2f(padding, padding);
    glVertex2f(zAxisEndX, zAxisEndY);
    glEnd();

    // Restore the projection matrix
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);

    // Restore the modelview matrix
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void DrawChessBoard(float startX, float startY, int n) {
    float tileSize = 1.0f; // Size of each tile in the chessboard

    float normal_vert[] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, &normal_vert);


    // Rows
    for (int i = 0; i < n; ++i) {
        // Columns
        for (int j = 0; j < n; ++j) {
            if ((i + j) % 2 == 0) {
                // Light color for even indices
                glColor3f(0.8f, 0.8f, 0.8f); // Light gray
            } else {
                // Dark color for odd indices
                glColor3f(0.2f, 0.2f, 0.2f); // Dark gray
            }

            // Calculate the position of the current tile
            float x = startX + j * tileSize;
            float y = startY + i * tileSize;

            // Print the coordinates of the current tile
            // std::cout << "Tile at (" << x << ", 0, " << y << ")" << std::endl;

            // Draw the current tile as a quadrilateral
            glBegin(GL_QUADS);
            glVertex3f(x, 0.0f, y);
            glVertex3f(x + tileSize, 0.0f, y);
            glVertex3f(x + tileSize, 0.0f, y + tileSize);
            glVertex3f(x, 0.0f, y + tileSize);
            glEnd();
        }
    }

    glDisable(GL_NORMAL_ARRAY);
}

void DrawFloor(float width, float length, float tileSize) {
    // Define the vertices of the floor
    float minX = -width / 2.0f; // Calculate minimum x-coordinate
    float minY = -length / 2.0f; // Calculate minimum y-coordinate
    float maxX = width / 2.0f; // Calculate maximum x-coordinate
    float maxY = length / 2.0f; // Calculate maximum y-coordinate

    // TODO: why do i need that?..
    float normal_vert[] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, &normal_vert);

    // Draw the floor as a quadrilateral lying on the x-y plane
    glBegin(GL_QUADS);
    glColor3d(204 / 255.f, 204 / 255.f, 204 / 255.f); // Light gray color

    // Draw the chessboard pattern directly on the floor
    bool isLight = true;

    for (float x = minX; x < maxX; x += tileSize) {
        isLight = !isLight; // Toggle between light and dark rows
        for (float y = minY; y < maxY; y += tileSize) {
            if (isLight) {
                // Light color for even tiles
                glColor3f(0.8f, 0.8f, 0.8f); // Light gray
            } else {
                // Dark color for odd tiles
                glColor3f(0.2f, 0.2f, 0.2f); // Dark gray
            }

            // Draw the current tile as a quadrilateral
            glVertex3f(x, y, 0.0f);
            glVertex3f(x + tileSize, y, 0.0f);
            glVertex3f(x + tileSize, y + tileSize, 0.0f);
            glVertex3f(x, y + tileSize, 0.0f);

            isLight = !isLight; // Toggle between light and dark tiles for next column
        }
    }

    glEnd();

    glDisable(GL_NORMAL_ARRAY);
}

// 6

void Init_Light() {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_position[] = {0.0f, 0.0f, 0.0f, 1.0f}; // Torch position (initialized at the camera)
    GLfloat light_direction[] = {0.0f, 0.0f, -1.0f}; // Torch direction (same as camera direction)
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 25.0f);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.0f);
    glEnable(GL_LIGHT0);
}

void Init_Material() {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    GLfloat material_ambient[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat material_diffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat material_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat material_shininess[] = {32.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
}

void Draw_Cube() {
    // Dimensions (0.5 to half a size)
    const GLfloat vertices[] = {
        -1.0, -1.0, 1.0, // A
        1.0, -1.0, 1.0, // B
        1.0, 1.0, 1.0, // D
        -1.0, 1.0, 1.0, // C

        -1.0, -1.0, -1.0, // E
        1.0, -1.0, -1.0, // F
        1.0, 1.0, -1.0, // H
        -1.0, 1.0, -1.0, // G
    };

    const GLuint indices[] = {
        0, 1, 2, 3, // Front face (ABCD)
        1, 5, 6, 2, // Right face (BFHG)
        7, 6, 5, 4, // Back face (GHFE)
        4, 0, 3, 7, // Left face (EADG)
        3, 2, 6, 7, // Top face (CDHG)
        4, 5, 1, 0, // Bottom face (FEBA)
    };

    const GLfloat normals[] = {
        // Front face normals
        0.0f, 0.0f, 1.0f,   // A
        0.0f, 0.0f, 1.0f,   // B
        0.0f, 0.0f, 1.0f,   // C
        0.0f, 0.0f, 1.0f,   // D

        // Right face normals
        1.0f, 0.0f, 0.0f,   // B
        1.0f, 0.0f, 0.0f,   // F
        1.0f, 0.0f, 0.0f,   // H
        1.0f, 0.0f, 0.0f,   // C

        // Back face normals
        0.0f, 0.0f, -1.0f,  // F
        0.0f, 0.0f, -1.0f,  // E
        0.0f, 0.0f, -1.0f,  // A
        0.0f, 0.0f, -1.0f,  // H

        // Left face normals
        -1.0f, 0.0f, 0.0f,  // E
        -1.0f, 0.0f, 0.0f,  // A
        -1.0f, 0.0f, 0.0f,  // D
        -1.0f, 0.0f, 0.0f,  // G

        // Top face normals
        0.0f, 1.0f, 0.0f,   // D
        0.0f, 1.0f, 0.0f,   // C
        0.0f, 1.0f, 0.0f,   // H
        0.0f, 1.0f, 0.0f,   // G

        // Bottom face normals
        0.0f, -1.0f, 0.0f,  // E
        0.0f, -1.0f, 0.0f,  // F
        0.0f, -1.0f, 0.0f,  // B
        0.0f, -1.0f, 0.0f   // A
    };


    glColor3f(0.9f, 0.0f, 0.9f); // ������� ����

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals); // Provide normals to OpenGL

    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, indices);
    // glDrawArrays(GL_QUADS, 0, 24);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_CULL_FACE);
}

// TODO: remove redundant comments
void Draw_Bulb(GLfloat bulb_color[], GLfloat bulb_width, GLfloat bulb_height, GLfloat bulb_position[]) {
    // Set default color if not provided
    GLfloat default_color[] = {1.0f, 1.0f, 0.0f}; // Yellow color
    if (bulb_color == nullptr) {
        bulb_color = default_color;
    }

    // Set default position if not provided
    GLfloat default_position[] = {0.0f, 0.0f, 0.0f};
    if (bulb_position == nullptr) {
        bulb_position = default_position;
    }

    // Set the color of the bulb with intensity
    GLfloat intensity = 20.f;
    GLfloat bulb_color_with_intensity[3]; // Array to store color with intensity
    for (int i = 0; i < 3; ++i) {
        bulb_color_with_intensity[i] = bulb_color[i] * intensity;
    }

    // Set the position of the bulb light source
    GLfloat light_position[] = {bulb_position[0], bulb_position[1], bulb_position[2], 1.0f};
    // Set as a positional light

    // Set up the light properties
    glEnable(GL_LIGHT1); // Enable the additional light source
    glLightfv(GL_LIGHT1, GL_DIFFUSE, bulb_color_with_intensity); // Make illumination color to bulb_color
    glLightfv(GL_LIGHT1, GL_POSITION, light_position);

    // Set up the light direction
    GLfloat light_direction[] = {0.0f, 0.0f, -1.0f}; // Pointing towards the cube
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light_direction);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 128.0f); // Adjusted exponent for smoother illumination, set to ~5-10

    // Set up the light cutoff angle
    GLfloat light_cutoff = 15.0f; // 10 is also good
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, light_cutoff);

    // Draw the bulb as a simple rectangle
    glColor3fv(bulb_color_with_intensity);
    glPushMatrix();
    glTranslatef(bulb_position[0], bulb_position[1], bulb_position[2]);
    glBegin(GL_QUADS);
    glVertex3f(-bulb_width / 2, -bulb_height / 2, 0.0f);
    glVertex3f(bulb_width / 2, -bulb_height / 2, 0.0f);
    glVertex3f(bulb_width / 2, bulb_height / 2, 0.0f);
    glVertex3f(-bulb_width / 2, bulb_height / 2, 0.0f);
    glEnd();
    glPopMatrix();
}
