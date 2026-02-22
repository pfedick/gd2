# Binäres Level-Format (PFMagic)

Dieses Dokument beschreibt das binäre Speicherformat der Level-Dateien für **gd2**. Das Format ist chunk-basiert und wird mit dem Header `PFMagic` eingeleitet.

## Grundstruktur

Eine Level-Datei besteht aus einem Haupt-Header, gefolgt von einer Sequenz von Daten-Chunks.

| Offset | Größe | Beschreibung |
| :--- | :--- | :--- |
| 0 | 7 Byte | Dateisignatur: `PFMagic` |
| 7 | - | Start des ersten Chunks |

### Aufbau eines Chunks

Jeder Chunk hat einen standardisierten Header von mindestens 5 Byte, gefolgt von weiteren Header-Informationen (abhängig von der ID) und den eigentlichen Nutzdaten.

| Offset | Größe | Beschreibung |
| :--- | :--- | :--- |
| 0 | 4 Byte | Gesamte Chunk-Größe (inkl. Header) |
| 4 | 1 Byte | Chunk-ID (siehe [Chunk-IDs](#chunk-ids)) |
| 5 | - | Chunk-spezifische Daten (beginnen oft mit einer Version) |

---

## Chunk-IDs

Die folgenden IDs sind in `Level::ChunkId` definiert:

| ID | Name | Beschreibung |
| :- | :--- | :----------- |
| 1 | `Tiles` | Kachel-Gitter (Tilemap) einer Parallax-Ebene |
| 2 | `TileTypes` | Logische Kachel-Typen (Kollisionen, etc.) |
| 3 | `Sprites` | Frei platzierbare Sprites |
| 4 | `Objects` | Interaktive Objekte und Trigger |
| 5 | `WayNet` | Navigations-Netzwerk (aktuell ungenutzt) |
| 6 | `LevelParameter` | Allgemeine Level-Einstellungen (muss als erster Chunk kommen) |
| 7 | `ColorPalette` | Die globale Farbtabelle des Levels |
| 8 | `Lights` | Lichtquellen (geplant) |

---

## Detaillierte Chunk-Beschreibungen

### 1. LevelParameter (ID 6)

Dieser Chunk enthält allgemeine Informationen wie Größe, Autor und Start-Musik. Er verwendet das binäre Format der `ppl7::AssocArray`-Klasse.

**Chunk-Header (spezifisch):**
- Offset 5: Version (1 Byte) - aktuell `1`

**Nutzdaten:**
Die restlichen Daten des Chunks enthalten ein serialisiertes `ppl7::AssocArray`. Wichtige Schlüssel sind:
- `level_width`, `level_height`: Logische Größe des Levels in Kacheln.
- `initial_song`: Pfad zur Musikdatei.
- `BackgroundImage`: Hintergrundgrafik.
- Informationen zur Story, Beschreibung und Autor.

### 2. ColorPalette (ID 7)

Hier wird die Farbtabelle gespeichert, die für die Farbmodulation von Tiles und Sprites verwendet wird.

**Chunk-Header (spezifisch):**
- Offset 5: Version (1 Byte) - aktuell `1`

**Nutzdaten - Einträge:**
Die Nutzdaten bestehen aus einer sequenziellen Liste von Farb-Einträgen. Das Ende wird durch einen Eintrag mit der Größe `0` markiert.

| Größe | Beschreibung |
| :--- | :----------- |
| 2 Byte | Größe des Eintrags (inkl. dieser 2 Byte) |
| 2 Byte | Farb-Index (0-255) |
| 2 Byte | LDraw-Material-Referenz (0 wenn ungenutzt) |
| 1 Byte | Rot |
| 1 Byte | Grün |
| 1 Byte | Blau |
| 1 Byte | Alpha |
| n Byte | Name der Farbe (UTF-8, Null-terminiert) |

### 3. Tiles (ID 1)

Dieser Chunk speichert die visuellen Kacheln einer Ebene.

**Chunk-Header (spezifisch):**
- Offset 5: Parallax-Ebenen-ID (0-9)
- Offset 6: Version (1 Byte) - aktuell `2`
- Offset 7: Breite der Ebene (2 Byte)
- Offset 9: Höhe der Ebene (2 Byte)
- Offset 11: Max Layers pro Kachel (1 Byte) - meist `4`

**Nutzdaten:**
Es werden nur Kacheln gespeichert, die nicht leer sind.
Jeder Eintrag besteht aus:
- 2 Byte: X-Koordinate
- 2 Byte: Y-Koordinate
- 1 Byte: Flag `block_background`
- Pro Layer (meist 4):
  - 2 Byte Tileset-ID
  - 2 Byte Kachel-Nummer
  - 2 Byte Origin X
  - 2 Byte Origin Y
  - 1 Byte Occupation-Status
  - 1 Byte ShowStuds-Flag
  - 1 Byte Color-Index

### 4. Sprites (ID 3)

Frei platzierte Sprites auf einer Ebene.

**Chunk-Header (spezifisch):**
- Offset 5: Parallax-Ebenen-ID
- Offset 6: Position (1 Byte: `0` = Vordergrund, `1` = Hintergrund)
- Offset 7: Version (1 Byte) - aktuell `1`

**Nutzdaten - Sprites (22 Byte pro Sprite):**
- 4 Byte: X-Position (Integer)
- 4 Byte: Y-Position (Integer)
- 1 Byte: Z-Ebene (Layering innerhalb des Spritesystems)
- 1 Byte: Color-Index
- 2 Byte: Spriteset-ID
- 2 Byte: Sprite-Nummer
- 4 Byte: Skalierung (Float)
- 4 Byte: Rotation in Grad (Float)

### 5. Objects (ID 4)

Interaktive Objekte (Gegner, Schalter, Startpunkte, etc.).

**Chunk-Header (spezifisch):**
- Offset 5: Parallax-Ebenen-ID
- Offset 6: Version (1 Byte) - aktuell `1`

**Nutzdaten:**
Enthält eine Liste von Objekten. Jeder Eintrag beginnt mit seiner Größe:
- 4 Byte: Gesamtgröße des Objekt-Eintrags (inkl. dieser 4 Byte)
- Nutzdaten des Objekts (Header + spezifische Daten)

#### Gemeinsamer Objekt-Header (17 Byte)

Jedes Objekt beginnt mit diesen Feldern:

| Offset | Größe | Beschreibung |
| :--- | :--- | :--- |
| 0 | 1 Byte | Objekt-Header-Version (aktuell `1`) |
| 1 | 2 Byte | Objekt-Typ (ID aus `Objects::Type`) |
| 3 | 1 Byte | Render-Ebene (Hinter/Vor Bricks, Hinter/Vor Spieler) |
| 4 | 4 Byte | Eindeutige Objekt-ID |
| 8 | 4 Byte | Initiale X-Position (Logical 4K Space als Integer) |
| 12 | 4 Byte | Initiale Y-Position (Logical 4K Space als Integer) |
| 16 | 1 Byte | Schwierigkeits-Matrix (Flags für Easy/Normal/Hard) |

Nach dem Header folgen typspezifische Daten des Objekts. Viele Objekte beginnen diese mit einem eigenen Versions-Byte.

---

## Parallax-Ebenen-IDs

Die IDs der Ebenen (für Offset 5 in vielen Chunks) entsprechen dem `enum ParallaxLayerId`:

| ID | Name |
| :- | :--- |
| 0 | Near |
| 1 | Close |
| 2 | Front |
| 3 | Player |
| 4 | Back |
| 5 | Behind |
| 6 | Middle |
| 7 | Far |
| 8 | Horizon |
| 9 | Sky |
