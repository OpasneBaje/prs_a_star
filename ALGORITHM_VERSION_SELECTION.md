# Algorithm Version Selection Guide for Qt Application

## 📊 Current Situation

Your **local main.cpp** currently has:
- ✅ `A_star` - Basic version (2D vector)
- ✅ `A_star_v2` - Optimized version (flattened vector)

Your **origin/main** branch has additional versions:
- ✅ `A_star_v3` - With path reconstruction + DaryHeap
- ✅ `A_star_v4` - With SIMD AVX2 + path reconstruction

**To get the latest versions:**
```bash
git pull origin main
```

---

## 🎯 Algorithm Versions Overview

### Version 1: A_star (Serijski - Basic Sequential)
```cpp
float A_star(std::pair<int,int> start, std::pair<int,int> finish)
```

**Features:**
- Uses 2D vector storage
- Standard `std::priority_queue`
- Returns: **Cost only** (no path)

**Use Case:** Testing, comparison baseline

**Performance:** Baseline (slowest)

---

### Version 2: A_star_v2 (Najbolja sekvencijalna - Best Sequential)
```cpp
float A_star_v2(std::pair<int,int> start, std::pair<int,int> finish)
```

**Features:**
- Uses **flattened vector** (cache-friendly)
- Smart heap insertion (only adds if better f(n))
- Returns: **Cost only** (no path)

**Use Case:** When you only need path cost, not the actual path

**Performance:** ~20-30% faster than v1

---

### Version 3: A_star_v3 (Najbolja sa putanjom - Best with Path) ⭐ RECOMMENDED
```cpp
std::pair<float, std::vector<int>> A_star_v3(std::pair<int,int> start, std::pair<int,int> finish)
```

**Features:**
- Custom **DaryHeap<4>** (faster than std::priority_queue)
- Path reconstruction (backtracking)
- Returns: **{cost, path}** where path is vector of node indices

**Use Case:** **Primary version for Qt application** - has everything you need

**Performance:** ~35-40% faster than v1

---

### Version 4: A_star_v4 (Paralelno/SIMD - Parallel/Vectorized)
```cpp
std::pair<float, std::vector<int>> A_star_v4(std::pair<int,int> start, std::pair<int,int> finish)
```

**Features:**
- Everything from v3
- **AVX2 SIMD** vectorization (calculates 8 heuristics at once)
- Requires AVX2 CPU support

**Use Case:** Maximum performance on supported CPUs

**Performance:** ~40-45% faster than v1 (if AVX2 available)

⚠️ **Note:** Requires CPU with AVX2 support (Intel Haswell 2013+, AMD Excavator 2015+)

---

## 🎨 Qt Implementation: Algorithm Selector

### Step 1: Define Algorithm Enum

**astar.h:**
```cpp
#ifndef ASTAR_H
#define ASTAR_H

#include <vector>
#include <utility>
#include <string>

enum class AlgorithmVersion {
    SERIJSKI,              // v1 - Basic sequential
    NAJBOLJA_SEKVENCIJALNA, // v2 - Optimized sequential
    SA_PUTANJOM,           // v3 - With path (recommended)
    SIMD_PARALELNO         // v4 - SIMD vectorized
};

struct PathResult {
    bool found;
    float cost;
    std::vector<int> path;              // Node indices (finish → start)
    std::vector<bool> explored;         // Optional: which nodes were visited
    int nodes_explored;
    AlgorithmVersion version_used;

    // Helper to convert path indices to coordinates
    std::vector<std::pair<int,int>> getPathCoordinates(int cols) const {
        std::vector<std::pair<int,int>> coords;
        for(int idx : path) {
            coords.push_back({idx / cols, idx % cols});
        }
        return coords;
    }
};

// Helper function to get version name
inline std::string getVersionName(AlgorithmVersion version) {
    switch(version) {
        case AlgorithmVersion::SERIJSKI:
            return "Serijski (osnovni)";
        case AlgorithmVersion::NAJBOLJA_SEKVENCIJALNA:
            return "Najbolja sekvencijalna";
        case AlgorithmVersion::SA_PUTANJOM:
            return "Sa putanjom (preporučeno)";
        case AlgorithmVersion::SIMD_PARALELNO:
            return "SIMD paralelno";
        default:
            return "Nepoznato";
    }
}

class Graf {
    // ... existing members ...
public:
    // Existing methods
    float A_star(std::pair<int,int> start, std::pair<int,int> finish);
    float A_star_v2(std::pair<int,int> start, std::pair<int,int> finish);
    std::pair<float, std::vector<int>> A_star_v3(std::pair<int,int> start, std::pair<int,int> finish);
    std::pair<float, std::vector<int>> A_star_v4(std::pair<int,int> start, std::pair<int,int> finish);

    // Unified interface - calls appropriate version based on selection
    PathResult findPath(std::pair<int,int> start,
                       std::pair<int,int> finish,
                       AlgorithmVersion version = AlgorithmVersion::SA_PUTANJOM);
};

#endif
```

