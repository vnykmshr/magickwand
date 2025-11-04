'use strict';

const mgwnd = require('./build/Release/magickwand');
const fs = require('fs');
const path = require('path');

/**
 * Default resource limits for security
 * Users can override these in options
 */
const DEFAULT_LIMITS = {
  maxDimension: 16384  // Maximum width or height in pixels
};

/**
 * Validate and sanitize image file path
 * @param {string} imagefile - Path to validate
 * @throws {Error} If path is invalid or unsafe
 * @returns {string} Resolved absolute path
 */
function validateImagePath(imagefile) {
  if (!imagefile || typeof imagefile !== 'string') {
    throw new Error('Invalid image path: must be a non-empty string');
  }

  // Check for null bytes (security)
  if (imagefile.includes('\0')) {
    throw new Error('Invalid image path: contains null bytes');
  }

  // Resolve to absolute path
  const resolvedPath = path.resolve(imagefile);

  // Check if file exists
  try {
    const stats = fs.statSync(resolvedPath);

    // Must be a file, not directory
    if (!stats.isFile()) {
      throw new Error('Invalid image path: must be a file');
    }

    // Check if readable
    fs.accessSync(resolvedPath, fs.constants.R_OK);

    return resolvedPath;
  } catch (err) {
    if (err.code === 'ENOENT') {
      throw new Error(`Image file not found: ${path.basename(imagefile)}`);
    } else if (err.code === 'EACCES') {
      throw new Error(`Image file not readable: ${path.basename(imagefile)}`);
    }
    throw err;
  }
}

/**
 * Validate resource limits
 * @param {Object} options - Options with width/height
 * @param {Object} limits - Resource limits
 * @throws {Error} If dimensions exceed limits
 */
function validateResourceLimits(options, limits) {
  const maxDim = limits.maxDimension || DEFAULT_LIMITS.maxDimension;

  if (options.width && options.width > maxDim) {
    throw new Error(`Width ${options.width} exceeds maximum allowed dimension ${maxDim}`);
  }

  if (options.height && options.height > maxDim) {
    throw new Error(`Height ${options.height} exceeds maximum allowed dimension ${maxDim}`);
  }
}

/**
 * MagickWand - Native Node.js bindings for ImageMagick
 * High-performance image resizing and manipulation
 */
const magickwand = {
  /**
   * Resize an image with various options
   * @param {string} imagefile - Path to the image file
   * @param {Object} options - Resize options
   * @param {number} [options.width=0] - Target width (0 to maintain aspect ratio)
   * @param {number} [options.height=0] - Target height (0 to maintain aspect ratio)
   * @param {number} [options.quality=0] - JPEG quality (1-100, 0 for default)
   * @param {string} [options.format] - Target format (e.g., 'png', 'jpg')
   * @param {boolean} [options.autocrop=false] - Enable autocrop with gravity at center
   * @param {number} [options.maxDimension] - Override maximum allowed dimension
   * @param {Function} cb - Callback function (err, data, info)
   * @returns {void}
   */
  resize(imagefile, options, cb) {
    if (!options || typeof options !== 'object' || !Object.keys(options).length) {
      throw new Error('Invalid options: must provide at least one of width, height, format, or quality');
    }

    // Validate file path (security)
    let validatedPath;
    try {
      validatedPath = validateImagePath(imagefile);
    } catch (err) {
      // Return errors asynchronously to match API expectations
      return process.nextTick(() => cb(err));
    }

    const params = {
      width: options.width || 0,
      height: options.height || 0,
      quality: options.quality || 0,
      format: options.format || undefined,
      autocrop: options.autocrop || false
    };

    // Validate resource limits
    try {
      validateResourceLimits(params, {
        maxDimension: options.maxDimension
      });
    } catch (err) {
      return process.nextTick(() => cb(err));
    }

    mgwnd.resize(
      validatedPath,
      params.width,
      params.height,
      params.quality,
      params.format,
      params.autocrop,
      cb
    );
  },

  /**
   * Create a thumbnail of an image (optimized for smaller file sizes)
   * @param {string} imagefile - Path to the image file
   * @param {Object|Function} options - Thumbnail options or callback
   * @param {number} [options.width=0] - Target width (0 to maintain aspect ratio)
   * @param {number} [options.height=0] - Target height (0 to maintain aspect ratio)
   * @param {number} [options.quality=0] - JPEG quality (1-100, 0 for default)
   * @param {boolean} [options.autocrop=false] - Enable autocrop with gravity at center
   * @param {number} [options.maxDimension] - Override maximum allowed dimension
   * @param {Function} cb - Callback function (err, data, info)
   * @returns {void}
   */
  thumbnail(imagefile, options, cb) {
    // Handle optional options parameter
    if (typeof options === 'function') {
      cb = options;
      options = {};
    }

    // Validate file path (security)
    let validatedPath;
    try {
      validatedPath = validateImagePath(imagefile);
    } catch (err) {
      // Return errors asynchronously to match API expectations
      return process.nextTick(() => cb(err));
    }

    const params = {
      width: options.width || 0,
      height: options.height || 0,
      quality: options.quality || 0,
      autocrop: options.autocrop || false
    };

    // Validate resource limits
    try {
      validateResourceLimits(params, {
        maxDimension: options.maxDimension
      });
    } catch (err) {
      return process.nextTick(() => cb(err));
    }

    mgwnd.thumbnail(
      validatedPath,
      params.width,
      params.height,
      params.quality,
      params.autocrop,
      cb
    );
  }
};

module.exports = magickwand;
