'use strict';

const mgwnd = require('./build/Release/magickwand');

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
   * @param {Function} cb - Callback function (err, data, info)
   * @returns {void}
   */
  resize(imagefile, options, cb) {
    if (!options || typeof options !== 'object' || !Object.keys(options).length) {
      throw new Error('Invalid options: must provide at least one of width, height, format, or quality');
    }

    const params = {
      width: options.width || 0,
      height: options.height || 0,
      quality: options.quality || 0,
      format: options.format || undefined,
      autocrop: options.autocrop || false
    };

    mgwnd.resize(
      imagefile,
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
   * @param {Function} cb - Callback function (err, data, info)
   * @returns {void}
   */
  thumbnail(imagefile, options, cb) {
    // Handle optional options parameter
    if (typeof options === 'function') {
      cb = options;
      options = {};
    }

    const params = {
      width: options.width || 0,
      height: options.height || 0,
      quality: options.quality || 0,
      autocrop: options.autocrop || false
    };

    mgwnd.thumbnail(
      imagefile,
      params.width,
      params.height,
      params.quality,
      params.autocrop,
      cb
    );
  }
};

module.exports = magickwand;
