# Windows Desktop Application Specification
## A* Algorithm Parallelization Visualizer

---

## Application Overview

**Purpose**: Interactive desktop application for visualizing and comparing different A* pathfinding algorithm implementations (sequential vs parallel).

**Target Platform**: Windows 10/11 Desktop

**Backend**: C++ library (imported as DLL)

---

## User Interface Layout

```
┌────────────────────────────────────────────────────────────────────┐
│  A* Algorithm Parallelization                            [_][□][X] │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  ┌─────────────────────┐  ┌────────────────────────────────────┐ │
│  │   CONTROL PANEL     │  │                                    │ │
│  │                     │  │                                    │ │
│  │  Grid Settings:     │  │                                    │ │
│  │   Width:  [1000  ]  │  │                                    │ │
│  │   Height: [1000  ]  │  │                                    │ │
│  │                     │  │                                    │ │
│  │  Algorithm:         │  │                                    │ │
│  │   [▼ Najbolja      ]│  │         GRID VISUALIZATION         │ │
│  │      sekvencijalna  │  │                                    │ │
│  │                     │  │        (Interactive Canvas)        │ │
│  │  Mode Selection:    │  │                                    │ │
│  │   ○ Select Start    │  │    - Zoomable                      │ │
│  │   ○ Select Finish   │  │    - Pannable                      │ │
│  │   ○ Disable Square  │  │    - Click to select squares       │ │
│  │   ○ Set Weights     │  │                                    │ │
│  │                     │  │                                    │ │
│  │  ┌────────────────┐ │  │                                    │ │
│  │  │ START ALGORITHM│ │  │                                    │ │
│  │  └────────────────┘ │  │                                    │ │
│  │                     │  │                                    │ │
│  │  Results:           │  │                                    │ │
│  │   Cost: ------      │  │                                    │ │
│  │   Time: ------ ms   │  │                                    │ │
│  │   Nodes: ------     │  └────────────────────────────────────┘ │
│  └─────────────────────┘                                          │
│                                                                    │
│  Status: Ready                                                     │
└────────────────────────────────────────────────────────────────────┘
```

---

## Detailed Component Specifications

### 1. Control Panel (Left Side)

#### A. Grid Settings

**Width Input**
- Type: Numeric input field
- Range: 10 - 12000
- Default: 1000
- Validation: Must be positive integer
- Behavior: When changed, prompt "Recreate grid?" (will clear current grid)

**Height Input**
- Type: Numeric input field
- Range: 10 - 12000
- Default: 1000
- Validation: Must be positive integer
- Behavior: When changed, prompt "Recreate grid?" (will clear current grid)

#### B. Algorithm Version Selector

**Dropdown Menu** with options:
1. **"Serijski"** (Serial/Sequential - basic A*)
   - Maps to: `Graf::A_star()` from library
   - Description: "Osnovna sekvencijalna implementacija"

2. **"Najbolja sekvencijalna"** (Best Sequential - optimized A*)
   - Maps to: `Graf::A_star_v2()` from library
   - Description: "Optimizovana sekvencijalna verzija (flattened vector)"

3. **"Paralelno"** (Parallel - future implementation)
   - Status: "U izradi" (In development)
   - Disabled/Grayed out for now

4. **"Sve verzije (uporedi)"** (All versions - comparison mode)
   - Runs all available algorithms and shows comparison table
   - Status: "U izradi" (In development)

#### C. Mode Selection (Radio Buttons)

**Mode 1: Select Start** (○ Selektuj početak)
- Behavior: Next click on grid sets start position
- Visual: Selected square colored **GREEN**
- Validation: Only one start allowed (clicking new square moves start)
- Indicator: Mouse cursor changes to green crosshair when hovering grid

**Mode 2: Select Finish** (○ Selektuj kraj)
- Behavior: Next click on grid sets finish position
- Visual: Selected square colored **RED**
- Validation: Only one finish allowed (clicking new square moves finish)
- Indicator: Mouse cursor changes to red crosshair when hovering grid

**Mode 3: Disable Square** (○ Onemogući polje)
- Behavior: Click squares to toggle wall/obstacle
- Visual: Selected squares colored **GRAY**
- Effect: Sets all 8 edge weights to `∞` (infinity)
- Toggle: Click again to re-enable (returns to white/default)
- Multiple: Can disable many squares (click and drag supported)

