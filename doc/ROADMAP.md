# Roadmap GizmoTweak2

## Historique des versions

| Version | Contenu | Statut |
|---------|---------|--------|
| 0.1.0 | Infrastructure (lib + app + tests) | ✅ Terminé |
| 0.2.0 | Classes de base (Node, Connection, NodeGraph) | ✅ Terminé |
| 0.3.0 | Nodes I/O (Input, Output) | ✅ Terminé |
| 0.4.0 | Shapes (Gizmo, Transform, SurfaceFactory) | ✅ Terminé |
| 0.5.0 | Tweaks de base (Position, Scale, Rotation) | ✅ Terminé |
| 0.6.0 | Tweaks avancés (Color, Wave, Squeeze, etc.) | ✅ Terminé |
| 0.7.0 | Automation (keyframes, timeline) | ✅ Terminé |
| 0.8.0 | Undo/Redo, persistence JSON, clipboard | ✅ Terminé |
| 0.9.0 | Intégration Excalibur, preview temps réel | ✅ Terminé |

---

## Version actuelle: 0.9.x

### Fonctionnalités implémentées

**Core**
- [x] Node, Port, Connection, NodeGraph
- [x] GraphEvaluator avec évaluation en chaîne
- [x] Commands pattern (Undo/Redo)
- [x] Persistence JSON (.gt2)
- [x] Clipboard (Copy/Cut/Paste/Duplicate)

**Nodes (17 types)**
- [x] I/O: Input, Output
- [x] Shapes: Gizmo, Transform (ex-Group), SurfaceFactory
- [x] Utility: TimeShift, Mirror
- [x] Tweaks: Position, Scale, Rotation, Polar, Wave, Squeeze, Split, Rounder, Sparkle, Color, Fuzzyness, ColorFuzzyness

**Automation**
- [x] Param avec valeurs animables
- [x] KeyFrame avec courbes d'interpolation
- [x] AutomationTrack par paramètre
- [x] Timeline panel avec édition visuelle

**Interface QML**
- [x] NodeCanvas avec zoom/pan
- [x] Drag & drop création de nodes
- [x] Connexions Bézier
- [x] Properties panel (drawer)
- [x] Preview panel avec playback
- [x] Theme dark style SurfaceFactory
- [x] Recent files, window state persistence

**Laser**
- [x] ExcaliburEngine (connexion, zones, envoi frames)
- [x] Activation/désactivation par zone

---

## Prochaines versions

### v0.10.0 - Stabilisation

**Priorité haute**
- [x] Tests de couverture complète (objectif: 80%) - Ajout tst_automation.cpp
- [x] Validation robuste des graphes (cycles, connexions orphelines)
- [x] Gestion des erreurs utilisateur (Toast notifications)
- [x] Stabilité de l'intégration Excalibur (reconnexion auto, status, error handling)

**Priorité moyenne**
- [x] Import patterns ILDA (.ild) dans InputNode
- [x] Export frames ILDA
- [ ] Documentation utilisateur (guide de démarrage)

---

### v0.11.0 - Productivité

- [x] Templates de graphes prédéfinis (5 templates dans File > New from Template)
- [x] Favoris de nodes fréquemment utilisés (étoile dorée sur les 4 plus utilisés)
- [x] Préréglages (presets) par type de tweak (section Presets dans panneau propriétés)
- [x] Copier/coller de keyframes entre tracks (clic droit sur keyframe)
- [x] Snap magnétique des keyframes sur les beats (maintenir Shift)
- [x] Raccourcis clavier (F2 pour afficher la liste, inspirés de Paint Alchemy)

---

### v0.12.0 - Audio sync

- [ ] Import fichier audio (WAV, MP3)
- [ ] Affichage waveform dans timeline
- [ ] Beat detection automatique
- [ ] Sync automation sur les temps forts
- [ ] Preview audio pendant playback

---

### v0.13.0 - Multi-engine

- [ ] Architecture plugin pour engines laser
- [ ] Support Ikkonix
- [ ] Support EasyLase
- [ ] Configuration multi-zone avancée
- [ ] Mapping zones → outputs

---

### v0.14.0 - Performance

- [ ] Évaluation multi-thread du graphe
- [ ] Cache de frames évalués
- [ ] Preview à résolution réduite
- [ ] Profiling et optimisation des tweaks lourds
- [ ] Benchmark automatisé

---

### v1.0.0 - Release stable (à définir)

Version majeure à déclencher manuellement quand le projet sera considéré comme stable et complet pour une release publique.

---

## Backlog (non priorisé)

- SurfaceFactory: modes supplémentaires (sine, saw, triangle, custom)
- GroupNode: sous-graphes encapsulés
- Historique des versions de fichiers
- Backup automatique
- Drag & drop de fichiers .gt2 pour ouvrir
- Export vidéo de la preview
- Mode "compact" pour les nodes (icônes seulement)
- Annotations/commentaires sur le canvas
- Alignement et distribution de nodes
- Guide de migration depuis GizmoTweak v3.x

---

## Notes techniques

### Dépendances futures possibles
- FFmpeg pour audio/vidéo
- PortAudio ou Qt Multimedia pour playback audio
- RTMIDI pour contrôle MIDI
- liblo pour OSC

### Tests manquants
- tst_automation (keyframes, interpolation)
- tst_excalibur (mock de l'API laser)
- tst_qml (tests d'interface avec Qt Quick Test)
