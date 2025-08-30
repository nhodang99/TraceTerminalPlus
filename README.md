# TraceTerminal++
A tool written in Qt to help tracing and debugging for applications via UDP connection

## Description
TraceTerminal++ is a live-logging viewer that serves greatly for debugging embedded applications, which are difficult to do debug directly on computer.

TraceTerminal++ catches every data sent to its connected interface address via UDP protocol (any host, any IPv6 host, local host,...).
The app will need a feature to send traces (logging/debugging messages) over UDP to use this tool. The traces with special keywords in-line will be highlight with special color for easy debugging ability (" WARNG - ", " ERROR - ",... and some custom highlights).

This tool is inspired by TraceTerminal, a tool written purely in C++ and WindowsAPI, but with more features: the ability to save the file to html (to keep the highlight colors), import files and search in file.

## Usage
- Access menu by right-clicking.
- Set desired network interface and port that you want the tool to receive traces from.
- To open a file:
   - File -> Open or simply Drag and drop trace file to app view.
   - File format .txt and .html is supported.
- To save a file:
   - File -> Save or use shortcut Ctrl+S.
   - File can be saved in .txt or .html format.
- Search shortcuts:
   - Ctrl+F to open normal search mode.
   - Ctrl+Shift+F to open advanced search mode.
- Multiple texts search:
   - Use separator " + " and " | " between texts (text1 | text2 + text3...) to search for multiple texts at once.
   - Operator | takes precedence over +.
   - Example: Syntax: text1 | text2 + text3, we find lines containing (text1) or lines containing (both text2 and text3).