**Mode 4: Set Weights** (○ Postavi težine)
- Behavior: Click square to open weight editor modal
- See "Weight Editor Modal" section below

#### D. Start Algorithm Button

**Button**: Large, prominent button labeled "START ALGORITHM" / "POKRENI ALGORITAM"

**Behavior**:
- Validates start and finish are set
- If validation fails, shows error message
- If valid, calls C++ library with current grid state
- Shows progress indicator (spinner or progress bar)
- Disables UI during computation (except Cancel button)
- On completion, visualizes path and shows results

**States**:
- Enabled: When start + finish set
- Disabled: When missing start or finish
- Running: Changes to "CANCEL" / "PREKINI" button during execution

#### E. Results Panel

Displays after algorithm completes:

**Path Cost** (Cena puta)
- Format: "Cost: 1234.56"
- Description: Total cumulative weight of shortest path

**Execution Time** (Vreme izvršavanja)
- Format: "Time: 523 ms"
- Description: How long algorithm took to compute

**Nodes Explored** (Pretraženо čvorova)
- Format: "Nodes: 5432"
- Description: Number of nodes examined during search (optional feature)

---

### 2. Grid Visualization (Center/Right)

#### A. Grid Display

**Visual Representation**:
- Each cell is a square
- Grid lines visible (light gray)
- Cells can be colored:
  - **White/Light Gray**: Normal (default, weight = 1.0)
  - **Green**: Start position
  - **Red**: Finish position
  - **Dark Gray**: Wall/Disabled (weight = ∞)
  - **Blue**: Path (after algorithm runs)
  - **Yellow/Orange**: Cells with custom weights (gradient based on weight)

**Interaction**:
- **Click**: Perform action based on selected mode (radio button)
- **Drag**: Paint multiple cells (for Disable mode)
- **Hover**: Show tooltip with cell coordinates and weights
- **Zoom**: Mouse wheel to zoom in/out
- **Pan**: Click and drag with middle mouse or Ctrl+drag to pan

**Coordinate System**:
- Top-left is (0, 0)
- X increases rightward (columns)
- Y increases downward (rows)
- Display coordinates on hover: "(X: 234, Y: 567)"

#### B. Path Visualization

After algorithm completes:
- Highlight path cells in **BLUE** (or draw blue line connecting centers)
- Option: Animate the path (show it being drawn from start to finish)
- Option: Show search animation (visualize nodes being explored in real-time)

---

### 3. Weight Editor Modal

**Triggered**: When user clicks square in "Set Weights" mode

**Modal Window Layout**:
```
┌─────────────────────────────────────────┐
│  Set Edge Weights - Cell (X: 45, Y: 23) │
├─────────────────────────────────────────┤
│                                         │
│              ↖ [1.0]  ↑ [1.0]  ↗ [1.0]  │
│                                         │
│              ← [1.0]  ◉(45,23) → [1.0]  │
│                                         │
│              ↙ [1.0]  ↓ [1.0]  ↘ [1.0]  │
│                                         │
│  ┌──────────────────────────────────┐   │
│  │ Preset Values:                   │   │
│  │  [Normal (1.0)] [Difficult (5.0)]│   │
│  │  [Very Difficult (10.0)] [Wall] │   │
│  │  [Apply to All Directions]       │   │
│  └──────────────────────────────────┘   │
│                                         │
│        [Apply]  [Cancel]  [Reset]       │
└─────────────────────────────────────────┘
```

**Components**:

1. **Central cell** indicator showing selected cell coordinates

2. **8 Input fields** arranged around center:
   - ↑ (Up): Edge to cell (x, y-1)
   - ↗ (Up-Right): Edge to cell (x+1, y-1)
   - → (Right): Edge to cell (x+1, y)
   - ↘ (Down-Right): Edge to cell (x+1, y+1)
   - ↓ (Down): Edge to cell (x, y+1)
   - ↙ (Down-Left): Edge to cell (x-1, y+1)
   - ← (Left): Edge to cell (x-1, y)
   - ↖ (Up-Left): Edge to cell (x-1, y-1)