### Step 2: Implement Unified findPath Method

**astar.cpp:**
```cpp
#include "astar.h"
#include <stdexcept>

PathResult Graf::findPath(std::pair<int,int> start,
                         std::pair<int,int> finish,
                         AlgorithmVersion version) {

    PathResult result;
    result.version_used = version;
    result.nodes_explored = 0;

    try {
        switch(version) {
            case AlgorithmVersion::SERIJSKI: {
                // v1 - only returns cost
                float cost = A_star(start, finish);
                result.found = (cost >= 0);
                result.cost = cost;
                result.path = {};  // No path available
                break;
            }

            case AlgorithmVersion::NAJBOLJA_SEKVENCIJALNA: {
                // v2 - only returns cost
                float cost = A_star_v2(start, finish);
                result.found = (cost >= 0);
                result.cost = cost;
                result.path = {};  // No path available
                break;
            }

            case AlgorithmVersion::SA_PUTANJOM: {
                // v3 - returns {cost, path}
                auto [cost, path] = A_star_v3(start, finish);
                result.found = (cost >= 0);
                result.cost = cost;
                result.path = path;
                result.nodes_explored = path.size();  // Approximate
                break;
            }

            case AlgorithmVersion::SIMD_PARALELNO: {
                // v4 - returns {cost, path}
                auto [cost, path] = A_star_v4(start, finish);
                result.found = (cost >= 0);
                result.cost = cost;
                result.path = path;
                result.nodes_explored = path.size();  // Approximate
                break;
            }

            default:
                throw std::runtime_error("Unknown algorithm version");
        }
    }
    catch(const std::domain_error& e) {
        result.found = false;
        result.cost = -1.0f;
        result.path = {};
    }

    return result;
}
```

### Step 3: Qt UI - ComboBox for Algorithm Selection

**mainwindow.ui (or in code):**
```cpp
// In MainWindow constructor or setupUI()
ui->comboBoxAlgorithm->clear();
ui->comboBoxAlgorithm->addItem("Serijski (osnovni)",
    QVariant::fromValue(AlgorithmVersion::SERIJSKI));
ui->comboBoxAlgorithm->addItem("Najbolja sekvencijalna",
    QVariant::fromValue(AlgorithmVersion::NAJBOLJA_SEKVENCIJALNA));
ui->comboBoxAlgorithm->addItem("Sa putanjom (preporučeno) ⭐",
    QVariant::fromValue(AlgorithmVersion::SA_PUTANJOM));
ui->comboBoxAlgorithm->addItem("SIMD paralelno (zahteva AVX2)",
    QVariant::fromValue(AlgorithmVersion::SIMD_PARALELNO));

// Set default to v3
ui->comboBoxAlgorithm->setCurrentIndex(2);

// Add tooltips
ui->comboBoxAlgorithm->setItemData(0,
    "Osnovna implementacija (spora, samo cena)", Qt::ToolTipRole);
ui->comboBoxAlgorithm->setItemData(1,
    "Optimizovana verzija (brža, samo cena)", Qt::ToolTipRole);
ui->comboBoxAlgorithm->setItemData(2,
    "Najbolja opcija - vraća putanju za vizualizaciju", Qt::ToolTipRole);
ui->comboBoxAlgorithm->setItemData(3,
    "Najbrža verzija (zahteva AVX2 podršku)", Qt::ToolTipRole);
```

### Step 4: Call Algorithm Based on Selection

