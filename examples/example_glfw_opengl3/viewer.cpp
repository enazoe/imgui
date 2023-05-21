#include <iostream>
#include <string>
#include <vector>
#include <experimental/filesystem>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdio.h>
#include "imgui_tex_inspect.h"
#include "tex_inspect_opengl.h"
namespace fs = std::experimental::filesystem;

// Variables
GLFWwindow* window = nullptr;
GLuint imageTexture = 0;
int imageWidth = 0;
int imageHeight = 0;
float zoomLevel = 1.0f;
bool showFileDialog = false;
std::vector<std::string> imageFiles;
bool isMouseDragging = false;
double prevMouseX = 0.0;
double prevMouseY = 0.0;
float offsetX = 0.0f;
float offsetY = 0.0f;

// Function to load image using stb_image
bool LoadImage(const std::string& filePath)
{
    int channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* imageData = stbi_load(filePath.c_str(), &imageWidth, &imageHeight, &channels, 0);
    if (!imageData)
    {
        std::cerr << "Failed to load image: " << filePath << std::endl;
        return false;
    }

    glGenTextures(1, &imageTexture);
    glBindTexture(GL_TEXTURE_2D, imageTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(imageData);
    return true;
}

// Function to display the image using ImGui
void DisplayImage()
{
    ImGui::Begin("Image Viewer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    if (imageTexture != 0)
    {
        ImVec2 imageSize(static_cast<float>(imageWidth) * zoomLevel, static_cast<float>(imageHeight) * zoomLevel);
        ImVec2 imagePos = ImGui::GetCursorScreenPos();//´°¿Ú×óÉÏ½Ç
        std::cout << "imagePos£º" << imagePos.x << "," << imagePos.y << std::endl;

        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(imageTexture)),
            imageSize,
            ImVec2(0, 0),
            ImVec2(1, 1));

        // Check if the mouse is over the image
        if (ImGui::IsItemHovered())
        {
            // Start dragging on left mouse button press
            if (ImGui::IsMouseReleased(0))
            {
                isMouseDragging = false;
            }
            else if (ImGui::IsMouseDown(0) && !isMouseDragging)
            {
                std::cout << "drag" << std::endl;
                isMouseDragging = true;
                prevMouseX = ImGui::GetIO().MousePos.x;
                prevMouseY = ImGui::GetIO().MousePos.y;
                std::cout << "mouse£º" << prevMouseX << "," << prevMouseY << std::endl;
            }

            // Calculate the dragging offset
            if (isMouseDragging)
            {
                double currentMouseX = ImGui::GetIO().MousePos.x;
                double currentMouseY = ImGui::GetIO().MousePos.y;
                offsetX += static_cast<float>(currentMouseX - prevMouseX);
                offsetY += static_cast<float>(currentMouseY - prevMouseY);
                prevMouseX = currentMouseX;
                prevMouseY = currentMouseY;
            }
            // Zoom at the mouse position
            float zoomFactor = 0.1f;
            float zoomOffsetX = static_cast<float>(ImGui::GetIO().MousePos.x - imagePos.x);
            float zoomOffsetY = static_cast<float>(ImGui::GetIO().MousePos.y - imagePos.y);
            zoomOffsetX -= offsetX;
            zoomOffsetY -= offsetY;

            if (ImGui::GetIO().MouseWheel > 0)
            {
                // Zoom in
                offsetX -= zoomOffsetX * zoomFactor;
                offsetY -= zoomOffsetY * zoomFactor;
                zoomLevel += zoomFactor;
            }
            else if (ImGui::GetIO().MouseWheel < 0)
            {
                // Zoom out
                offsetX += zoomOffsetX * zoomFactor;
                offsetY += zoomOffsetY * zoomFactor;
                zoomLevel = std::max(zoomLevel - zoomFactor, 0.1f);
            }
        }

        // Apply the dragging offset to the image position
        ImGui::SetCursorPos(ImVec2(imagePos.x + offsetX, imagePos.y + offsetY));
        //   std::cout << imagePos.x + offsetX<<","<<imagePos.y + offsetY << std::endl;
           //Gui::SetCursorScreenPos(ImVec2(imagePos.x + offsetX, imagePos.y + offsetY));
    }
    else
    {
        ImGui::Text("No image loaded.");
    }

    if (ImGui::Button("Open File"))
        showFileDialog = true;

    ImGui::SameLine();
    if (ImGui::Button("Zoom In"))
        zoomLevel += 0.1f;

    ImGui::SameLine();
    if (ImGui::Button("Zoom Out"))
        zoomLevel = std::max(zoomLevel - 0.1f, 0.1f);

    ImGui::End();

    if (showFileDialog)
    {
        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Open File", &showFileDialog))
        {
            for (const auto& file : imageFiles)
            {
                if (ImGui::Selectable(file.c_str()))
                {
                    LoadImage(file);
                    showFileDialog = false;
                    zoomLevel = 1.0f;
                    offsetX = 0.0f;
                    offsetY = 0.0f;
                }
            }
        }
        ImGui::End();
    }
}

// GLFW mouse wheel callback
void MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoomLevel = std::max(zoomLevel + static_cast<float>(yoffset) * 0.1f, 0.1f);
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += static_cast<float>(xoffset);
    io.MouseWheel += static_cast<float>(yoffset);
}

// GLFW error callback
void ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Initialize GLFW and OpenGL
bool Initialize()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return false;
    }

    glfwSetErrorCallback(ErrorCallback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1280, 720, "Image Viewer", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    /*if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD." << std::endl;
        glfwTerminate();
        return false;
    }*/

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Set mouse wheel callback
    glfwSetScrollCallback(window, MouseWheelCallback);

    return true;
}

// Cleanup GLFW and ImGui resources
void Cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

// Main function
int main()
{
    if (!Initialize())
        return 1;

   // ImGuiTexInspect::ImplOpenGL3_Init(); // Or DirectX 11 equivalent (check your chosen backend header file)
    ImGuiTexInspect::Init();
    ImGuiTexInspect::CreateContext();

    LoadImage("D:/study/imgui/examples/example_glfw_opengl3/Release/demo_1.png");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Simple Texture Inspector");
    /*    ImGuiTexInspect::BeginInspectorPanel("Inspector",reinterpret_cast<void*>(static_cast<intptr_t>(imageTexture)),ImVec2(imageWidth, imageHeight),(ImGuiTexInspect::InspectorFlags)0);
        ImGuiTexInspect::EndInspectorPanel();
        ImGui::End();*/



        static bool flipX = false;
        static bool flipY = false;

        ImGuiTexInspect::InspectorFlags flags = 0;
      //  if (flipX) SetFlag(flags, ImGuiTexInspect::InspectorFlags_FlipX);
      //  if (flipY) SetFlag(flags, ImGuiTexInspect::InspectorFlags_FlipY);

        if (ImGuiTexInspect::BeginInspectorPanel("##ColorFilters", reinterpret_cast<void*>(static_cast<intptr_t>(imageTexture)), ImVec2(imageWidth, imageHeight), flags))
        {
            // Draw some text showing color value of each texel (you must be zoomed in to see this)
            ImGuiTexInspect::DrawAnnotations(ImGuiTexInspect::ValueText(ImGuiTexInspect::ValueText::BytesDec));
        }
        ImGuiTexInspect::EndInspectorPanel();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    Cleanup();
    return 0;
}
