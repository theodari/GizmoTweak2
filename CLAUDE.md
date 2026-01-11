# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

GizmoTweak2 is a node-based refactoring of GizmoTweak for laser composition. It uses a directed graph where Shapes produce ratios and Tweaks apply effects to Frames.

## Build Commands

Build is done via Qt Creator (not command line). Open `CMakeLists.txt` in Qt Creator with Qt 6.x MSVC 64-bit kit.

```bash
# Run tests (after building in Qt Creator)
ctest --test-dir build -C Release

# Run a single test
./build/Desktop_Qt_X_X_X_MSVC20XX_64bit-Release/test/tst_nodegraph.exe
```

## Project Structure

```
GizmoTweak2/
├── lib/                    # GizmoTweakLib2 static library
│   └── src/
│       ├── core/           # Node, Port, Connection, NodeGraph
│       └── gizmotweaklib2.h
├── app/                    # QML application
│   ├── src/main.cpp
│   └── qml/                # QML components
│       ├── Main.qml
│       ├── Theme.qml       # Singleton colors/dimensions
│       ├── NodeCanvas.qml
│       ├── NodeItem.qml
│       ├── PortItem.qml
│       ├── ConnectionItem.qml
│       └── ConnectionPreview.qml
├── test/                   # Qt Test tests
└── doc/                    # Documentation
```

## Architecture

See `doc/ARCHITECTURE.md` for complete details.

**Core classes** (lib/src/core/):
- `Node`: Base abstract class with ports and position
- `Port`: Connection point with Direction (In/Out) and DataType (Frame, Ratio2D, Ratio1D)
- `Connection`: Links two compatible ports
- `NodeGraph`: QAbstractListModel for QML, manages nodes and connections

**Data flow**: Input → Tweaks (chained via Frame connections) → Output

**Node categories** (Node::Category):
- IO: Input, Output
- Shape: Gizmo, Transform, SurfaceFactory
- Utility: TimeShift
- Tweak: Position, Scale, Rotation, etc.

## QML Architecture

- Backend C++ classes expose Q_PROPERTY for QML bindings
- NodeGraph is a QAbstractListModel for the Repeater
- Connections use Qt Quick Shapes with PathCubic for Bézier curves
- Theme.qml singleton provides colors (SurfaceFactory style dark theme)

## Code Conventions

- Namespace: `gizmotweak2`
- Headers: `#pragma once`
- Private members: `_` prefix
- Style: Allman (braces on new line)
- C++17, use `auto` when appropriate
- QML: use required property

## Dependencies

- Qt 6.x (Core, Gui, Quick, QuickControls2, Shapes, Test)
- MSVC 2019/2022
- CMake 3.16+
