# A* Pathfinding Library - Integration Guide for Frontend Application

## Executive Summary

This is a **high-performance A* pathfinding library** written in C++ that finds the shortest path between two points on a 2D grid with weighted edges. It's designed for large-scale pathfinding (tested up to 12,000×12,000 grids = 144M nodes) and includes two optimized implementations.

---

## Library Overview

### What This Library Does
- **Input**: Grid dimensions, edge weights, start position, end position
- **Output**: The cost (length) of the shortest path from start to finish
- **Algorithm**: A* pathfinding with Euclidean distance heuristic
- **Grid Type**: 8-directional movement (up, down, left, right, and 4 diagonals)

### Current Limitations (That Need to Be Addressed)
⚠️ **The library currently only returns the path COST, not the actual PATH itself**
- This needs to be modified to return the sequence of coordinates from start to finish
- See "Required Modifications" section below

---

## Core Data Structures

### 1. Graph Representation

The library represents a grid-based graph where:
- **Grid**: `rows × cols` cells (nodes)
- **Each cell**: Has up to 8 edges (connections to neighbors)
- **Edge weights**: Float values representing movement cost
  - `1.0` = normal terrain
  - `>1.0` = difficult terrain (higher cost)
  - `∞` (infinity) = wall/obstacle (impassable)

### 2. Two Storage Formats

#### Option A: 2D Vector (Used by `A_star()`)
```cpp
vector<vector<float>> edges(rows * cols, vector<float>(8))
```
- Node at position (x, y) → index = `x * cols + y`
- Its 8 edge weights stored at: `edges[index][0..7]`

**Edge Direction Mapping:**
```
Index | Direction | Offset (dx, dy)
------|-----------|----------------
  0   | Right     | (0, +1)
  1   | Up        | (-1, 0)
  2   | Left      | (0, -1)
  3   | Down      | (+1, 0)
  4   | Up-Right  | (-1, +1)
  5   | Up-Left   | (-1, -1)
  6   | Down-Right| (+1, +1)
  7   | Down-Left | (+1, -1)
```

#### Option B: Flattened Vector (Used by `A_star_v2()`)
```cpp
vector<float> edges_flat(rows * cols * 8)
```
- Node (x, y) edges start at: `(x * cols + y) * 8`
- **Performance**: 20-30% faster due to better CPU cache locality

---

## Library API (Current)

### Class: `Graf`

```cpp
class Graf {
public:
    // Constructor for 2D vector representation
    Graf(const std::vector<std::vector<float>> &edges, int rows, int cols);

    // Constructor for flattened vector representation (FASTER)
    Graf(const std::vector<float> &edges_flat, int rows, int cols);

    // Find shortest path (returns cost only - NEEDS MODIFICATION)
    float A_star(std::pair<int,int> start, std::pair<int,int> finish);
    float A_star_v2(std::pair<int,int> start, std::pair<int,int> finish);
};
```

### Current Usage Example

```cpp
const int rows = 100;
const int cols = 100;

// Create grid with all edges = 1.0 (uniform cost)
std::vector<float> edges(rows * cols * 8, 1.0f);

// Add some walls (infinite cost)
int wall_node = 50 * cols + 25; // Node at (50, 25)
for(int i = 0; i < 8; i++) {
    edges[wall_node * 8 + i] = std::numeric_limits<float>::infinity();
}

// Create graph
Graf graph(edges, rows, cols);

// Find path from (0,0) to (99,99)
std::pair<int,int> start = {0, 0};
std::pair<int,int> finish = {99, 99};
float cost = graph.A_star_v2(start, finish);

if(cost >= 0) {
    std::cout << "Path found with cost: " << cost << "\n";
} else {
    std::cout << "No path exists!\n";
}
```

---

## Required Modifications for Frontend Integration

### 1. **Add Path Reconstruction**

**Current Issue**: Only returns cost, not the actual path

**What's Needed**: Modify functions to return `vector<pair<int,int>>` containing coordinates from start to finish

**Implementation**: Use the `obradjeniCvorovi` (processed nodes) structure which already tracks parent pointers - backtrack from finish to start

### 2. **Create Library Interface (.h/.cpp separation)**

Currently everything is in one file. For library use, need:

```
PathfindingLib/
├── include/
│   └── astar.h          // Public API header
├── src/
│   └── astar.cpp        // Implementation
└── CMakeLists.txt       // Build configuration
```

### 3. **Export as DLL/Shared Library for Windows**

Add export declarations for Windows:
```cpp
#ifdef _WIN32
    #define PATHFINDING_API __declspec(dllexport)
#else
    #define PATHFINDING_API
#endif

class PATHFINDING_API Graf { ... };
```

### 4. **Simplified API for Frontend**

Suggested cleaner interface:

```cpp
struct PathResult {
    bool found;                          // Was path found?
    float cost;                          // Total path cost
    std::vector<std::pair<int,int>> path; // Sequence of coordinates
    int nodes_explored;                  // For statistics
};

class PATHFINDING_API Pathfinder {
public:
    Pathfinder(int rows, int cols);

    // Set edge weight for a specific node and direction
    void SetEdgeWeight(int x, int y, int direction, float weight);

    // Set entire grid from array (for bulk initialization)
    void SetGridWeights(const float* weights, size_t size);

    // Find path
    PathResult FindPath(int start_x, int start_y, int end_x, int end_y);

    // Helper: Mark cell as wall (all edges = infinity)
    void SetWall(int x, int y);

    // Clear grid (reset all weights to 1.0)
    void Clear();
};
```

---

## Frontend Application Requirements

### What the Frontend Needs to Provide

1. **Grid Dimensions**
   - Width (columns) and Height (rows)
   - Example: 1000×1000 for a large map

2. **Edge Weights Data**
   - Float array of size `rows × cols × 8`
   - Or: Functions to mark cells as walls/terrain types

3. **Start and End Positions**
   - Coordinates (x, y) where 0 ≤ x < rows and 0 ≤ y < cols

### What the Frontend Receives

1. **Path Coordinates** (after modification)
   - List of (x, y) positions from start to finish
   - Example: `[(0,0), (1,1), (2,1), (2,2), ...]`

2. **Path Cost**
   - Total cumulative weight along the path

3. **Success/Failure Status**
   - Whether a path exists

### Performance Expectations

Based on the current implementation:
- **Grid size**: Up to 12,000×12,000 tested successfully
- **Execution time**:
  - 10,000×10,000 grid: ~1-5 seconds (depending on complexity)
  - 1,000×1,000 grid: ~10-100ms
- **Memory**: Approximately `rows × cols × 40 bytes` (depends on heap size during search)

---

## Integration Options for Windows Desktop Application

### Option 1: C++/CLI Wrapper (For C# Frontend)

If using C#/WPF/WinForms:
```csharp
// C++/CLI wrapper exposes library to .NET
public ref class PathfinderWrapper {
public:
    PathfinderWrapper(int rows, int cols);
    List<Point>^ FindPath(int x1, int y1, int x2, int y2);
    void SetWall(int x, int y);
};
```

### Option 2: Native C++ Frontend

Use Qt or wxWidgets:
```cpp
#include "astar.h"

Pathfinder pathfinder(1000, 1000);
PathResult result = pathfinder.FindPath(0, 0, 999, 999);

// Draw path on UI
for(auto [x, y] : result.path) {
    DrawPixel(x, y, COLOR_PATH);
}
```

### Option 3: C API + FFI

For other languages (Python, Rust, etc.):
```c
// C wrapper API
extern "C" {
    void* pathfinder_create(int rows, int cols);
    int pathfinder_find_path(void* pf, int x1, int y1, int x2, int y2,
                              int* path_x, int* path_y, int max_length);
    void pathfinder_destroy(void* pf);
}
```

---

## Data Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    FRONTEND APPLICATION                     │
│  (Windows Desktop: WPF/WinForms/Qt/wxWidgets)               │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          │ 1. Initialize Grid (rows, cols)
                          │ 2. Set Edge Weights / Walls
                          │ 3. Call FindPath(start, end)
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                   A* PATHFINDING LIBRARY                    │
│                        (C++ DLL)                            │
│                                                             │
│  ┌─────────────┐         ┌──────────────┐                  │
│  │   Graf      │────────▶│   A_star_v2  │                  │
│  │   Class     │         │   Algorithm  │                  │
│  └─────────────┘         └──────────────┘                  │
│        │                                                    │
│        │ Uses                                               │
│        ▼                                                    │
│  ┌──────────────────────────────────┐                      │
│  │  Flattened Vector Storage        │                      │
│  │  (rows × cols × 8 floats)        │                      │
│  └──────────────────────────────────┘                      │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          │ Returns: Path coordinates + cost
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                   VISUALIZATION LAYER                       │
│  - Draw grid                                                │
│  - Highlight path                                           │
│  - Show statistics (cost, nodes explored)                   │
└─────────────────────────────────────────────────────────────┘
```

---

## Example Frontend Use Cases

### Use Case 1: Interactive Grid Editor

```
User Actions:
1. Create 500×500 grid
2. Click cells to toggle walls
3. Click "Start" position
4. Click "End" position
5. Press "Find Path" button

Library Calls:
→ Pathfinder(500, 500)
→ SetWall(x, y) for each wall click
→ FindPath(start_x, start_y, end_x, end_y)

Frontend Receives:
← PathResult{found=true, cost=523.4, path=[...], nodes_explored=1523}

Frontend Draws:
- Green line through path coordinates
- Shows "Path Length: 523.4" label
```

### Use Case 2: Game Map Navigation

```
Game Scenario:
- 2000×2000 tile map
- Different terrain costs (grass=1, sand=2, water=∞)
- Unit at (100, 100) needs to reach (1800, 1900)

Library Calls:
→ Pathfinder(2000, 2000)
→ SetGridWeights(terrain_cost_array)
→ FindPath(100, 100, 1800, 1900)

