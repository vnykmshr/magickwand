"use strict";

var mgwnd = require('./build/Release/magickwand');
// console.log("Native addon 'mgwnd' object:", mgwnd); // Debugging line removed

var magickwand = {
  resize: function (imagefile, options, cb) {

    if (!Object.keys(options).length) {
      throw new Error('Invalid width/height/format/quality arguments');
    }

    ['quality', 'width', 'height'].forEach(function (param) {
      if (!options[param])
        options[param] = 0;
    });

    options.autocrop = options.autocrop || false;

    mgwnd.resizeAsync(imagefile, options.width, options.height, options.quality, options.format, options.autocrop, cb);
  },

  thumbnail: function (imagefile, options, cb) {
    var args = {};
    if (typeof options === 'function') {
      cb = options;
      options = {};
    }

    ['width', 'height', 'quality'].forEach(function (param) {
      args[param] = options[param] || 0;
    });

    options.autocrop = options.autocrop || false;

    mgwnd.thumbnailAsync(imagefile, args.width, args.height, args.quality, options.autocrop, cb);
  },

  rotate: function (imagefileOrBuffer, options, cb) {
    if (typeof options === 'function') {
      cb = options;
      options = {};
    }
    if (typeof options.degrees !== 'number') {
      // Immediately invoke callback with an error if degrees is not a number
      // This ensures async behavior consistent with other functions if input validation fails early.
      if (typeof cb === 'function') {
        // Use process.nextTick to ensure async callback
        process.nextTick(function() {
          cb(new Error('Invalid degrees: must be a number.'));
        });
      } else {
        // If no callback, throw error (though API expects callback)
        throw new Error('Invalid degrees: must be a number and callback is required.');
      }
      return;
    }

    mgwnd.rotateAsync(imagefileOrBuffer, options.degrees, cb);
  }
};

module.exports = magickwand;
