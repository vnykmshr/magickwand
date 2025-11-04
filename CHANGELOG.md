# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - TBD

### Added
- **Windows platform support**: Full first-class support for Windows alongside macOS and Linux
  - Windows-specific build configuration in `binding.gyp`
  - Automatic ImageMagick discovery via Windows registry
  - Support for multiple installation methods (Chocolatey, manual installer, custom paths)
  - Comprehensive Windows troubleshooting documentation
- CI/CD coverage for Windows: Added `windows-latest` to GitHub Actions matrix
  - Tests all three platforms: macOS, Linux, Windows
  - Node.js versions: 18.x, 20.x, 22.x
  - Automated ImageMagick installation and verification
- `tools/get_magick_root.py`: Windows registry discovery script
  - Queries registry for ImageMagick installation path
  - Falls back to `MAGICK_HOME` environment variable
  - Searches common installation paths
  - Provides helpful error messages with installation instructions

### Changed
- Updated platform support: `"os": ["darwin", "linux", "win32"]` in `package.json`
- Enhanced README: Added Windows installation instructions
  - Chocolatey installation (recommended primary method)
  - Manual installer instructions
  - Custom location configuration via `MAGICK_HOME`
- Enhanced TROUBLESHOOTING: Added 8 Windows-specific troubleshooting sections
  - ImageMagick installation issues
  - node-gyp build errors
  - Python and Visual Studio dependencies
  - Library and DLL path issues
  - Runtime loading errors
- Unified CI workflow: Single matrix job covering all three platforms

### Documentation
- Windows installation methods documented with priority on Chocolatey
- Windows-specific build requirements (Python, Visual Studio Build Tools)
- Compatibility section updated: "Platforms: macOS, Linux, Windows"
- Windows error messages and solutions added to troubleshooting guide

## [1.0.0] - 2025-11-04

### Added
- **Security hardening**: Path validation to prevent directory traversal attacks
  - Validates file exists and is readable before processing
  - Checks for null bytes in file paths
  - Sanitizes error messages to avoid leaking filesystem information
- **Resource limits**: Configurable dimension limits with sensible defaults
  - Default maximum dimension: 16384 pixels (width or height)
  - Users can override via `options.maxDimension`
  - Prevents resource exhaustion attacks

### Changed
- **BREAKING**: Now validates all file paths before processing
  - Non-existent files return error asynchronously via callback
  - Invalid paths (null bytes, non-files) throw errors
- **Code quality**: Major refactoring to eliminate C++ code duplication
  - Extracted shared logic into `src/common.cpp`
  - Reduced codebase by ~100 lines
  - Improved maintainability
- Modernized test suite: `var` → `const`/`let`
- Error handling improvements: sanitized error messages

### Fixed
- Potential buffer overflow in path handling
- Memory leak in error paths (missing null checks)

### Security
- Fixed path traversal vulnerability (CVE pending)
- Added input sanitization for file paths
- Improved error message security (no path disclosure)

### Technical Debt Addressed
- Eliminated 85% code duplication between resize.cpp and thumbnail.cpp
- Consolidated error handling logic
- Improved C++ code organization and maintainability

**Migration Notes:**
- No API changes from 0.0.12
- Path validation is now stricter: ensure all file paths are valid before calling resize/thumbnail
- Error messages are more generic for security reasons
- If you need custom resource limits, use `options.maxDimension` parameter

## [0.0.12] - 2025-11-03

### Added
- Comprehensive test suite with 15 tests using Mocha
- Test image for automated testing
- npm test script in package.json
- JSDoc documentation for all public API methods
- ESLint configuration for code quality
- CONTRIBUTING.md with development guidelines
- CHANGELOG.md for version tracking
- Modern README with badges and detailed examples
- Auto-crop functionality with gravity at center

### Changed
- **BREAKING**: Modernized to Node.js v24 and ImageMagick 7
- Updated all C++ code to use modern Node.js N-API
- Modernized JavaScript to ES6+ (const/let, arrow functions, template literals)
- Improved package.json metadata with proper repository links
- Updated minimum Node.js requirement to v10.0.0
- Enhanced error messages for better debugging
- Fixed typo in thumbnail function call
- Renamed default branch from master to main

### Fixed
- Typo in index.js (removed extra 'ss' in thumbnail call)
- Logic bug in width/height checking (!mgr->width == 0 → mgr->width == 0)
- uv_after_work_cb signature compatibility
- MagickResizeImage API compatibility (removed blur parameter for ImageMagick 7)

### Deprecated
- None

### Removed
- None

### Security
- Updated all dependencies to latest secure versions

## [0.0.11] - Historical (original upstream)

### Changed
- Added test infrastructure (original qzaidi/magickwand project)
- Added PNG test support

**Note**: Version 0.0.11 and earlier are from the original qzaidi/magickwand project. This repository diverged at v0.0.10 and was modernized to v0.0.12.

## [0.0.10] - Historical

### Added
- Thumbnail quality parameter support
- Quality attribute in thumbnail info object

### Changed
- Version bump to 0.0.10

## [0.0.9] - Historical

### Added
- Resize support in thumbnail function
- Quality parameter for thumbnail

## [0.0.8] - Historical

### Added
- Initial autocrop support with gravity at center
- Format attribute for specifying output format
- Quality parameter for JPEG compression

### Changed
- Code formatting improvements
- Minor lint fixes

## Earlier Versions

See git history for changes in versions prior to 0.0.8.

---

## Version Numbering

This project uses [Semantic Versioning](https://semver.org/):
- **MAJOR**: Incompatible API changes
- **MINOR**: New functionality (backwards compatible)
- **PATCH**: Bug fixes (backwards compatible)

## Migration Guides

### Migrating from 0.0.11 to 0.0.12

**Requirements:**
- Upgrade to Node.js v10.0.0 or higher (tested up to v24)
- Upgrade to ImageMagick 7 (ImageMagick 6 may still work but is not officially supported)

**Breaking API Changes:**

The resize and thumbnail functions now require an **options object** instead of positional parameters:

**Old API (0.0.11 and earlier):**
```javascript
// Positional parameters (NO LONGER WORKS)
magickwand.resize('/path/to/image.jpg', 800, 600, function(err, data) {
  // ...
});
```

**New API (0.0.12+):**
```javascript
// Options object (REQUIRED)
magickwand.resize('/path/to/image.jpg', {
  width: 800,
  height: 600,
  quality: 80,      // optional
  format: 'png',    // optional
  autocrop: false   // optional
}, function(err, data, info) {
  // ...
});

// Thumbnail follows the same pattern
magickwand.thumbnail('/path/to/image.jpg', {
  width: 200,
  height: 200,
  quality: 70,
  autocrop: true
}, function(err, data, info) {
  // ...
});
```

**Migration Steps:**

1. Update all `resize()` and `thumbnail()` calls to use options object:
   ```javascript
   // Before
   magickwand.resize(path, width, height, callback);

   // After
   magickwand.resize(path, { width, height }, callback);
   ```

2. Clean old build and reinstall:
   ```bash
   rm -rf build node_modules
   npm install
   ```

3. Test your application thoroughly - the API change is **not backwards compatible**

---

[1.0.0]: https://github.com/vnykmshr/magickwand/releases/tag/v1.0.0
[0.0.12]: https://github.com/vnykmshr/magickwand/releases/tag/v0.0.12
[0.0.11]: Historical version from original upstream project
[0.0.10]: Historical version from original upstream project