3. **Input Validation**:
   - Accept: Positive floats (0.1 - 999.9)
   - Special: "∞" or "inf" for infinity (wall)
   - Default: 1.0

4. **Preset Buttons**:
   - "Normal (1.0)": Sets selected direction to 1.0
   - "Difficult (5.0)": Sets selected direction to 5.0
   - "Very Difficult (10.0)": Sets selected direction to 10.0
   - "Wall (∞)": Sets selected direction to infinity
   - "Apply to All Directions": Applies selected preset to all 8 directions

5. **Action Buttons**:
   - "Apply": Save changes and close modal
   - "Cancel": Discard changes and close
   - "Reset": Reset all values to 1.0

**Visual Feedback**:
- After applying, color the cell based on average weight:
  - White: 1.0
  - Light Yellow: 1.1 - 3.0
  - Orange: 3.1 - 7.0
  - Dark Orange: 7.1+
  - Gray: Any edge is ∞

---

## Color Scheme

### Primary Colors:
- **Background**: #F5F5F5 (light gray)
- **Grid Lines**: #CCCCCC (medium gray)
- **Control Panel**: #FFFFFF (white)

### Cell States:
- **Default/Normal**: #FFFFFF (white)
- **Start**: #00FF00 (bright green)
- **Finish**: #FF0000 (bright red)
- **Wall/Disabled**: #808080 (gray)
- **Path**: #0080FF (blue)
- **Weighted (light)**: #FFFF99 (light yellow)
- **Weighted (medium)**: #FFCC66 (orange)
- **Weighted (heavy)**: #FF9933 (dark orange)

### UI Elements:
- **Buttons**: #007ACC (blue) with white text
- **Button Hover**: #005A9E (darker blue)
- **Disabled**: #CCCCCC (gray)
- **Selected Radio**: #007ACC (blue)

---

## Workflow / User Journey

### Typical Usage Flow:

1. **Setup Grid**
   - User enters width (e.g., 1000) and height (e.g., 1000)
   - Grid is generated and displayed

2. **Select Algorithm**
   - User chooses "Najbolja sekvencijalna" from dropdown

3. **Set Start Position**
   - User clicks "Select Start" radio button
   - Clicks on grid at position (10, 10)
   - Cell turns green

4. **Set Finish Position**
   - User clicks "Select Finish" radio button
   - Clicks on grid at position (990, 990)
   - Cell turns red

5. **Add Obstacles** (Optional)
   - User clicks "Disable Square" radio button
   - Clicks and drags across grid to create walls
   - Selected cells turn gray

6. **Set Custom Weights** (Optional)
   - User clicks "Set Weights" radio button
   - Clicks a cell
   - Modal opens showing 8 direction arrows
   - User sets → (right) to 5.0, ↓ (down) to 10.0
   - Clicks "Apply"
   - Cell turns light orange

7. **Run Algorithm**
   - User clicks "START ALGORITHM" button
   - Progress indicator appears
   - After computation (e.g., 234 ms):
     - Path is drawn in blue
     - Results panel shows: Cost: 1543.2, Time: 234 ms
     - Status: "Path found!"

8. **Modify and Re-run**
   - User adds more walls
   - Clicks "START ALGORITHM" again
   - New path is calculated and displayed

---

## Technical Integration with C++ Library

### Data Flow: Frontend → Library

**Step 1: Initialize Grid**
```cpp
// Frontend creates edge weight array
int rows = 1000;
int cols = 1000;
std::vector<float> edges(rows * cols * 8, 1.0f); // All edges default to 1.0
```

**Step 2: Apply User Modifications**
```cpp
// Example: User disabled cell (5, 5) - set all its edges to infinity
int cell_index = 5 * cols + 5;
for(int i = 0; i < 8; i++) {
    edges[cell_index * 8 + i] = std::numeric_limits<float>::infinity();
}

// Example: User set custom weights for cell (10, 10)
// Right edge (direction 0) = 5.0
int cell2_index = 10 * cols + 10;
edges[cell2_index * 8 + 0] = 5.0f; // Right
edges[cell2_index * 8 + 3] = 10.0f; // Down
```

