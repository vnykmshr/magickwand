"use strict";

var mgwnd = require('./build/Release/magickwand');

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

    mgwnd.resize(imagefile, options.width, options.height, options.quality, options.format, options.autocrop, cb);
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

    mgwnd.thumbnail(imagefile, args.width, args.height, args.quality,ss options.autocrop, cb);
  }
};

module.exports = magickwand;
