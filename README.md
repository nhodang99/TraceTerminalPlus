# TraceTerminal++
A tool written in Qt to help tracing and debugging for applications via UDP connection

## Description
TraceTerminal++ is a debugging utility that serves greatly for debugging embedded applications, which are difficult to do debug directly on computer.

TraceTerminal++ will catch every data sent to its connected interface address via UDP protocol (any host, any IPv6 host, local host,...port 911). The embedded app will need a feature to send traces (debugging messages) over UDP to use this tool. The traces with special keywords in-line will be highlight with special color for easy debugging ability (" WARNG - ", " ERROR - ",... and some custom highlights).

This tool is inspired by TraceTerminal, a tool written purely in C++ and WindowsAPI, but with more features: the ability to save the file to html (to keep the highlight colors), import files and search in file.

This is also a side project for me to practice how to build an app from scratch.

## What was done
- Catch traces 'quite' smoothly
- Connect to any inteface, any IPv6 host only, localhost only
- Open and convert txt file, save file

## To do list
- Disable autoscroll working incorrectly if scroll down to the end of the view (only when at the end)
- Sometimes receive the wrong trace order (under monitoring)
- Custom highlights
- Searching
- Connect to remote host only, serial host only
