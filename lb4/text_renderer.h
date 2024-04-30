//
// Created by alex0 on 05.04.2024.
//

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <GL/gl.h>

#include "Button.h"

// Function declarations
void loadFont(const char *fontPath, int fontSize);
void renderText(const char *text, float x, float y, float scale);
void renderOverlay(const char *text, float x, float y, float width, float height, float scale, float opacity/*, Button *buttons[]*/);

#endif // TEXT_RENDERER_H

