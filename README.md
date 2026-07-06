# FFGui

FFGui is a powerful, professional C++ Qt6-based GUI frontend for FFmpeg, designed to match the comprehensive multi-track editing layout of Kdenlive. It provides real-time previews, custom timeline dragging/splitting/trimming, and dynamic filter-complex generation for high-performance exports.

## Features

### Multi-Track Timeline
- **4-track layout** matching Kdenlive: Video 2, Video 1, Audio 1, Audio 2
- Drag-and-drop clips from Project Bin onto timeline tracks
- Playhead scrubbing with mouse drag
- In/Out point markers (press **I** / **O** keys)
- Track locking and muting/hiding
- Clip trimming and positioning

### Dual Monitor System
- **Clip Monitor**: Preview source media from Project Bin
- **Project Monitor**: Preview timeline output at playhead position
- Playback controls (Play/Pause, Stop)
- Frame-accurate seeking with keyboard shortcuts

### Project Bin (Media Management)
- Import video, audio, and image files via dialog or drag-and-drop
- Automatic media probing via FFmpeg (duration, resolution, FPS, codecs)
- Thumbnail generation for video clips
- Clip metadata display (duration, resolution, FPS, audio/video tracks)
- Add clips to timeline with double-click or "Add to Timeline" button
- Remove clips from project

### Effect Stack
- Apply effects to selected timeline clips
- **Text Overlay**: Custom text, font size, X/Y position
- **Crop Video**: Crop video frames
- Enable/disable effects with checkboxes
- Reorder and remove effects
- Real-time parameter updates

### Project Management
- **New Project**: Creates default 4-track timeline
- **Open Project** (.ffgui): Loads media clips, timeline clips, effects, track settings, playhead position
- **Save Project** (.ffgui): JSON-based project format preserving all timeline data
- **Recent Projects** tracking (via config file)

### Rendering & Export
- **Render Dialog** with FFmpeg integration
- Video codec selection (libx264, libx265, libvpx-vp9, etc.)
- Audio codec selection (aac, libmp3lame, libopus, etc.)
- Bitrate, FPS, resolution configuration
- Custom FFmpeg arguments (advanced mode)
- Real-time progress bar with time estimation
- Render history with double-click to open output files

### Theming
- **Arch Stealth** (dark, blue accents)
- **Kdenlive Dark** (classic Kdenlive look)
- **Nordic Frost** (Nord color palette)
- Persisted in `.config` file

### Keyboard Shortcuts
| Key | Action |
|-----|--------|
| `Space` | Play/Pause |
| `←` / `→` | Frame step backward/forward |
| `Ctrl+←` / `Ctrl+→` | Seek 5 seconds backward/forward |
| `Shift+←` / `Shift+→` | Jump to In/Out point |
| `I` | Set In point at playhead |
| `O` | Set Out point at playhead |
| `Ctrl+N` | New Project |
| `Ctrl+O` | Open Project |
| `Ctrl+S` | Save Project |
| `Ctrl+R` | Render Project |
| `F11` | Toggle Fullscreen |

### Drag & Drop
- Drop media files onto Main Window or Project Bin to import
- Drag clips from Project Bin to Timeline tracks
- Drop position determines track and timeline position

## Installation

### Prerequisites
- CMake 3.20 or higher
- Ninja build system
- Qt 6.2 or higher (Core, Gui, Widgets, Multimedia, MultimediaWidgets, Quick, QuickWidgets, Svg, SvgWidgets, OpenGL, OpenGLWidgets)
- FFmpeg 4.0 or higher (libavformat, libavcodec, libavutil, libavfilter, libavdevice, libswscale, libswresample)
- OpenGL
- OpenMP (optional, for parallel processing)

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/RainXuZhang/ffgui.git
   cd ffgui
   ```

2. Create a build directory and navigate to it:
   ```bash
   mkdir build
   cd build
   ```

3. Run CMake to configure the project:
   ```bash
   cmake -G Ninja ..
   ```

4. Build the project:
   ```bash
   ninja
   ```

5. Run the application:
   ```bash
   ./FFGui
   ```

## Project Structure

```
ffgui/
├── CMakeLists.txt
├── README.md
├── .config                 # Theme configuration (auto-generated)
├── src/
│   ├── main.cpp           # Application entry point
│   ├── MainWindow.h/cpp   # Main window, menus, layout, project I/O
│   ├── core/
│   │   ├── FFGuiApplication.h/cpp  # Qt application subclass, theming, settings
│   │   ├── FFmpegProbe.h/cpp       # FFmpeg media probing, thumbnail generation
│   │   ├── ProjectModel.h          # Data structures: MediaClip, TimelineClip, Track, Effect
│   │   └── Clip.h                  # Clip data structure
│   └── widgets/
│       ├── ProjectBinWidget.h/cpp  # Media bin with list view, drag-drop, import
│       ├── MonitorWidget.h/cpp     # Video preview with playback controls
│       ├── TimelineWidget.h/cpp    # Multi-track timeline with playhead, clips
│       ├── EffectStackWidget.h/cpp # Effect stack with parameter controls
│       └── RenderDialog.h/cpp      # FFmpeg render configuration and execution
└── build/                   # Build output (generated)
```

## Project File Format (.ffgui)

Projects are saved as JSON files with the following structure:
```json
{
  "duration": 300.0,
  "currentPlayhead": 0.0,
  "mediaClips": [
    {
      "id": "uuid",
      "filePath": "/path/to/video.mp4",
      "fileName": "video.mp4",
      "duration": 10.5,
      "width": 1920,
      "height": 1080,
      "fps": 30.0,
      "thumbnailPath": "/tmp/thumb_xxx.jpg",
      "hasAudio": true,
      "hasVideo": true
    }
  ],
  "tracks": [
    {
      "id": 1,
      "name": "Video 2",
      "isLocked": false,
      "isMutedOrHidden": false,
      "clips": [
        {
          "id": "uuid",
          "mediaClipId": "uuid",
          "timelineIn": 0.0,
          "timelineOut": 10.5,
          "sourceIn": 0.0,
          "duration": 10.5,
          "effects": [
            {
              "type": "text",
              "enabled": true,
              "params": { "text": "Hello", "size": 48, "x": 100, "y": 100 }
            }
          ]
        }
      ]
    }
  ]
}
```

## Dependencies

### Runtime
- Qt6 libraries (Core, Gui, Widgets, Multimedia, MultimediaWidgets, Quick, QuickWidgets, Svg, SvgWidgets, OpenGL, OpenGLWidgets)
- FFmpeg libraries (avformat, avcodec, avutil, avfilter, avdevice, swscale, swresample)
- OpenGL
- OpenMP (if available)

### Build-time
- CMake 3.20+
- Ninja
- pkg-config
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 19.28+)

## Contributing

Contributions are welcome! Please follow these steps to contribute:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your changes.
4. Commit your changes with a descriptive commit message.
5. Push your changes to your fork.
6. Create a pull request to the main repository.

## License

FFGui is licensed under the MIT License. See the LICENSE file for more details.

## Acknowledgments

- Built with [Qt6](https://www.qt.io/)
- Powered by [FFmpeg](https://ffmpeg.org/)
- Inspired by [Kdenlive](https://kdenlive.org/) workflow and layout
- Icons from system theme (Freedesktop.org specification)