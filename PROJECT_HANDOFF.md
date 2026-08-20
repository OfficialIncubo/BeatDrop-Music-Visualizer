# Project BeatDrop Handoff

## Experimental build

This branch contains the experimental x64 preset-loader and lyrics-module work for BeatDrop Music Visualizer.

Repository: `OfficialIncubo/BeatDrop-Music-Visualizer`

Build target: `Release_x64`

## Preset loading changes

- Preset discovery now walks the entire preset tree recursively, including nested subfolders.
- Preset and state-file access uses Windows long-path prefixes where needed.
- Preset enumeration no longer depends on a single `MAX_PATH`-sized path buffer.
- The original preset ordering behavior remains unchanged; sequential ordering is not forced on.
- The production diagnostic scan logging was removed after verification.

These changes address both nested preset packs and preset paths/file names longer than the traditional Windows path limit.

## Lyrics module changes

BeatDrop now reads Windows Global System Media Transport Controls (SMTC) metadata and timeline information and includes a native lyrics editor based on the Lrcdrop implementation.

### Metadata and lookup

- Reads artist, title, album, duration, playback state, and position from the current Windows media session.
- Normalizes common YouTube metadata forms such as `Artist - Topic`, `ArtistVEVO`, repeated artist prefixes, and `Official Audio` suffixes.
- Looks up synchronized lyrics through LRCLIB.
- Uses WinHTTP for the LRCLIB request path, with the existing Windows Runtime path retained as fallback.
- Caches each song under `%LOCALAPPDATA%\\BeatDrop\\Lyrics`.
- Loads the local cache before making a network request.
- Saves successful network results to the local cache.
- Extrapolates playback position while a browser reports `Playing` but does not continuously refresh its SMTC timeline position.
- Clears/restarts the displayed lyric when playback rewinds.

### Lyrics display and editor

- Displays the active timed lyric line in the visualizer.
- Keeps each active lyric line visible for 10 seconds before it melts into the visualization.
- `Ctrl+Shift+E` opens the native editor.
- The F1 help screen documents `Ctrl+Shift+E` for opening the lyrics editor.
- The editor has Input and Parsed Text panes with the Lrcdrop graphite theme.
- Timestamp formatting uses RichEdit range formatting: bold orange timestamps and light-gray lyric text.
- Includes Parse text, Import LRC, Delete line, Insert silence, Reset editor, Undo, Redo, Seek to line, line movement, capture timestamp, capture-and-advance, local save, and close controls.
- Delete removes the selected whole line; the Delete key removes only the selected line timestamp, matching Lrcdrop behavior.
- Capture uses the current SMTC playback position and replaces or inserts the selected line timestamp.
- Seek controls use the active SMTC session and support +/-5, +/-10, and +/-30 seconds.
- Seek to line uses the timestamp from the highlighted parsed line.
- Undo and redo preserve parsed-text edits through BeatDrop’s editor history.
- Save writes metadata-complete LRC headers for duration, artist, album, title, producer, version, and offset.
- The editor includes LRCLIB upload with proof-of-work challenge solving and metadata payload submission.

## Build prerequisites

The machine has the legacy DirectX SDK installed at:

`C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)`

The x64 build must provide that SDK path to MSBuild because `DXSDK_DIR` is not set automatically:

```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe" `
  "vis_milk2\plugin.vcxproj" /t:Build /p:Configuration=Release_x64 /p:Platform=x64 `
  /p:DXSDK_DIR="C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)" `
  /p:PostBuildEventUseInBuild=false /m
```

The resulting executable is `vis_milk2\Release_x64\BeatDrop_x64.exe`. Deployment copies it to `BeatDrop\BeatDrop_x64.exe`.

## Verification performed

- Clean x64 builds succeeded after restoring the DirectX SDK path and fixing the current Windows SDK JPEG type conflict.
- Recursive preset loading was tested against nested preset folders.
- LRCLIB lookup was verified with Tool / The Pot and the synchronized result was cached locally.
- SMTC position tracing verified advancing playback positions while a browser reported Playing.
- The experimental executable was launched after the final x64 build.

## Notes for the next developer

- Do not replace the Lrcdrop-derived editor handlers with simplified status-only implementations.
- When changing metadata normalization, inspect the raw SMTC trace at `%TEMP%\\BeatDropSMTC.log` and the lyrics trace at `%TEMP%\\BeatDropLyrics.log`.
- Keep the editor’s RichEdit range formatting and selection behavior aligned with `Project Lrcdrop\\vendor\\aimp_projectM\\sources\\LyricsOverlay.cpp`.