**mainwindow.cpp:**
```cpp
void MainWindow::onStartAlgorithm() {
    // Validate
    if(!hasStart || !hasFinish) {
        QMessageBox::warning(this, "Greška", "Molimo postavite početak i kraj!");
        return;
    }

    // Get selected algorithm version
    AlgorithmVersion selectedVersion = ui->comboBoxAlgorithm->currentData()
        .value<AlgorithmVersion>();

    // Prepare graph
    Graf graph(edgeWeights, rows, cols);

    // Disable UI during computation
    ui->btnStart->setEnabled(false);
    ui->comboBoxAlgorithm->setEnabled(false);
    setCursor(Qt::WaitCursor);

    // Run algorithm with timing
    auto t_start = std::chrono::high_resolution_clock::now();

    PathResult result = graph.findPath(startPos, finishPos, selectedVersion);

    auto t_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        t_end - t_start
    );
    result.execution_time = duration.count();

    // Re-enable UI
    ui->btnStart->setEnabled(true);
    ui->comboBoxAlgorithm->setEnabled(true);
    setCursor(Qt::ArrowCursor);

    // Display results
    if(result.found) {
        QString versionName = QString::fromStdString(
            getVersionName(result.version_used)
        );

        ui->labelCost->setText(QString("Cena: %1").arg(result.cost, 0, 'f', 2));
        ui->labelTime->setText(QString("Vreme: %1 ms").arg(duration.count()));
        ui->labelAlgorithm->setText(QString("Algoritam: %1").arg(versionName));

        // Visualize path (only available for v3 and v4)
        if(!result.path.empty()) {
            gridWidget->setPath(result.path, cols);
            gridWidget->update();

            QMessageBox::information(this, "Uspeh",
                QString("Put pronađen!\n\n"
                        "Cena: %1\n"
                        "Vreme: %2 ms\n"
                        "Algoritam: %3\n"
                        "Dužina puta: %4 čvorova")
                    .arg(result.cost, 0, 'f', 2)
                    .arg(duration.count())
                    .arg(versionName)
                    .arg(result.path.size()));
        } else {
            // v1 or v2 - no path available
            QMessageBox::information(this, "Uspeh",
                QString("Put pronađen!\n\n"
                        "Cena: %1\n"
                        "Vreme: %2 ms\n"
                        "Algoritam: %3\n\n"
                        "Napomena: Ova verzija ne vraća putanju.\n"
                        "Koristite 'Sa putanjom' za vizualizaciju.")
                    .arg(result.cost, 0, 'f', 2)
                    .arg(duration.count())
                    .arg(versionName));
        }
    } else {
        QMessageBox::warning(this, "Greška",
            QString("Put nije pronađen!\n\n"
                    "Algoritam: %1\n"
                    "Vreme: %2 ms")
                .arg(QString::fromStdString(getVersionName(result.version_used)))
                .arg(duration.count()));
    }
}
```

---

## 🎮 Alternative: Radio Buttons

If you prefer radio buttons instead of dropdown:

**mainwindow.ui:**
```cpp
QGroupBox* algorithmGroup = new QGroupBox("Verzija algoritma");
QVBoxLayout* algorithmLayout = new QVBoxLayout();

radioSerijski = new QRadioButton("Serijski (osnovni)");
radioBestSeq = new QRadioButton("Najbolja sekvencijalna");
radioWithPath = new QRadioButton("Sa putanjom (preporučeno)");
radioSIMD = new QRadioButton("SIMD paralelno");

radioWithPath->setChecked(true);  // Default

algorithmLayout->addWidget(radioSerijski);
algorithmLayout->addWidget(radioBestSeq);
algorithmLayout->addWidget(radioWithPath);
algorithmLayout->addWidget(radioSIMD);

algorithmGroup->setLayout(algorithmLayout);
```

**Get selection:**
```cpp
AlgorithmVersion MainWindow::getSelectedVersion() {
    if(radioSerijski->isChecked())
        return AlgorithmVersion::SERIJSKI;
    else if(radioBestSeq->isChecked())
        return AlgorithmVersion::NAJBOLJA_SEKVENCIJALNA;
    else if(radioWithPath->isChecked())
        return AlgorithmVersion::SA_PUTANJOM;
    else if(radioSIMD->isChecked())
        return AlgorithmVersion::SIMD_PARALELNO;
    else
        return AlgorithmVersion::SA_PUTANJOM;  // Default
}

void MainWindow::onStartAlgorithm() {
    AlgorithmVersion version = getSelectedVersion();
    PathResult result = graph.findPath(startPos, finishPos, version);
    // ... process result ...
}
```

