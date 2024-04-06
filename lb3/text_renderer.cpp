#include "text_renderer.h"
#include <cstdio>

#include "Button.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb-master/stb_truetype.h"

unsigned char ttf_buffer[1 << 25]; // You may need to adjust the buffer size
unsigned char temp_bitmap[512 * 512];
GLuint fontTextureID;
stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

void loadFont(const char *fontPath, int fontSize) {
    FILE *file = fopen(fontPath, "rb");
    if (!file) {
        std::cerr << "Failed to open font file: " << fontPath << std::endl;
        return;
    }
    fread(ttf_buffer, 1, 1 << 25, file);
    fclose(file);

    stbtt_BakeFontBitmap(ttf_buffer, 0, fontSize, temp_bitmap, 512, 512, 32, 96, cdata); // ASCII 32 to 126
    glGenTextures(1, &fontTextureID);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void renderText(const char *text, float x, float y, float scale) {
    std::cout << "Rendering text: " << text << " at position (" << x << ", " << y << ")" << std::endl;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glBegin(GL_QUADS);
    while (*text) {
        if (*text >= 32 && *text < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512, 512, *text - 32, &x, &y, &q, 1);
            glTexCoord2f(q.s0, q.t1);
            glVertex2f(q.x0 * scale, q.y1 * scale);
            glTexCoord2f(q.s1, q.t1);
            glVertex2f(q.x1 * scale, q.y1 * scale);
            glTexCoord2f(q.s1, q.t0);
            glVertex2f(q.x1 * scale, q.y0 * scale);
            glTexCoord2f(q.s0, q.t0);
            glVertex2f(q.x0 * scale, q.y0 * scale);
        }
        ++text;
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void renderOverlay(const char *text, float x, float y, float width, float height, float scale, float opacity/*, Button *buttons*/) {
    int numButtons = 3;// sizeof(buttons) / sizeof(buttons[0]);
    // std::cout << numButtons << '\n';
    char* buttonText[] = {"Something 1", "Something 2", "Quit"};

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw a translucent rectangle for the overlay
    glColor4f(0.2f, 0.2f, 0.2f, opacity); // Adjust color and opacity as needed
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Render text at the top of the overlay
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Set text color to white
    renderText(text, x + width / 2 - (strlen(text) * scale * 12) / 2, y + 20, scale);

    // Calculate the height for each button
    float buttonHeight = height / (numButtons + 1);

    // Render rectangles for buttons
    for (int i = 0; i < numButtons; ++i) {
        glColor4f(0.4f, 0.4f, 0.4f, 1.0f); // Set button color
        glBegin(GL_QUADS);
        glVertex2f(x + 20, y + 50 + (i + 1) * buttonHeight);
        glVertex2f(x + width - 20, y + 50 + (i + 1) * buttonHeight);
        glVertex2f(x + width - 20, y + 50 + (i + 2) * buttonHeight);
        glVertex2f(x + 20, y + 50 + (i + 2) * buttonHeight);
        glEnd();

        // Render button text
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Set text color to white
        renderText(buttonText[i], x + width / 2 - (strlen(buttonText[i]) * scale * 12) / 2, y + 50 + (i + 1) * buttonHeight + 10, scale);
    }

    // Disable blending after rendering the overlay
    glDisable(GL_BLEND);
}