Frontend Uses:
- Move unit along returned path coordinates
- Animate movement frame-by-frame
- Display estimated time (cost / unit_speed)
```

### Use Case 3: Maze Solver

```
Input: Binary maze image (white=passable, black=wall)

Processing:
1. Convert image to grid (1920×1080)
2. Set edges:
   - White pixels: weight = 1.0
   - Black pixels: weight = ∞
3. FindPath(0, 0, 1919, 1079)
4. Draw red line on original image showing solution
```

---

## Prompt for AI to Implement Frontend

**Use this prompt when working with AI to create the frontend:**

```
I need to create a Windows desktop application that uses an A* pathfinding
library. The library is written in C++ and provides the following functionality:

LIBRARY CAPABILITIES:
- Finds shortest path on a 2D grid (up to 12,000×12,000 cells tested)
- 8-directional movement (cardinal + diagonal directions)
- Supports weighted edges (different terrain costs)
- Supports walls/obstacles (infinite cost edges)
- High performance (uses flattened vector for cache optimization)

LIBRARY API (needs to be modified from current state):
- Currently returns only path COST (float)
- Needs modification to return actual PATH (vector of coordinates)
- Current interface: Graf class with A_star_v2() method
- Suggested new interface: Pathfinder class with FindPath() returning PathResult

FRONTEND REQUIREMENTS:
1. Visual grid display (zoomable, pannable)
2. Click/drag to place walls
3. Set start position (green marker)
4. Set end position (red marker)
5. "Find Path" button
6. Visualize found path (blue line or highlighted cells)
7. Display statistics (path cost, nodes explored, computation time)
8. Grid size selector (100x100, 500x500, 1000x1000, custom)
9. Terrain type selector (normal=1.0, difficult=2.0, wall=∞)
10. Export/import grid configurations

TECHNICAL INTEGRATION:
- Library compiled as C++ DLL (Windows)
- Need C++/CLI wrapper if using C#, or direct C++ if using Qt/wxWidgets
- Grid data stored as float array: rows × cols × 8 values
- Each cell has 8 edge weights (one per direction)

PERFORMANCE:
- Should handle real-time pathfinding on grids up to 1000×1000
- Larger grids may need progress indicator
- Expected latency: 10-100ms for 1000×1000, 1-5s for 10,000×10,000

Please help me:
1. Choose appropriate GUI framework (WPF/Qt/wxWidgets)
2. Design the integration layer (DLL wrapper)
3. Implement efficient grid visualization
4. Handle user interactions (wall placement, start/end selection)
5. Call pathfinding library and visualize results

Current library files: main.cpp (single file implementation)
Target platform: Windows 10/11 desktop
```

---

## Next Steps

### Immediate Actions Needed:

1. ✅ **Modify A_star_v2()** to return path coordinates (not just cost)
2. ✅ **Split into header/implementation** (astar.h / astar.cpp)
3. ✅ **Add DLL export macros** for Windows
4. ✅ **Create simplified Pathfinder API** (easier to use than Graf)
5. ✅ **Add CMake build system** for cross-platform compilation
6. ✅ **Write unit tests** (basic paths, no path scenarios, edge cases)

### Then Choose Frontend Approach:

**Option A: C# + WPF** (Recommended for Windows)
- Modern UI framework
- Easy to create rich visualizations
- Need C++/CLI wrapper

**Option B: C++ + Qt**
- Cross-platform potential
- Direct library integration (no wrapper needed)
- Steeper learning curve

**Option C: C++ + wxWidgets**
- Native Windows look and feel
- Direct library integration
- Simpler than Qt

---

## Questions to Answer Before Frontend Development

1. **What's the primary use case?**
   - Educational tool (maze solving demonstration)?
   - Game development (NPC pathfinding)?
   - Research tool (algorithm comparison)?

2. **What's the expected grid size?**
   - Small (100×100) → prioritize UI responsiveness
   - Large (10,000×10,000) → need async computation + progress bars

3. **What visualization features are essential?**
   - Just show final path?
   - Animate the search process (nodes being explored)?
   - Multiple paths comparison?

4. **Performance requirements?**
   - Real-time (user drags start/end, path updates live)?
   - Batch mode (load configuration, compute, show result)?

5. **Additional features needed?**
   - Save/load maps?
   - Different algorithms (Dijkstra, BFS) for comparison?
   - Heuristic weight adjustment (weighted A*)?

---

## Conclusion

This A* library provides a solid, high-performance pathfinding core. With the modifications outlined above (primarily adding path reconstruction and creating a clean API), it will be ready for integration into a Windows desktop application. The frontend's main responsibilities will be:

1. **Visualization** → drawing the grid, walls, and path
2. **User Interaction** → handling mouse/keyboard input
3. **Data Management** → converting UI state to library's edge weight format
4. **Result Presentation** → displaying the computed path and statistics

The library handles all the complex algorithmic work efficiently, so the frontend can focus on providing an intuitive user experience.
