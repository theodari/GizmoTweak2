# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

GizmoTweak2 is a node-based laser composition tool. A directed graph connects Shapes (which produce ratios) and Tweaks (which apply effects to Frames). Frames flow from InputNode through a chain of Tweaks to OutputNode, while ratio signals flow laterally from Shape nodes into Tweak ratio ports.

## Build Commands

Build is done via Qt Creator (not command line). Open `CMakeLists.txt` in Qt Creator with Qt 6.x MSVC 64-bit kit.

```bash
# Run all tests (after building in Qt Creator)
ctest --test-dir build/Desktop_Qt_6_X_X_MSVC20XX_64bit-Release -C Release

# Run a single test
./build/Desktop_Qt_6_X_X_MSVC20XX_64bit-Release/test/tst_nodegraph.exe
```

Available test executables (CTest names in parentheses):
- `tst_gizmotweaklib2` (GizmoTweakLib2Tests) - basic library
- `tst_nodegraph` (NodeGraphTests) - graph operations, selection, clipboard, model
- `tst_commands` (CommandsTests) - undo/redo system
- `tst_automation` (AutomationTests) - keyframes and automation tracks
- `tst_port_connection` (PortConnectionTests) - port compatibility edge cases
- `tst_graph_evaluator` (GraphEvaluatorTests) - graph execution
- `tst_evaluate_up_to` (EvaluateUpToTests) - partial evaluation
- `tst_node_formulas` (NodeFormulasTests) - math transformations
- `tst_node_persistence` (NodePersistenceTests) - JSON roundtrip
- `tst_zoom` (ZoomTests) - zoom-to-cursor math

## Architecture

See `doc/ARCHITECTURE.md` for complete details.

### Three-layer structure

1. **lib/** (GizmoTweakLib2 static library) - all logic, no UI
   - `src/core/` - Node, Port, Connection, NodeGraph, GraphEvaluator, Commands
   - `src/nodes/` - 17 concrete node types
   - `src/automation/` - Param, KeyFrame, AutomationTrack
2. **app/** - QML application
   - `src/` - C++ bridge: ExcaliburEngine, FramePreviewItem, NodePreviewItem, FileIO, RecentFilesManager
   - `qml/` - UI components (NodeCanvas, NodeItem, PropertiesPanel, TimelinePanel, etc.)
3. **test/** - Qt Test suite (10 executables)

### Core class relationships

- **Node** (abstract): has input/output Port lists, AutomationTracks, position, category. Virtual `syncToAnimatedValues(timeMs)` for automation. JSON serialization via `propertiesToJson()`/`propertiesFromJson()`.
- **Port**: Direction (In/Out) + DataType (Frame, Ratio2D, Ratio1D, RatioAny, Position). `RatioAny` ports accept both Ratio1D and Ratio2D and track `effectiveDataType` after connection.
- **NodeGraph** (QAbstractListModel): owns all nodes and connections. Factory via `createNode(type)`. Exposes roles: UuidRole, TypeRole, CategoryRole, PositionRole, DisplayNameRole, SelectedRole, NodeRole. QUndoStack for all mutations.
- **GraphEvaluator**: `evaluate()` builds the Frame path (Input→Tweaks→Output), then for each Tweak calls `evaluateRatioChain()` recursively to resolve the connected Shape tree before applying the tweak.
- **Commands**: QUndoCommand subclasses (CreateNode, DeleteNode, MoveNode with merge support, Connect, Disconnect).

### Data flow model

```
InputNode ──Frame──► TweakA ──Frame──► TweakB ──Frame──► OutputNode
                       ▲                  ▲
                    Ratio2D            Ratio1D
                       │                  │
                   GizmoNode      SurfaceFactoryNode
```

Tweaks have 3 input ports: Frame (required, chained), Ratio (RatioAny, from shapes), Position (optional, for gizmo center override). Shape nodes (Gizmo, Transform/Group, SurfaceFactory) feed into Tweak ratio ports. Transform nodes compose multiple ratio inputs with configurable CompositionMode.

### External dependency: ExcaliburEngine

Located at `D:/QtProjects/ExcaliburEngine`. Provides `xengine::Frame` and `xengine::Stack` types. Linked via absolute path in `lib/CMakeLists.txt`. Debug/release libs: `excalibur_engined.lib` / `excalibur_engine.lib`.

### QML integration

- All core classes use `QML_ELEMENT` macro (Qt 6 declarative type registration)
- Two QML modules: `GizmoTweakLib2` (lib) and `GizmoTweak2` (app)
- `Theme.qml` is a singleton (`QT_QML_SINGLETON_TYPE`) providing colors/dimensions
- NodeGraph model drives QML Repeater for NodeItem instances
- FramePreviewItem and NodePreviewItem are QQuickPaintedItem subclasses
- NodePreviewItem has a static `RatioGridCache` shared across instances

### Automation system

`AutomationTrack` manages keyframes (time → parameter values) per node. Keyframes use `QEasingCurve` for interpolation. Before each graph evaluation, `Node::syncToAnimatedValues(timeMs)` reads animated values from tracks into node properties.

## Code Conventions

- Namespace: `gizmotweak2`
- Headers: `#pragma once`
- Private members: `_` prefix
- Style: Allman (braces on new line)
- C++17, use `auto` when appropriate
- QML: use `required property`
- Debug postfix: `d` suffix on library/executable names

## Adding a new node type

1. Create header and source in `lib/src/nodes/` inheriting from `Node`
2. Set category, ports, and properties in constructor
3. Add `QML_ELEMENT` macro
4. Register the type string in `NodeGraph::createNode()` factory and `NodeGraph::availableNodeTypes()`
5. Add source/header to `LIB_SOURCES`/`LIB_HEADERS` in `lib/CMakeLists.txt`
6. For Tweaks: implement `apply(x, y, ratio)` and add handling in `GraphEvaluator::applyTweak()`
7. For Shapes: implement `computeRatio()` and add handling in `GraphEvaluator::evaluateRatioChain()`

## BMad Method

This project uses BMad Method for AI-assisted agile planning and development.

### Available BMad commands
- `/bmad-orchestrator` - Main BMad orchestrator
- `/analyst` - Analysis and research
- `/architect` - Software architecture
- `/pm` - Product Manager
- `/dev` - Developer
- `/qa` - Quality assurance

See `.bmad-core/user-guide.md` for full documentation.
