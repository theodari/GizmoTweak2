pragma Singleton
import QtQuick

QtObject {
    // Fonds
    readonly property color background: "#222222"
    readonly property color backgroundLight: "#2A2A2A"
    readonly property color surface: "#303030"
    readonly property color surfaceHover: "#404040"
    readonly property color surfacePressed: "#505050"
    readonly property color menuHighlight: "#4060A0"  // Accent color for menu selection

    // Accents
    readonly property color accent: "#4060A0"
    readonly property color accentLight: "#3060B0"
    readonly property color accentBright: "#2060FF"
    readonly property color selection: "#4444FF"

    // Texte
    readonly property color text: "#FFFFFF"
    readonly property color textMuted: "#666666"
    readonly property color textHighlight: "#00FFFF"

    // Bordures
    readonly property color border: "#555555"
    readonly property color borderLight: "#444444"

    // Nodes par catégorie
    readonly property color nodeIO: "#4060A0"
    readonly property color nodeShape: "#40A060"      // Gizmo, SurfaceFactory
    readonly property color nodeGroup: "#308050"      // Group (distinct green)
    readonly property color nodeUtility: "#606060"
    readonly property color nodeTweak: "#A06040"

    // Ports par type de données
    readonly property color portFrame: "#64B4FF"
    readonly property color portRatio2D: "#64FF96"
    readonly property color portRatio1D: "#FFA064"

    // Câbles
    readonly property color cableFrame: "#64B4FF"
    readonly property color cableRatio2D: "#64FF96"
    readonly property color cableRatio1D: "#FFA064"
    readonly property color cablePreview: "#FFFFFF"

    // Grille du canvas
    readonly property color gridLine: "#333333"
    readonly property color gridLineMajor: "#444444"

    // Dimensions
    readonly property int nodeRadius: 6
    readonly property int portRadius: 6
    readonly property int nodePadding: 12
    readonly property int nodeMinWidth: 120
    readonly property int cableWidth: 2

    // Fonts
    readonly property int fontSizeSmall: 10
    readonly property int fontSizeNormal: 12
    readonly property int fontSizeLarge: 14
}
