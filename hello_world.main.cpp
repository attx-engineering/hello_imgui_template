#include "hello_imgui/hello_imgui.h"

int main(int , char *[])
{
#ifdef ASSETS_LOCATION
    HelloImGui::SetAssetsFolder(ASSETS_LOCATION);
#endif
    auto guiFunction = []() {
        ImGui::Text("Hello, ");                    // Display a simple label
        HelloImGui::ImageFromAsset("world.jpg");   // Display a static image
        static bool show_goodbye_text = false;
        if (ImGui::Button("Bye!")) {               // Display a button
            // and immediately handle its action if it is clicked!
            // HelloImGui::GetRunnerParams()->appShallExit = true;
            show_goodbye_text = true;
        }
        if (show_goodbye_text) {
            ImGui::Text("Just kidding, you can never leave :)");
        }
     };
    HelloImGui::Run(guiFunction, "Hello, globe", true);
    return 0;
}
