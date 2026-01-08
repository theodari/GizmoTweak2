# Architecture GizmoTweak2

## Vue d'ensemble

GizmoTweak2 est une refactorisation node-based de GizmoTweak. L'application permet de composer des effets laser via un graphe de nodes connectés.

### Principes fondamentaux

- **Architecture node-based** : graphe dirigé de nodes connectés
- **Flux séquentiel** : les Frames traversent les Tweaks dans l'ordre défini par le câblage
- **Séparation Shapes/Tweaks** : les Shapes produisent des ratios, les Tweaks appliquent des effets
- **Fan-out** : une sortie Shape peut alimenter plusieurs Tweaks ou Groups

---

## Catalogue des Nodes

### I/O

| Node | Description | Entrées | Sorties |
|------|-------------|---------|---------|
| **Input** | Point d'entrée des Frames (ILDA, pattern) | - | Frame |
| **Output** | Point de sortie final | Frame | - |

### Shapes (producteurs de ratios)

| Node | Description | Entrées | Sorties |
|------|-------------|---------|---------|
| **Gizmo** | Zone d'influence 2D paramétrique | - | Ratio 2D |
| **Group** | Combine plusieurs Shapes avec modes de composition | Ratio(s) 2D | Ratio 2D |
| **SurfaceFactory** | Surface paramétrique (remplace GizmoLine) | - | Ratio 1D |

### Utility

| Node | Description | Entrées | Sorties |
|------|-------------|---------|---------|
| **TimeShift** | Décale le temps d'application (retard/avance) | Ratio | Ratio (décalé) |

### Tweaks (effets)

| Node | Description | Entrées | Sorties |
|------|-------------|---------|---------|
| **PositionTweak** | Offset X/Y, position initiale/finale | Frame + Ratio | Frame |
| **ScaleTweak** | Échelle horizontale/verticale | Frame + Ratio | Frame |
| **RotationTweak** | Rotation | Frame + Ratio | Frame |
| **MoebiusTweak** | Transformation conforme f(z)=(az+b)/(cz+d) | Frame + Ratio | Frame |
| **RounderTweak** | Distorsion cylindrique | Frame + Ratio | Frame |
| **PolarTweak** | Transformation coordonnées polaires | Frame + Ratio | Frame |
| **ColorTweak** | Effets couleur (Filter, Color, Sparkle) | Frame + Ratio | Frame |

---

## Types de données

### Frame
Collection de samples laser. Chaque sample contient :
- Position (x, y) dans [-1, +1]
- Couleur (r, g, b)

### Ratio 2D
Valeur [0, 1] calculée à partir d'une position (x, y).
Produit par : Gizmo, Group

### Ratio 1D
Valeur [0, 1] calculée à partir de t = index/totalSamples.
Produit par : SurfaceFactory

---

## Règles de connexion

### Compatibilités

```
Source              →  Destination         Valide?
─────────────────────────────────────────────────────
Gizmo (Ratio 2D)    →  Group               ✓
Gizmo (Ratio 2D)    →  Tweak               ✓
Group (Ratio 2D)    →  Group               ✓
Group (Ratio 2D)    →  Tweak               ✓
SurfaceFactory (1D) →  Tweak               ✓
TimeShift           →  Tweak               ✓
TimeShift           →  Group               ✓

Tweak (Frame)       →  Tweak               ✓ (chaînage)
Tweak (Frame)       →  Output              ✓
Input (Frame)       →  Tweak               ✓

Tweak               →  Gizmo               ✗
Tweak               →  Group               ✗
```

### Fan-out (sorties multiples)

Une Shape peut être connectée à plusieurs destinations :

```
         ┌──────────────▶ PositionTweak
         │
Group ───┼──────────────▶ ScaleTweak
         │
         └──────────────▶ RotationTweak
```

### Mode de ratio des Tweaks

Chaque Tweak possède un commutateur pour le mode de ratio :

```cpp
enum class RatioMode
{
    Gizmo2D,    // Ratio calculé avec f(x, y) depuis Gizmo/Group
    Surface1D   // Ratio calculé avec f(t) depuis SurfaceFactory
};
```

---

## Flux de données

### Graphe typique