---

## 📊 Version Comparison Feature (Optional)

Allow users to compare all versions side-by-side:

```cpp
void MainWindow::onCompareAllVersions() {
    if(!hasStart || !hasFinish) {
        QMessageBox::warning(this, "Greška", "Postavite početak i kraj!");
        return;
    }

    Graf graph(edgeWeights, rows, cols);

    struct VersionResult {
        QString name;
        float cost;
        float time;
        bool hasPath;
    };

    std::vector<VersionResult> results;

    // Test all versions
    for(int v = 0; v < 4; v++) {
        AlgorithmVersion version = static_cast<AlgorithmVersion>(v);

        auto t_start = std::chrono::high_resolution_clock::now();
        PathResult result = graph.findPath(startPos, finishPos, version);
        auto t_end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            t_end - t_start
        );

        results.push_back({
            QString::fromStdString(getVersionName(version)),
            result.cost,
            static_cast<float>(duration.count()),
            !result.path.empty()
        });
    }

    // Display comparison table
    QString message = "Poređenje verzija algoritma:\n\n";
    message += QString("%-30s %10s %10s %8s\n")
        .arg("Verzija").arg("Cena").arg("Vreme(ms)").arg("Putanja");
    message += QString("-").repeated(65) + "\n";

    for(const auto& r : results) {
        message += QString("%-30s %10.2f %10.1f %8s\n")
            .arg(r.name)
            .arg(r.cost)
            .arg(r.time)
            .arg(r.hasPath ? "Da" : "Ne");
    }

    // Calculate speedups
    float baseTime = results[0].time;
    message += "\n" + QString("-").repeated(65) + "\n";
    message += QString("Ubrzanje u odnosu na osnovnu verziju:\n");
    for(size_t i = 1; i < results.size(); i++) {
        float speedup = baseTime / results[i].time;
        message += QString("  %1: %.2fx brže\n").arg(results[i].name).arg(speedup);
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Poređenje algoritama");
    msgBox.setText(message);
    msgBox.setFont(QFont("Courier", 10));  // Monospace for alignment
    msgBox.exec();
}
```

---

## 🎯 Recommended Setup for Your Qt App

### For Development/Testing:
```cpp
// Default to v3 (best balance of features + performance)
AlgorithmVersion defaultVersion = AlgorithmVersion::SA_PUTANJOM;
```

### UI Layout:
```
┌─────────────────────────┐
│  Algorithm Settings     │
├─────────────────────────┤
│  Version: [Dropdown ▼] │
│    ┌─────────────────┐  │
│    │ Serijski         │  │
│    │ Najbolja sekv.   │  │
│    │ Sa putanjom ⭐   │  │ ← Selected
│    │ SIMD paralelno   │  │
│    └─────────────────┘  │
│                         │
│  [Compare All Versions] │ ← Optional button
└─────────────────────────┘
```

---

## ⚙️ Quick Reference: When to Use Which Version

| Scenario | Recommended Version | Why |
|----------|-------------------|-----|
| **Development/Testing** | v3 (Sa putanjom) | Has path for visualization |
| **Visualization** | v3 or v4 | Only these return path |
| **Maximum Performance** | v4 (if AVX2 available) | SIMD optimization |
| **Compatibility** | v3 | Works on all CPUs |
| **Cost Only** | v2 | Fastest without path |
| **Debugging** | v1 | Simplest implementation |

---

## 🚀 Summary

**Simple Selection:**
```cpp
// In your Qt application
AlgorithmVersion version = AlgorithmVersion::SA_PUTANJOM;  // ⭐ Best choice
PathResult result = graph.findPath(startPos, finishPos, version);

if(result.found) {
    // Use result.cost and result.path
    visualizePath(result.path);
}
```

**User Selection:**
```cpp
// Get from UI
AlgorithmVersion version = ui->comboBoxAlgorithm->currentData()
    .value<AlgorithmVersion>();

PathResult result = graph.findPath(startPos, finishPos, version);
```

**My Recommendation:**
- **Default to v3** - It has everything you need (path, good performance, wide compatibility)
- **Offer v4 as option** - For users with newer CPUs who want max speed
- **Keep v1/v2** - For educational comparison and testing

The unified `findPath()` interface makes it easy to switch between versions without changing your Qt code! 🎉
