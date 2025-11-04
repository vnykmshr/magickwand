'use strict';

// This example demonstrates using magickwand as Express middleware for CDN-style image resizing
// Dependencies (not included in magickwand):
//   npm install express mime

const {parse} = require('url');
const magickwand = require('magickwand');
const mime = require('mime');
const {log} = require('util');

function errorResponse(res) {
  res.statusCode = 404;
  res.setHeader('Content-Length', 0);
  res.end('<h1>404 Not Found</h1>');
}

function cdnCache(options) {
  const config = {
    path: `^/${options.path || 'cache'}`,
    mime: options.mime || '^image/',
    srcPath: options.srcPath || 'public/images/',
    fileTypes: options.fileTypes || ['.jpg', '.gif', '.png'],
    validSizes: options.validSizes,
    getParams: options.getParams || ((path) => {
      const params = path.match(`/${options.path}/([0-9]*)x([0-9]*)/(.*)`);

      if (!params || params.length !== 4) {
        return null;
      }

      const size = `${params[1]}x${params[2]}`;
      const hasValidFileType = config.fileTypes.some(elem => params[3].match(elem));
      const hasValidSize = !config.validSizes || config.validSizes.some(validSize => validSize === size);

      if (hasValidFileType && hasValidSize) {
        return {
          width: params[1],
          height: params[2],
          path: config.srcPath + params[3]
        };
      }

      return null;
    })
  };

  return (req, res, next) => {
    const pathname = parse(req.url).pathname;

    if (!pathname.match(config.path)) {
      return next();
    }

    const resizeOpts = config.getParams(req.url);

    if (resizeOpts) {
      log(`resizing image ${req.url}`);

      magickwand.resize(resizeOpts.path, {
        width: parseInt(resizeOpts.width, 10),
        height: parseInt(resizeOpts.height, 10)
      }, (err, data) => {
        if (!err && data) {
          res.setHeader('Content-Type', mime.lookup(resizeOpts.path));
          res.setHeader('Content-Length', data.length);
          res.end(data, 'binary');
        } else {
          errorResponse(res);
        }
      });
    } else {
      log(`failed to resize image ${req.url}`);
      errorResponse(res);
    }
  };
}

module.exports = cdnCache;
