#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <windows.h>
#include <thread>


# pragma region Game variables

// Size of game grid and window
const int gridSize = 10;
const int windowSize = 420;

// Sound disabled
bool playSounds = false;

// Game speed and state variables
float updateDelay = 0.2f;
float lastFrameTime = 0;
bool gameOver = false;

// Snake position, direction and size
int snakeX = gridSize / 2;
int snakeY = gridSize / 2;
int SnakeDirX = 0;
int SnakeDirY = 0;
std::vector<std::pair<int, int>> snakeBody;
bool increaseLength = false;
bool inputWaiting = false;

// Apple position
int appleX = 5;
int appleY = 5;

#pragma endregion

#pragma region Gameplay functions

// Function for playing sounds
void playBeep(int frequency, int duration)
{
    if (playSounds)
    {
        std::thread([frequency, duration] {
            Beep(frequency, duration);
            std::this_thread::sleep_for(std::chrono::milliseconds(duration + 200));
            }).detach();
    }
}

// Function for setting apple position
void placeApple()
{
    do
    {
        appleX = rand() % gridSize;
        appleY = rand() % gridSize;
    } while (std::find_if(snakeBody.begin(), snakeBody.end(),
        [&](const auto& segment)
        {
            return segment.first == appleX && segment.second == appleY;
        }) != snakeBody.end());
}



// Function for (re)setting the game
void initGame() 
{
    updateDelay = 0.2f;
    snakeBody.clear();
    snakeX = gridSize / 2;
    snakeY = gridSize / 2;
    snakeBody.push_back({ snakeX, snakeY });
    SnakeDirX = 0;
    SnakeDirY = 0;
    gameOver = false;
    inputWaiting = false;
    placeApple();
}

// Function for checking for game over state
bool checkCollisionWithSelf() 
{
    for (size_t i = 1; i < snakeBody.size(); ++i) 
    {
        if (snakeX == snakeBody[i].first && snakeY == snakeBody[i].second) 
        {
            playBeep(300, 1000);
            return true;
        }
    }

    return false;
}

// Function for gameplay loop
void update(float deltaTime) 
{
    static float accumulator = 0.0f;
    accumulator += deltaTime;

    if (accumulator >= updateDelay) 
    {
        accumulator -= updateDelay;

        snakeX += SnakeDirX;
        snakeY += SnakeDirY;

        // Check for game over state;
        if (checkCollisionWithSelf()) 
        {
            gameOver = true;
        }

        // Wrap-around logic
        if (snakeX >= gridSize) snakeX = 0;
        else if (snakeX < 0) snakeX = gridSize - 1;
        if (snakeY >= gridSize) snakeY = 0;
        else if (snakeY < 0) snakeY = gridSize - 1;

        // Move snake and grow if needed
        if (!increaseLength)
        {
            snakeBody.erase(snakeBody.begin());
        }
        else 
        {
            updateDelay = updateDelay * 0.95f;
        }

        snakeBody.push_back(std::make_pair(snakeX, snakeY));

        // Check for apple collision
        if (snakeX == appleX && snakeY == appleY) 
        {
            increaseLength = true;
            playBeep(1000, 500);
            placeApple();
        }
        else 
        {
            increaseLength = false;
        }

        // Allow inputting new direction
        inputWaiting = false;
    }
}

#pragma endregion

#pragma region Callback functions

// Function to be called whenever an error occurs
void glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

// Function for getting hand handling player inputs
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    if (action == GLFW_PRESS && !inputWaiting) {
        switch (key) {
        case GLFW_KEY_W: if (SnakeDirY == 0) { SnakeDirX = 0; SnakeDirY = -1; inputWaiting = true; } break;
        case GLFW_KEY_S: if (SnakeDirY == 0) { SnakeDirX = 0; SnakeDirY = 1; inputWaiting = true; } break;
        case GLFW_KEY_A: if (SnakeDirX == 0) { SnakeDirX = -1; SnakeDirY = 0; inputWaiting = true; } break;
        case GLFW_KEY_D: if (SnakeDirX == 0) { SnakeDirX = 1; SnakeDirY = 0; inputWaiting = true; } break;
        }
    }
}

#pragma endregion

#pragma region Drawing functions


// Function for drawing the game grid
void drawGrid()
{
    glColor3f(0.3f, 0.3f, 0.3f);

    for (int i = 0; i <= gridSize; ++i)
    {
        glBegin(GL_LINES);
        glVertex2f(i, 0);
        glVertex2f(i, gridSize);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(0, i);
        glVertex2f(gridSize, i);
        glEnd();
    }

    glColor3f(1.0f, 1.0f, 1.0f);
}

// Function for drawing the snake
void drawSnake() 
{
    for (auto& part : snakeBody)
    {
        glColor3f(1.0, 0, 0);
        glRectf(part.first, part.second, part.first + 1, part.second + 1);
        glColor3f(1.0, 1.0, 1.0);

    }
}

// Function for drawing the apple
void drawApple() 
{
    glColor3f(0.0, 1.0, 0);
    glRectf(appleX, appleY, appleX + 1, appleY + 1);
    glColor3f(1.0, 1.0, 1.0); // Reset color to white for the rest of the drawing
}

//  Function for drawing the game
void draw() 
{
    glClear(GL_COLOR_BUFFER_BIT);
    drawGrid();
    drawSnake();
    drawApple();
}

#pragma endregion

// Main function
int main(void) 
{
    // Set the GLFW error callback 
    glfwSetErrorCallback(glfwErrorCallback);

    // Define a pointer to a GLFW window
    GLFWwindow* window;

    // Initialize the random number generator with current time
    srand(static_cast<unsigned int>(time(nullptr)));

    // Initialize the GLFW library, exit if fails
    if (!glfwInit()) return -1;

    // Disable window resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create a window with a specified size and title, for the game, exit if fails
    window = glfwCreateWindow(windowSize, windowSize, "Snake Game", NULL, NULL);

    if (!window) 
    {
        glfwTerminate();
        return -1;
    }

    // Make the created window's OpenGL context current on the calling thread
    glfwMakeContextCurrent(window);

    // Set the function to be called when a key is pressed or released
    glfwSetKeyCallback(window, keyCallback);

    // Initialize the game
    initGame();

    // Set projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, gridSize, gridSize, 0, -1, 1); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Gameplay
    while (!glfwWindowShouldClose(window))
    {
        // Calculate time since last frame
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Process events such as input
        glfwPollEvents();

        // Update the game logic based on elapsed time
        update(deltaTime);

        // Draw the game state
        draw();

        // Swap the front and back buffers to display the rendered frame
        glfwSwapBuffers(window);

        // If the game is over, reset the game state
        if (gameOver) {
            initGame();
        }
    }

    // Clean up and terminate GLFW and exit
    glfwTerminate();
    return 0;
}