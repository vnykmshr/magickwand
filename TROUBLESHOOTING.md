# Troubleshooting

Common issues and solutions for MagickWand.

## Installation Problems

### "MagickWand-config: command not found"

**Cause**: ImageMagick is not installed or not in PATH.

**Solution**:

macOS:
```bash
brew install pkg-config imagemagick
```

Ubuntu/Debian:
```bash
sudo apt-get install libmagickwand-dev
```

Verify installation:
```bash
MagickWand-config --version
```

### Build fails with "node-gyp" errors

**Cause**: Missing build tools.

**Solution**:

macOS:
```bash
xcode-select --install
```

Linux:
```bash
sudo apt-get install build-essential
```

### OpenMP-related errors on macOS

**Error**: `ld: library not found for -lomp`

**Solution**: Install ImageMagick without OpenMP support:
```bash
brew install imagemagick --without-openmp
```

Or install libomp:
```bash
brew install libomp
```

## Runtime Errors

### "Image file not found"

**Cause**: File path is incorrect or file doesn't exist.

**Solution**: Use absolute paths or verify relative paths:
```javascript
const path = require('path');
const imagePath = path.resolve('./images/photo.jpg');

magickwand.resize(imagePath, { width: 300 }, callback);
```

### "Width/height exceeds maximum allowed dimension"

**Cause**: Requested dimensions exceed the default 16384px limit.

**Solution**: Override the limit if you need larger dimensions:
```javascript
magickwand.resize('image.jpg', {
  width: 20000,
  maxDimension: 25000  // Custom limit
}, callback);
```

### "Invalid width/height arguments"

**Cause**: Negative values or invalid types.

**Solution**: Ensure dimensions are positive integers or 0:
```javascript
// Correct
{ width: 100, height: 0 }  // height auto-calculated

// Wrong
{ width: -100, height: null }
```

### "Unable to open blob: no decode delegate"

**Cause**: ImageMagick doesn't support the image format.

**Solution**: Check ImageMagick's supported formats:
```bash
magick -list format
```

Install format delegates if needed (e.g., for JPEG):
```bash
brew reinstall imagemagick --with-libjpeg
```

## Performance Issues

### Slow processing times

**Check**: File size and dimensions of source images.

**Solutions**:
1. Pre-process very large images before using MagickWand
2. Use `thumbnail()` instead of `resize()` for smaller output
3. Lower quality settings for faster processing

### Memory issues with large images

**Error**: Process runs out of memory

**Solution**: Process images sequentially rather than in parallel:
```javascript
// Avoid parallel processing of many large images
async function processImages(files) {
  for (const file of files) {
    await processOne(file);  // Sequential
  }
}
```

## Testing Issues

### Tests fail with "Image file not found"

**Cause**: Running tests from wrong directory.

**Solution**:
```bash
cd /path/to/magickwand
npm test
```

### Build fails before running tests

**Cause**: Native module needs rebuilding.

**Solution**:
```bash
rm -rf build
npm install
npm test
```

## Version Compatibility

### ImageMagick 6 vs 7 differences

MagickWand v1.0.0+ is optimized for ImageMagick 7 but maintains backward compatibility with v6.

If you're on ImageMagick 6:
```bash
# Check version
magick --version  # ImageMagick 7
convert --version  # ImageMagick 6

# Upgrade to ImageMagick 7 (recommended)
brew upgrade imagemagick
```

### Node.js version issues

MagickWand requires Node.js v10.0.0 or higher.

Check your version:
```bash
node --version
```

Upgrade if needed:
```bash
# Using nvm
nvm install 20
nvm use 20
```

## Still Having Issues?

1. Check existing [GitHub Issues](https://github.com/vnykmshr/magickwand/issues)
2. Run with verbose logging to get more details
3. Include the following in bug reports:
   - Node.js version (`node --version`)
   - ImageMagick version (`magick --version`)
   - Operating system
   - Full error message
   - Minimal code to reproduce
