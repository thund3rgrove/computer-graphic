//
// Created by alex0 on 13.04.2024.
//

#ifndef CHARACTER_H
#define CHARACTER_H

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

    Character()
        : x(0.0f), y(0.0f), velocityX(0.0f), velocityY(0.0f), scale(1.0f),
          characterTextureID(0), texWidth(0), texHeight(0) {}

    Character(float _x, float _y, float _scale, GLuint _textureID)
        : x(_x), y(_y), velocityX(0.0f), velocityY(0.0f), scale(_scale),
          characterTextureID(_textureID) {}

    void setTextureID(GLuint _textureID) {
        characterTextureID = _textureID;
    }

    void update(float deltaTime, bool isPaused, const std::vector<std::string>& levelData, float tileSize) {
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

        // Check for collision with walls
        if (collidesWithWalls(levelData, tileSize)) {
            // Handle collision with walls (e.g., stop character's movement)
            velocityX = 0;
            velocityY = 0;
        }

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

    bool collidesWithWalls(const std::vector<std::string>& levelData, float tileSize) const {
        // Convert character position to tile coordinates
        float characterWidth = texWidth / spriteColumnCount * scale;
        float characterHeight = texHeight / spriteRowCount * scale;
        // Calculate character's top-left and bottom-right corners
        float characterLeft = x;
        float characterRight = x + characterWidth;
        float characterTop = y;
        float characterBottom = y + characterHeight;

        // Convert character corners to tile coordinates
        int topLeftTileX = static_cast<int>(characterLeft / tileSize);
        int topLeftTileY = static_cast<int>(characterTop / tileSize);
        int bottomRightTileX = static_cast<int>(characterRight / tileSize);
        int bottomRightTileY = static_cast<int>(characterBottom / tileSize);

        // Check collision for all four corners of the character
        for (int tileY = topLeftTileY; tileY <= bottomRightTileY; ++tileY) {
            for (int tileX = topLeftTileX; tileX <= bottomRightTileX; ++tileX) {
                // Check if tile is within the level bounds
                if (tileX >= 0 && tileX < levelData[0].size() && tileY >= 0 && tileY < levelData.size()) {
                    // Check if the tile is a wall
                    if (levelData[tileY][tileX] == '#') {
                        return true; // Collision detected
                    }
                } else {
                    return true; // Collision with level boundary
                }
            }
        }

        return false; // No collision detected
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

#endif //CHARACTER_H