**Step 3: Create Graph and Find Path**
```cpp
Graf graph(edges, rows, cols);
std::pair<int,int> start = {10, 10};   // From user selection
std::pair<int,int> finish = {990, 990}; // From user selection

auto t_start = std::chrono::high_resolution_clock::now();
float cost = graph.A_star_v2(start, finish);
auto t_end = std::chrono::high_resolution_clock::now();

std::chrono::duration<double, std::milli> elapsed = t_end - t_start;
```

**Step 4: Return Results to Frontend**
```cpp
// Need to modify library to return this structure:
struct PathResult {
    bool found;                          // true if path exists
    float cost;                          // total path cost
    std::vector<std::pair<int,int>> path; // coordinates from start to finish
    int nodes_explored;                  // optional: for statistics
};
```

### Data Flow: Library → Frontend

**Frontend receives:**
1. `path` - Vector of (x, y) coordinates
2. `cost` - Float value for display
3. `execution_time` - Measured by frontend
4. `found` - Boolean for success/failure

**Frontend then:**
1. Validates result (check if path is empty)
2. Displays path on grid (color cells blue)
3. Updates results panel
4. Shows success/error message

---

## Error Handling

### Validation Errors:

**No Start Position**
- Message: "Molimo selektujte početnu poziciju" (Please select start position)
- Action: Disable START button until start is set

**No Finish Position**
- Message: "Molimo selektujte krajnju poziciju" (Please select finish position)
- Action: Disable START button until finish is set

**Start = Finish**
- Message: "Početna i krajnja pozicija ne mogu biti iste" (Start and finish cannot be the same)
- Action: Show warning, auto-enable when different

**Grid Too Large**
- Message: "Upozorenje: Mreža >5000×5000 može biti spora" (Warning: Grid >5000×5000 may be slow)
- Action: Show warning but allow

### Runtime Errors:

**No Path Found**
- Message: "Put nije pronađen! Proverite da li su prepreke blokirale put." (Path not found! Check if obstacles blocked the path)
- Action: Show message, allow user to modify grid and retry

**Out of Memory**
- Message: "Nedovoljno memorije za mrežu ove veličine" (Not enough memory for grid this size)
- Action: Suggest reducing grid size

**Timeout** (if implementing)
- Message: "Algoritam predugo izvršava (>30s). Prekinuto." (Algorithm taking too long (>30s). Cancelled.)
- Action: Return to ready state, allow retry

---

## Performance Considerations

### Grid Size Recommendations:

| Grid Size | Expected Time | UI Responsiveness | Notes |
|-----------|---------------|-------------------|-------|
| 100×100 | <10 ms | Instant | Good for testing |
| 500×500 | 10-50 ms | Very smooth | Recommended for demos |
| 1000×1000 | 50-200 ms | Smooth | Default size |
| 5000×5000 | 1-5 seconds | Slight delay | Show progress bar |
| 10000×10000 | 5-30 seconds | Noticeable wait | Async + progress |
| 12000×12000 | 10-60 seconds | Long wait | Async + cancel option |

### UI Optimization:

**For Large Grids (>2000×2000)**:
- Don't render every grid line (render every 10th or 100th)
- Use tiling/virtualization (only render visible portion)
- Update display in chunks during path drawing

**During Computation**:
- Run algorithm in separate thread (C++ std::thread or async)
- Show progress indicator (spinner)
- Keep UI responsive (allow cancel button)

**Memory**:
- Estimate: ~40 bytes per cell (8 floats × 4 bytes + overhead)
- 10000×10000 = ~4 GB memory
- Warn user if grid would exceed available RAM

---

## Future Features (Phase 2)

1. **Save/Load Grid Configurations**
   - Export: `.grid` file format (JSON or binary)
   - Import: Load previous configurations

2. **Parallel Algorithm Comparison**
   - Enable "Paralelno" option
   - Show side-by-side comparison of serial vs parallel performance

3. **Animation Mode**
   - Checkbox: "Animiraj pretragu" (Animate search)
   - Visualize nodes being explored in real-time
   - Color gradient: green → yellow → red (as f(n) increases)

