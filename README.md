# magickwand

[![npm version](https://badge.fury.io/js/magickwand.svg)](https://www.npmjs.com/package/magickwand)
[![Node.js Version](https://img.shields.io/node/v/magickwand.svg)](https://nodejs.org)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Native Node.js bindings for ImageMagick - High-performance image resizing and manipulation using direct C API calls instead of spawning child processes.

## Features

- ✨ **Native Performance**: Direct C API calls to ImageMagick (no child processes)
- 🖼️ **Resize & Thumbnail**: Multiple resizing strategies with quality control
- ✂️ **Auto-crop**: Smart cropping with gravity at center
- 🎨 **Format Conversion**: Convert between image formats (JPEG, PNG, etc.)
- 📐 **Aspect Ratio**: Automatic aspect ratio preservation
- ⚡ **Modern**: Built for Node.js v10+ with ImageMagick 7 support
- ✅ **Well-tested**: Comprehensive test suite with 15+ tests

## Requirements

### Node.js
- Node.js >= 10.0.0 (tested up to v24.x)

### ImageMagick

**macOS** (using Homebrew):
```bash
brew install pkg-config imagemagick
```

**Ubuntu/Debian**:
```bash
sudo apt-get install libmagickwand-dev
```

**Note**: If you experience issues on macOS with OpenMP, install without it:
```bash
brew install imagemagick --without-openmp
```

## Installation

```bash
npm install magickwand
```

### Building from Source

```bash
git clone https://github.com/vnykmshr/magickwand.git
cd magickwand
npm install
```

## Usage

### Basic Resize

```javascript
const magickwand = require('magickwand');
const fs = require('fs');

// Resize to specific dimensions
magickwand.resize('input.jpg', {
  width: 300,
  height: 200,
  quality: 80
}, (err, data, info) => {
  if (err) throw err;

  fs.writeFileSync('output.jpg', data, 'binary');
  console.log('Resized to:', info); // { width: 300, height: 200 }
});
```

### Maintain Aspect Ratio

Set one dimension to `0` to maintain aspect ratio:

```javascript
// Resize width to 100px, height calculated automatically
magickwand.resize('input.jpg', {
  width: 100,
  height: 0  // Auto-calculated
}, (err, data, info) => {
  if (err) throw err;

  console.log('New dimensions:', info); // { width: 100, height: 75 }
  fs.writeFileSync('output.jpg', data, 'binary');
});
```

### Format Conversion

```javascript
// Convert JPEG to PNG
magickwand.resize('input.jpg', {
  format: 'png',
  width: 300,
  height: 250
}, (err, data) => {
  if (err) throw err;

  fs.writeFileSync('output.png', data, 'binary');
});
```

### Auto-crop with Gravity

Smart cropping with gravity at center:

```javascript
magickwand.resize('input.jpg', {
  width: 200,
  height: 200,
  autocrop: true  // Crop to exact dimensions from center
}, (err, data, info) => {
  if (err) throw err;

  fs.writeFileSync('cropped.jpg', data, 'binary');
});
```

### Thumbnail (Optimized for Smaller File Size)

The `thumbnail` method is optimized for generating smaller file sizes:

```javascript
magickwand.thumbnail('input.jpg', {
  width: 150,
  height: 150,
  quality: 75
}, (err, data, info) => {
  if (err) throw err;

  console.log('Thumbnail quality:', info.quality);
  fs.writeFileSync('thumb.jpg', data, 'binary');
});
```

## API

### `resize(imagefile, options, callback)`

Resize an image with various options.

**Parameters:**
- `imagefile` (string): Path to the input image
- `options` (Object):
  - `width` (number): Target width in pixels (0 = auto from height)
  - `height` (number): Target height in pixels (0 = auto from width)
  - `quality` (number): JPEG quality 1-100 (0 = default)
  - `format` (string): Output format ('png', 'jpg', etc.)
  - `autocrop` (boolean): Enable smart cropping (default: false)
- `callback` (Function): `(err, data, info) => {}`
  - `err`: Error object or null
  - `data`: Buffer containing image data
  - `info`: Object with `{ width, height }` of result

### `thumbnail(imagefile, options, callback)`

Create an optimized thumbnail (better compression than resize).

**Parameters:** Same as `resize()`, except `format` is not supported.

**Returns:** Callback with `info` object containing `{ width, height, quality }`

## Use as Express/Connect Middleware

See [examples/cdn.js](examples/cdn.js) for an example of using magickwand as middleware for dynamic image resizing in Express.

## Testing

```bash
npm test
```

Runs comprehensive test suite covering:
- Error handling (missing files, invalid params)
- Resize operations with various options
- Aspect ratio preservation
- Format conversion
- Auto-crop functionality
- Thumbnail generation

## Performance

MagickWand provides significant performance benefits over child process-based solutions:

- **Direct API calls**: No process spawning overhead
- **Memory efficient**: Direct buffer manipulation
- **Faster execution**: Native C bindings eliminate IPC overhead

## Compatibility

- **Node.js**: v10.0.0 and above (tested up to v24.x)
- **ImageMagick**: v7.x (with backward compatibility for v6.x)
- **Platforms**: macOS (darwin), Linux

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history.

## License

MIT

## Credits

- Original author: [Qasim Zaidi](https://github.com/qzaidi)
- Contributors: Sameer Akhtar, Binayak Mishra
- Maintainer: [Vinayak Mishra](https://github.com/vnykmshr)
- Modernization: Updated for Node.js v24 and ImageMagick 7

## Related Projects

- [imagemagick](https://www.npmjs.com/package/imagemagick) - Child process-based ImageMagick wrapper
- [gm](https://www.npmjs.com/package/gm) - GraphicsMagick and ImageMagick wrapper
- [sharp](https://www.npmjs.com/package/sharp) - High-performance image processing (libvips-based)
