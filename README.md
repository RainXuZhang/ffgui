# FFGui

FFGui is a powerful GUI application for video editing and processing.

## Installation

### Prerequisites

- CMake 3.10 or higher
- Ninja build system
- Qt 5.15 or higher
- FFmpeg 4.0 or higher

### Steps

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
   cmake -G Ninja .
   ```

4. Build the project:
   ```bash
   ninja
   ```

5. Run the application:
   ```bash
   ./FFGui
   ```

## Usage

### Main Window

The main window provides access to all the features of FFGui. It consists of several widgets:

- **Timeline Widget**: Displays and edits the timeline of the video project.
- **Monitor Widget**: Shows the video preview.
- **Project Bin Widget**: Manages the media files in the project.
- **Effect Stack Widget**: Applies and manages effects on the selected clip.

### Timeline Widget

The Timeline Widget allows you to:

- Add, remove, and move clips.
- Trim and split clips.
- Adjust the duration of clips.
- Add transitions between clips.

### Monitor Widget

The Monitor Widget provides:

- Video preview of the current frame.
- Playback controls (play, pause, stop).
- Frame navigation (previous frame, next frame).

### Project Bin Widget

The Project Bin Widget helps you:

- Import media files (video, audio, images).
- Organize media files into folders.
- Search for media files.

### Effect Stack Widget

The Effect Stack Widget allows you to:

- Apply effects to the selected clip.
- Adjust effect parameters.
- Reorder effects.
- Remove effects.

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