4. **Statistics Dashboard**
   - Graph showing nodes explored over time
   - Heatmap of most-explored areas
   - Comparison table (multiple algorithm runs)

5. **Terrain Presets**
   - "Maze Generator" button
   - "Random Terrain" button
   - "Import from Image" (black pixels = walls)

6. **Heuristic Tuning**
   - Slider for heuristic weight (weighted A*)
   - Compare A* vs Dijkstra vs Greedy

7. **Path Smoothing**
   - Post-process path to reduce zigzag
   - Show original vs smoothed path

---

## Technology Stack Recommendations

### Option 1: C# + WPF (Recommended)

**Pros:**
- Modern Windows UI framework
- Rich controls (Canvas for grid)
- Easy data binding
- XAML for UI design
- C++/CLI wrapper for library integration

**Cons:**
- .NET dependency
- Learning curve for XAML

**Integration:**
```csharp
// C++/CLI Wrapper (PathfinderWrapper.cpp)
public ref class PathfinderWrapper {
    Graf* graph;
public:
    PathfinderWrapper(array<float>^ edges, int rows, int cols);
    PathResult^ FindPath(int x1, int y1, int x2, int y2);
};

// C# WPF (MainWindow.xaml.cs)
PathfinderWrapper pathfinder = new PathfinderWrapper(edgeWeights, rows, cols);
PathResult result = pathfinder.FindPath(startX, startY, endX, endY);
```

### Option 2: C++ + Qt

**Pros:**
- Native C++ (no wrapper needed)
- Cross-platform
- Powerful graphics (QGraphicsView)
- Direct library integration

**Cons:**
- Larger learning curve
- More verbose than WPF
- Licensing (LGPL or commercial)

**Integration:**
```cpp
// Direct integration (no wrapper)
#include "astar.h"

Graf graph(edges, rows, cols);
auto result = graph.A_star_v2(start, finish);
// Directly paint on QGraphicsScene
```

### Option 3: C++ + wxWidgets

**Pros:**
- Native Windows look
- Simpler than Qt
- Direct library integration
- No licensing issues

**Cons:**
- Less modern UI
- More manual work for grid rendering
- Smaller community

---

## Implementation Priority

### Phase 1 (MVP - Minimum Viable Product):
1. ✅ Basic grid display (fixed size: 100×100)
2. ✅ Start/Finish selection
3. ✅ Wall placement (Disable Square)
4. ✅ Algorithm dropdown (only "Najbolja sekvencijalna" enabled)
5. ✅ START button
6. ✅ Path visualization
7. ✅ Results display (cost, time)

### Phase 2 (Enhanced):
1. ✅ Custom grid size input
2. ✅ Zoom and pan
3. ✅ Weight editor modal
4. ✅ Hover tooltips
5. ✅ Error handling

### Phase 3 (Advanced):
1. ✅ Parallel algorithm implementation
2. ✅ Algorithm comparison
3. ✅ Save/Load configurations
4. ✅ Animation mode

---

## Testing Checklist

- [ ] Grid creation (various sizes: 10×10, 100×100, 1000×1000)
- [ ] Start selection (can move start position)
- [ ] Finish selection (can move finish position)
- [ ] Wall placement (single click + drag)
- [ ] Weight editor (all 8 directions)
- [ ] Path finding (simple path on empty grid)
- [ ] Path finding (path around obstacles)
- [ ] No path scenario (finish completely blocked)
- [ ] Start = Finish edge case
- [ ] Out of bounds handling
- [ ] Large grid performance (10000×10000)
- [ ] Zoom in/out functionality
- [ ] Pan functionality
- [ ] Hover tooltips accuracy
- [ ] Results accuracy (cost matches library output)
- [ ] Execution time measurement
- [ ] UI remains responsive during long computations

---

## Summary

This specification provides a complete blueprint for developing the A* pathfinding Windows desktop application. The interface is designed to be intuitive and educational, allowing users to:

1. **Visually configure** grid size, start, finish, walls, and weights
2. **Run algorithms** with different implementations (serial, parallel)
3. **Compare performance** between different approaches
4. **Understand pathfinding** through visualization

The C++ library handles the computational work while the frontend focuses on providing an excellent user experience for interacting with and visualizing the algorithms.
