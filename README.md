magickwand
==========

Native bindings for ImageMagick to resize, compress, and rotate images. This module allows you to process images either from filepaths or directly from Node.js Buffers. It can be used to dynamically manipulate images in an Express/Connect based server. See [examples/cdn.js](https://github.com/qzaidi/magickwand/blob/master/examples/cdn.js) for an example of connect middleware using magickwand.

Most other modules exposing ImageMagick invoke the `convert` utility as a child process, instead of making direct C API calls. While this works, API calls are much faster than invoking `convert`, and that is the motivation for this module.

Features
--------
* Resize images with options for width, height, quality, and aspect ratio preservation.
* Create image thumbnails.
* Rotate images by specified degrees.
* Support for input images as filepaths or Node.js Buffers.
* Option for automatic cropping during resize/thumbnail operations.
* Convert to different image formats.
* Improved error handling with proper JavaScript `Error` objects.

Example: Resize
----------------

**Using a filepath:**
```javascript
var magickwand = require('magickwand');
var fs = require('fs');

magickwand.resize('<pathtoimagefile>', { width: 300, height: 200, quality: 80 }, function(err, data, info) {
  if (err) {
    console.error("Resize Error (filepath):", err);
    return;
  }
  fs.writeFile('/tmp/resized_from_path.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  console.log('Resized image (from path) info:', info);
});
```

**Using a Buffer:**
```javascript
var magickwand = require('magickwand');
var fs = require('fs');

fs.readFile('<pathtoimagefile>', function(err, imageBuffer) {
  if (err || !imageBuffer) {
    console.error("Error reading image for buffer input:", err);
    return;
  }
  magickwand.resize(imageBuffer, { width: 150, height: 100, format: 'png' }, function(err, data, info) {
    if (err) {
      console.error("Resize Error (buffer):", err);
      return;
    }
    fs.writeFile('/tmp/resized_from_buffer.png', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
    console.log('Resized image (from buffer) info:', info);
  });
});
```

To maintain aspect ratio while resizing, set one of the `width` or `height` parameters to 0 (or omit it). The callback is passed an `info` argument that has the `width` and `height` of the newly resized image.

```javascript
magickwand.resize('<pathtoimagefile>', { width: 100 }, function(err, data, info) {
  if (err) return console.error(err);
  fs.writeFile('/tmp/resized_aspect.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  console.log('New height is ' + info.height); // info.width will also be available
});
```

To convert to another supported format, use the `format` argument (e.g., `'png'`, `'gif'`).

Example: Thumbnail
------------------
The `thumbnail` function works similarly to `resize` and also supports both filepath and Buffer inputs. It's generally optimized for creating small image thumbnails.

```javascript
var magickwand = require('magickwand');
var fs = require('fs');

magickwand.thumbnail('<pathtoimagefile>', { width: 120 }, function(err, data, info) {
  if (err) {
    console.error("Thumbnail Error:", err);
    return;
  }
  fs.writeFile('/tmp/thumb.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  console.log('Thumbnail info:', info);
});
```

Example: Rotate
----------------
The `rotate` function allows you to rotate an image. It supports both filepath and Buffer inputs.

**Parameters:**
*   `image (String | Buffer)`: Path to the image file or a Buffer containing the image data.
*   `options (Object)`: An object containing:
    *   `degrees (Number)`: The angle (in degrees) by which to rotate the image. Positive values rotate clockwise.
*   `callback (Function)`: A function to be called upon completion: `function(err, data, info)`.
    *   `err`: An `Error` object if an error occurred, otherwise `null`.
    *   `data`: A `Buffer` containing the rotated image data.
    *   `info`: An object containing the `width` and `height` of the rotated image.

**Using a filepath:**
```javascript
var magickwand = require('magickwand');
var fs = require('fs');

magickwand.rotate('<pathtoimagefile>', { degrees: 90 }, function(err, data, info) {
  if (err) {
    console.error("Rotate Error:", err);
    return;
  }
  fs.writeFile('/tmp/rotated_image.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  console.log('Rotated image info:', info); // info will contain new width/height
});
```

**Using a Buffer:**
```javascript
fs.readFile('<pathtoimagefile>', function(err, imageBuffer) {
  if (err || !imageBuffer) return console.error("Error reading image:", err);
  
  magickwand.rotate(imageBuffer, { degrees: -45 }, function(err, data, info) {
    if (err) {
      console.error("Rotate Error (buffer):", err);
      return;
    }
    fs.writeFile('/tmp/rotated_from_buffer.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
    console.log('Rotated image (from buffer) info:', info);
  });
});
```

Error Handling
--------------
The module now consistently returns JavaScript `Error` objects in the callback as the first argument (`err`) if an operation fails. This provides more detailed error information from ImageMagick. Always check for this error object in your callbacks.

```javascript
magickwand.resize('<nonexistentfile.jpg>', { width: 100 }, function(err, data, info) {
  if (err) {
    console.error("Operation failed!");
    console.error("Error message:", err.message);
    // Example error: Error: [Error: unable to open image `/tmp/nonexistentfile.jpg': No such file or directory @ error/blob.c/OpenBlob/3537]
    return;
  }
  // Proceed with data and info
});
```

Requirements
------------

magickwand uses the C library by the same name, so `libmagickwand-dev` (or its equivalent) should be installed.

On Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install libmagickwand-dev
```

On macOS (using Homebrew):
```bash
brew install pkg-config
brew install imagemagick --disable-openmp
```
Note: If you encounter issues with OpenMP causing Node.js processes to hang, ensure ImageMagick is compiled with OpenMP disabled. The `--disable-openmp` flag with Homebrew helps with this.

On SmartOS (using pkgin):
```bash
pkgin install ImageMagick
```

Installation
------------

```bash
npm install magickwand
```

Source Install / Manual Compilation
-----------------------------------

```bash
git clone git://github.com/qzaidi/magickwand.git
cd magickwand
npm install # This will also run node-gyp rebuild
# Or manually:
# node-gyp configure
# node-gyp build
```
