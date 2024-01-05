# TraceTerminal++
A tool written in Qt to help tracing and debugging for applications via UDP connection

## Description
TraceTerminal++ is a live-logging viewer that serves greatly for debugging embedded applications, which are difficult to do debug directly on computer.

TraceTerminal++ catches every data sent to its connected interface address via UDP protocol (any host, any IPv6 host, local host,...). The app will need a feature to send traces (logging/debugging messages) over UDP to use this tool. The traces with special keywords in-line will be highlight with special color for easy debugging ability (" WARNG - ", " ERROR - ",... and some custom highlights).

This tool is inspired by TraceTerminal, a tool written purely in C++ and WindowsAPI, but with more features: the ability to save the file to html (to keep the highlight colors), import files and search in file.

This is a side project for me to practice how to build an app from scratch.

## What was done
- Catch traces 'quite' smoothly
- Connect to any inteface, any IPv6 host only, localhost only, or a specific address
- Open and convert txt file, save file in both rich text and plain text
- Custom highlights
- Searching in file

## Todo list
- Handle trace scrolling effect
- Create shortcut to connect remote host only, serial host only
- Handle trace selection by mouse and keyboard
