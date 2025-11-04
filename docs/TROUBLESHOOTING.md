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

## Windows-Specific Issues

### "ImageMagick not found" during npm install

**Cause**: ImageMagick not installed or development headers missing.

**Solution**:

1. Verify ImageMagick is installed:
   ```powershell
   magick --version
   ```

2. Check for development headers:
   ```powershell
   dir "C:\Program Files\ImageMagick-*\include\MagickWand\MagickWand.h"
   dir "C:\Program Files\ImageMagick-*\lib\CORE_RL_*.lib"
   ```

3. If missing, reinstall with development headers (Chocolatey):
   ```powershell
   choco install imagemagick.app -y --package-parameters="InstallDevelopmentHeaders=true" --force
   ```

4. Or set `MAGICK_HOME` if installed in custom location:
   ```powershell
   setx MAGICK_HOME "C:\Program Files\ImageMagick-7.1.1-Q16-HDRI"
   ```

5. Rebuild native module:
   ```powershell
   npm rebuild
   ```

### node-gyp build errors on Windows

**Error**: `Cannot find module 'msbuild'` or `MSBuild not found`

**Solution**: Install Visual Studio Build Tools:

```powershell
choco install visualstudio2022-workload-vctools -y
```

Or download manually:
https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022

### Python not found during build

**Error**: `Can't find Python executable "python"`

**Solution**:

```powershell
choco install python -y
```

Or install from [python.org](https://www.python.org/downloads/) and ensure it's in PATH:
```powershell
python --version
```

### CORE_RL_*.lib not found

**Error**: `fatal error LNK1104: cannot open file 'CORE_RL_MagickCore_.lib'`

**Cause**: ImageMagick libraries not found or ImageMagick installed without development headers.

**Solution**:

1. Verify library files exist:
   ```powershell
   dir "C:\Program Files\ImageMagick-*\lib\*.lib"
   ```

2. If missing, reinstall ImageMagick with development headers:
   ```powershell
   choco install imagemagick.app -y --package-parameters="InstallDevelopmentHeaders=true" --force
   ```

3. If installed in custom location, set `MAGICK_HOME`:
   ```powershell
   setx MAGICK_HOME "C:\Your\Custom\Path\ImageMagick"
   npm rebuild
   ```

### Windows registry not found

**Error**: `ERROR: This script is for Windows only`

**Cause**: Running on Windows but Python can't import winreg module.

**Solution**: Ensure you're using Python for Windows (not WSL Python):
```powershell
python --version
where python
```

Should show: `C:\Python3x\python.exe` (not `/usr/bin/python`)

### Module fails to load at runtime

**Error**: `The specified module could not be found` or DLL errors

**Cause**: ImageMagick DLLs not in PATH.

**Solution**:

1. Verify ImageMagick bin directory is in PATH:
   ```powershell
   $env:PATH -split ';' | Select-String -Pattern "ImageMagick"
   ```

2. If missing, add to PATH:
   ```powershell
   setx PATH "$env:PATH;C:\Program Files\ImageMagick-7.1.1-Q16-HDRI"
   ```

3. Or reinstall ImageMagick with "Add to system path" option checked.

4. Restart terminal/IDE for PATH changes to take effect.

### Build succeeds but tests fail

**Symptom**: `npm install` succeeds but `npm test` fails with image processing errors.

**Solution**:

1. Verify ImageMagick works standalone:
   ```powershell
   magick convert test.jpg -resize 100x100 output.jpg
   ```

2. Check ImageMagick version (v7.x required):
   ```powershell
   magick --version
   ```

3. Ensure development headers match installed version:
   ```powershell
   dir "C:\Program Files\ImageMagick-*\include"
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
