#!/usr/bin/env python
"""
Windows ImageMagick installation path discovery for node-gyp build.
Queries Windows registry and falls back to environment variables and common paths.
"""

import sys
import os

def get_imagemagick_root():
    """
    Discover ImageMagick installation root directory on Windows.

    Priority order:
    1. Windows Registry (HKLM\\SOFTWARE\\ImageMagick\\Current)
    2. Environment variable (MAGICK_HOME)
    3. Common installation paths

    Returns:
        str: Absolute path to ImageMagick root directory

    Raises:
        SystemExit: If ImageMagick cannot be found
    """
    try:
        import winreg
    except ImportError:
        # Not on Windows - this script should only run on Windows
        sys.stderr.write('ERROR: This script is for Windows only\n')
        sys.exit(1)

    # Strategy 1: Query Windows Registry
    registry_paths = [
        (winreg.HKEY_LOCAL_MACHINE, r'SOFTWARE\ImageMagick\Current'),
        (winreg.HKEY_LOCAL_MACHINE, r'SOFTWARE\WOW6432Node\ImageMagick\Current'),
    ]

    for hkey, key_path in registry_paths:
        try:
            key = winreg.OpenKey(hkey, key_path)
            bin_path, _ = winreg.QueryValueEx(key, 'BinPath')
            winreg.CloseKey(key)

            # BinPath points to bin/ directory, we need parent
            root = os.path.dirname(bin_path.rstrip('\\'))

            # Verify this is a valid ImageMagick installation
            if validate_imagemagick_root(root):
                return root

        except WindowsError:
            continue

    # Strategy 2: Check MAGICK_HOME environment variable
    if 'MAGICK_HOME' in os.environ:
        root = os.environ['MAGICK_HOME']
        sys.stderr.write(f'Checking MAGICK_HOME: {root}\n')
        if validate_imagemagick_root(root):
            return root
        else:
            sys.stderr.write(f'MAGICK_HOME validation failed for: {root}\n')

    # Strategy 3: Search common installation paths
    common_paths = [
        r'C:\Program Files\ImageMagick-7.1.1-Q16-HDRI',
        r'C:\Program Files\ImageMagick-7.1.0-Q16-HDRI',
        r'C:\Program Files\ImageMagick-7.0.11-Q16-HDRI',
        r'C:\ImageMagick',
    ]

    # Also search for any ImageMagick-7.* in Program Files
    program_files = os.environ.get('ProgramFiles', r'C:\Program Files')
    if os.path.exists(program_files):
        try:
            for entry in os.listdir(program_files):
                if entry.startswith('ImageMagick-7'):
                    candidate = os.path.join(program_files, entry)
                    if validate_imagemagick_root(candidate):
                        return candidate
        except (OSError, PermissionError):
            pass

    # Try common paths as last resort
    for path in common_paths:
        if validate_imagemagick_root(path):
            return path

    # Not found - provide helpful error message
    error_msg = '''
ImageMagick installation not found.

Please install ImageMagick with development headers:

  Using Chocolatey (Recommended):
    choco install imagemagick.app -PackageParameters InstallDevelopmentHeaders=true

  Or download from:
    https://imagemagick.org/script/download.php#windows

  During installation, ensure you select:
    [x] Install development headers and libraries for C and C++

  If ImageMagick is installed in a custom location, set MAGICK_HOME:
    setx MAGICK_HOME "C:\\Path\\To\\ImageMagick"

For more help, see: docs/TROUBLESHOOTING.md
'''
    sys.stderr.write(error_msg)
    sys.exit(1)


def validate_imagemagick_root(path):
    """
    Verify that a path contains a valid ImageMagick installation.

    Args:
        path (str): Directory path to validate

    Returns:
        bool: True if path contains ImageMagick with development headers
    """
    if not os.path.isdir(path):
        sys.stderr.write(f'  Not a directory: {path}\n')
        return False

    # Check for required header files
    include_path = os.path.join(path, 'include', 'MagickWand', 'MagickWand.h')
    if not os.path.exists(include_path):
        sys.stderr.write(f'  Header not found: {include_path}\n')
        # List what's actually there
        inc_dir = os.path.join(path, 'include')
        if os.path.exists(inc_dir):
            sys.stderr.write(f'  Contents of {inc_dir}: {os.listdir(inc_dir)}\n')
        return False

    # Check for required library files
    lib_path = os.path.join(path, 'lib')
    if not os.path.isdir(lib_path):
        sys.stderr.write(f'  Lib directory not found: {lib_path}\n')
        return False

    # Look for core libraries (at least one should exist)
    required_libs = [
        'CORE_RL_MagickCore_.lib',
        'CORE_RL_MagickWand_.lib',
        'CORE_RL_wand_.lib',
    ]

    has_lib = any(
        os.path.exists(os.path.join(lib_path, lib))
        for lib in required_libs
    )

    if not has_lib:
        sys.stderr.write(f'  No required libs found in {lib_path}\n')
        sys.stderr.write(f'  Contents: {os.listdir(lib_path) if os.path.exists(lib_path) else "N/A"}\n')

    return has_lib


if __name__ == '__main__':
    root = get_imagemagick_root()
    # Output path for node-gyp (no trailing newline issues)
    sys.stdout.write(root)
