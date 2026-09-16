HONDA CLINIC - hospitalclinicsystem
====================================

Language: C++17
GUI: Native Windows API (Win32)
Database: None yet
Editor: Visual Studio Code

This version is designed from the user's supplied HTML/CSS screenshots.
The login and role-selection screens use the same simple layout:
- blue left half
- pale gray/white right half
- centered Honda Clinic branding
- Welcome Back card
- simple inputs, checkboxes and buttons

The remaining admin/patient pages keep the same blue, light-gray and white
visual style. Sidebar navigation is handled directly in C++.

IMPORTANT:
No compiler is bundled. The source can be opened in VS Code without installing
anything. To actually RUN a C++ program, Windows still needs a C++ compiler
such as MinGW-w64/G++. The user does not need the full Visual Studio IDE or Qt.

Build command when a compiler is available:
  g++ -std=c++17 main.cpp -municode -mwindows -o hospitalclinicsystem.exe

The program uses only the Windows API and standard C++. No external GUI
library and no database are required.
