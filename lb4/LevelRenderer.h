//
// Created by alex0 on 14.04.2024.
//

#ifndef LEVELRENDERER_H
#define LEVELRENDERER_H


#include <vector>
#include <string>
#include <fstream>
#include <GL/gl.h>
#include <iostream>

class Level {
public:
    int texWidth, texHeight;
    GLuint textureID;

    float levelTileSize;
    std::vector<std::string> levelData;

    Level() {};

    Level(const std::string& _levelFilename, float _tileSize, GLuint _textureID = NULL)
        : textureID(_textureID), levelTileSize(_tileSize) {
        levelData = loadLevel(_levelFilename);
    }

    void render() const {
        // std::cout << textureID << ' '
        //           << texWidth <<  ' '
        //           << texHeight << '\n';
        const float LEVEL_TILE_SIZE = 37.5f; // Adjust this value as needed

        // Bind the wood texture
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set the texture environment mode to modulate
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        // Enable necessary OpenGL states
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Iterate over the level data
        for (size_t y = 0; y < levelData.size(); ++y) {
            for (size_t x = 0; x < levelData[y].size(); ++x) {
                if (levelData[y][x] == '#') {
                    // Calculate texture coordinates
                    // Calculate texture coordinates
                    // Calculate texture coordinates
                    float texCoordX = static_cast<float>(x * LEVEL_TILE_SIZE) / static_cast<float>(texWidth);
                    float texCoordY = static_cast<float>(y * LEVEL_TILE_SIZE) / static_cast<float>(texHeight);



                    // Render wall with texture
                    glBegin(GL_QUADS);
                    glTexCoord2f(texCoordX, texCoordY);
                    glVertex2f(x * LEVEL_TILE_SIZE, y * LEVEL_TILE_SIZE);

                    glTexCoord2f(texCoordX + LEVEL_TILE_SIZE / texWidth, texCoordY);
                    glVertex2f((x + 1) * LEVEL_TILE_SIZE, y * LEVEL_TILE_SIZE);

                    glTexCoord2f(texCoordX + LEVEL_TILE_SIZE / texWidth, texCoordY + LEVEL_TILE_SIZE / texHeight);
                    glVertex2f((x + 1) * LEVEL_TILE_SIZE, (y + 1) * LEVEL_TILE_SIZE);

                    glTexCoord2f(texCoordX, texCoordY + LEVEL_TILE_SIZE / texHeight);
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

private:
    std::vector<std::string> loadLevel(const std::string& filename) const {
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
};




#endif //LEVELRENDERER_H
