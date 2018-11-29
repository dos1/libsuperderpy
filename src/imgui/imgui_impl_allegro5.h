// dear imgui: Renderer + Platform Binding for Allegro 5
// (Info: Allegro 5 is a cross-platform general purpose library for handling windows, inputs, graphics, etc.)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ALLEGRO_BITMAP*' as ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.
//  [X] Platform: Clipboard support (from Allegro 5.1.12)
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
// Issues:
//  [ ] Renderer: The renderer is suboptimal as we need to unindex our buffers and convert vertices manually.
//  [ ] Platform: Missing gamepad support.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui, Original Allegro 5 code by @birthggd

#pragma once

#ifdef __cplusplus
#include "3rdparty/cimgui/imgui/imgui.h"
#else
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS 1
#endif

#include "3rdparty/cimgui/cimgui.h"
#include "libsuperderpy.h"
#include <stdbool.h>

bool ImGui_ImplAllegro5_Init(ALLEGRO_DISPLAY* display);
void ImGui_ImplAllegro5_Shutdown(void);
void ImGui_ImplAllegro5_NewFrame(void);
void ImGui_ImplAllegro5_RenderDrawData(struct ImDrawData* draw_data);
bool ImGui_ImplAllegro5_ProcessEvent(ALLEGRO_EVENT* event);

// Use if you want to reset your rendering device without losing ImGui state.
bool ImGui_ImplAllegro5_CreateDeviceObjects(void);
void ImGui_ImplAllegro5_InvalidateDeviceObjects(void);
