"use strict";

// This example demonstrates using magickwand as Express middleware for CDN-style image resizing
// Dependencies (not included in magickwand):
//   npm install express mime

var parse = require('url').parse;
var magickwand = require('magickwand');
var mime = require('mime');
var util = require('util');

function errorResponse(res) {
  res.statusCode = 404;
  res.setHeader('Content-Length', 0);
  res.end('<h1>404 Not Found</h1>');
}

function cdnCache(options) {
  var config = {};

  config.path = '^/' + (options.path || 'cache');
  config.mime = options.mime || '^image/';
  config.srcPath = options.srcPath || "public/images/";
  config.fileTypes = options.fileTypes || ['.jpg', '.gif', '.png'];
  config.validSizes = options.validSizes;
  config.getParams = options.getParams || function (path) {
    var params = path.match('/' + options.path + '/([0-9]*)x([0-9]*)/(.*)');
    var result;
    var dims;
    var size;

    if (params && params.length === 4) {
      size = params[1] + 'x' + params[2];

      if ((config.fileTypes.some(function (elem) {
        return params[3].match(elem);
      })) && (!config.validSizes || config.validSizes.some(function (validSize) {
        return (validSize === size);
      }))) {
        result = {
          width: params[1],
          height: params[2],
          path: config.srcPath + params[3]
        };
      }
    }

    return result;
  };


  return function (req, res, next) {
    var pathname = parse(req.url).pathname;
    var resizeOpts;

    if (!pathname.match(config.path)) {
      return next();
    }

    resizeOpts = config.getParams(req.url);
    if (resizeOpts) {
      util.log('resizing image ' + req.url);
      // Modern API: pass options object instead of positional parameters
      magickwand.resize(resizeOpts.path, {
        width: parseInt(resizeOpts.width, 10),
        height: parseInt(resizeOpts.height, 10)
      }, function (err, data) {
        if (!err && data) {
          res.setHeader('Content-Type', mime.lookup(resizeOpts.path));
          res.setHeader('Content-Length', data.length);
          res.end(data, "binary");
        } else {
          errorResponse(res);
        }
      });
    } else {
      util.log('failed to resize image ' + req.url);
      errorResponse(res);
    }
  };
}

module.exports = cdnCache;
