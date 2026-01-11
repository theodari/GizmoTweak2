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
    readonly property color textOnHighlight: "#000000"  // Black text on light highlight backgrounds

    // Properties panel - lighter colors
    readonly property color propLabel: "#AAAAAA"        // Lighter labels
    readonly property color propGroupTitle: "#BBBBBB"   // Lighter group titles
    readonly property color propGroupBorder: "#707070"  // Lighter group borders

    // Bordures
    readonly property color border: "#555555"
    readonly property color borderLight: "#444444"

    // Nodes par catégorie
    readonly property color nodeIO: "#A0A0E0"         // Pastel lavender for Input/Output (close to Tweaks)
    readonly property color nodeGizmo: "#80E0E0"      // Pastel cyan for Gizmo
    readonly property color nodeTransform: "#E080E0"   // Pastel magenta for Transform
    readonly property color nodeSurface: "#80E0E0"    // Pastel cyan for SurfaceFactory (same as Gizmo)
    readonly property color nodeUtility: "#606060"
    readonly property color nodeTweak: "#8080E0"      // Pastel blue for Tweaks

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

    // Splitter handle
    readonly property color splitterHandle: "#606060"
    readonly property color splitterHandleHover: "#808080"
    readonly property color splitterHandlePressed: "#4060A0"

    // Preview panel
    readonly property color previewBackground: "#000000"
    readonly property color previewGrid: "#333333"
    readonly property color previewPointSource: "#666666"
    readonly property color previewPointResult: "#FFFFFF"

    // Status colors
    readonly property color error: "#FF4444"
    readonly property color warning: "#FFB020"
    readonly property color success: "#40C040"

    // Dimensions
    readonly property int nodeRadius: 6
    readonly property int portRadius: 6
    readonly property int nodePadding: 12
    readonly property int nodeMinWidth: 120
    readonly property int cableWidth: 2
    readonly property int toolbarButtonSize: 36  // Square buttons in toolbar
    readonly property int splitterHandleWidth: 4

    // Fonts
    readonly property int fontSizeSmall: 10
    readonly property int fontSizeNormal: 12
    readonly property int fontSizeLarge: 14

    // Properties panel fonts (larger)
    readonly property int propFontSize: 13           // Main text in properties
    readonly property int propFontSizeTitle: 14      // Group titles
    readonly property int propSpinBoxHeight: 28      // SpinBox height
}