```
┌───────┐
│ INPUT │
└───┬───┘
    │ Frame
    ▼
┌──────────────┐      ┌─────────┐
│PositionTweak │◀─────│  Group  │◀─── Gizmo
└──────┬───────┘      └─────────┘◀─── Gizmo
       │ Frame              ▲
       ▼                    │
┌──────────────┐      ┌─────┴─────┐
│  ScaleTweak  │◀─────│ TimeShift │
└──────┬───────┘      └─────▲─────┘
       │ Frame              │
       ▼               ┌────┴────┐
┌──────────────┐       │  Group  │◀─── SurfaceFactory
│  ColorTweak  │◀──────└─────────┘
└──────┬───────┘
       │ Frame
       ▼
┌────────┐
│ OUTPUT │
└────────┘
```

### Ordre d'exécution

L'ordre des Tweaks est déterminé par le **câblage Frame → Frame**, pas par leur position visuelle dans l'interface.

1. Le graphe est parcouru depuis Input vers Output
2. Pour chaque Tweak rencontré (dans l'ordre topologique du câblage Frame) :
   - Remonter à contre-courant la chaîne Shape connectée
   - Calculer le ratio pour chaque sample
   - Appliquer l'effet

---

## Group : modes de composition

Le Group combine plusieurs Shapes selon un mode paramétrable :

| Mode | Formule | Description |
|------|---------|-------------|
| NORMAL | a | Prend le premier |
| MAX | max(a, b, ...) | Maximum |
| MIN | min(a, b, ...) | Minimum |
| SUM | a + b + ... | Somme (clampée) |
| PRODUCT | a × b × ... | Multiplication |
| AVERAGE | (a + b + ...) / n | Moyenne |
| ABSDIF | \|a - b\| | Différence absolue |

Le Group peut également appliquer des **distorsions géométriques** sur les coordonnées avant le calcul du ratio.

Tous les paramètres du Group supportent l'**automation**.

---

## TimeShift : décalage temporel

Le TimeShift décale le temps utilisé pour l'automation des Shapes en amont.

### Fonctionnement

- Parcouru à contre-courant (sortie → entrée)
- Pour un délai positif (retard) : soustrait le temps
- Pour un délai négatif (avance) : ajoute le temps

```
temps_effectif = temps_actuel - délai
```

### Exemple

```
Avec délai = +0.5s et temps_actuel = 1.0s :
→ Le Gizmo connecté via TimeShift est évalué à t = 0.5s
```

---

## SurfaceFactory : ratios 1D

SurfaceFactory produit un ratio basé sur la position du sample dans la frame :

```cpp
t = index / totalSamples;  // t ∈ [0, 1]
ratio = surfaceFactory.evaluate(t);
```

Cela remplace l'ancien système GizmoLine/LineTweak.

---

## Modules exclus

Les modules suivants de GizmoTweak v3.x ne font **pas** partie de GizmoTweak2 :

| Module | Raison |
|--------|--------|
| LineTweak | Remplacé par SurfaceFactory |
| GizmoLine | Remplacé par SurfaceFactory |
| LineGroup | Remplacé par SurfaceFactory |
| MaskTweak | Intégré aux moteurs (Excalibur/Ikkonix) |
| Collider | Intégré à Paint Alchemy |

---

## Classes principales (à implémenter)

### Core

```cpp
namespace gizmotweak2
{
    class Node;           // Base abstraite pour tous les nodes
    class Connection;     // Lien entre deux ports
    class NodeGraph;      // Conteneur du graphe complet
    class Port;           // Point de connexion (entrée/sortie)
}
```

### Nodes concrets

```cpp
// I/O
class InputNode;
class OutputNode;

// Shapes
class GizmoNode;
class GroupNode;
class SurfaceFactoryNode;

// Utility
class TimeShiftNode;

// Tweaks
class PositionTweak;
class ScaleTweak;
class RotationTweak;
class MoebiusTweak;
class RounderTweak;
class PolarTweak;
class ColorTweak;
```

---

## Versions

| Version | Contenu |
|---------|---------|
| 0.1.0 | Infrastructure (lib + app + tests) |
| 0.2.0 | Classes de base (Node, Connection, NodeGraph) |
| 0.3.0 | Nodes I/O (Input, Output) |
| 0.4.0 | Shapes (Gizmo, Group) |
| 0.5.0 | Tweaks de base |
| ... | ... |
