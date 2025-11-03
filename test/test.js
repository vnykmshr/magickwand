"use strict";

var magickwand = require('../index');
var assert = require('assert');

var imagePath = __dirname + '/abc.jpg';

describe("magickwand", function() {
  describe("Non existent image", function() {
    it('resize should generate an error', function(done) {
      magickwand.resize('./nonexistent.jpg', { width: 128, quality: 80 }, function(err, data) {
        assert(err !== undefined);
        assert(data === undefined);
        done();
      });
    });

    it('thumbnail should generate an error', function(done) {
      magickwand.thumbnail('./nonexistent.jpg', { width: 128, quality: 80 }, function(err, data) {
        assert(err !== undefined);
        assert(err.message.indexOf('No such') > -1 || err.message.indexOf('Unable to open') > -1);
        assert(data === undefined);
        done();
      });
    });
  });

  describe("Invalid parameters", function() {
    it('resize with invalid width should throw error', function(done) {
      try {
        magickwand.resize(imagePath, { width: -10, height: 0 }, function(err, data) {
          assert(false, 'Should have thrown an error');
        });
      } catch(e) {
        assert(e !== undefined);
        assert(e.message.indexOf('Invalid width/height') > -1);
        done();
      }
    });

    it('resize with invalid quality should throw error', function(done) {
      try {
        magickwand.resize(imagePath, { width: 100, height: 100, quality: 150 }, function(err, data) {
          assert(false, 'Should have thrown an error');
        });
      } catch(e) {
        assert(e !== undefined);
        assert(e.message.indexOf('Invalid quality') > -1);
        done();
      }
    });
  });

  describe("Valid resize operations", function() {
    it("resize should produce valid image with both dimensions", function(done) {
      magickwand.resize(imagePath, { width: 50, height: 50 }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        assert(info.width === 50);
        assert(info.height === 50);
        done();
      });
    });

    it("resize should maintain aspect ratio when height is 0", function(done) {
      magickwand.resize(imagePath, { width: 100, height: 0 }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        assert(info.width === 100);
        assert(info.height > 0);
        done();
      });
    });

    it("resize should maintain aspect ratio when width is 0", function(done) {
      magickwand.resize(imagePath, { width: 0, height: 100 }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        assert(info.width > 0);
        assert(info.height === 100);
        done();
      });
    });

    it("resize with quality parameter should work", function(done) {
      magickwand.resize(imagePath, { width: 100, height: 100, quality: 50 }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        done();
      });
    });

    it("resize with autocrop should produce valid image", function(done) {
      magickwand.resize(imagePath, { width: 100, height: 100, autocrop: true }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        assert(info.width === 100);
        assert(info.height === 100);
        done();
      });
    });

    it("resize with format conversion should work", function(done) {
      magickwand.resize(imagePath, { width: 100, height: 100, format: 'png' }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        // PNG files start with 0x89504E47
        assert(data[0] === 0x89);
        assert(data[1] === 0x50);
        assert(data[2] === 0x4E);
        assert(data[3] === 0x47);
        done();
      });
    });
  });

  describe("Valid thumbnail operations", function() {
    it("thumbnail should produce valid image", function(done) {
      magickwand.thumbnail(imagePath, { width: 50, height: 50 }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        assert(info.width === 50);
        assert(info.height === 50);
        done();
      });
    });

    it("thumbnail should maintain aspect ratio when height is 0", function(done) {
      magickwand.thumbnail(imagePath, { width: 100, height: 0 }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        assert(info.width === 100);
        assert(info.height > 0);
        done();
      });
    });

    it("thumbnail with quality parameter should work", function(done) {
      magickwand.thumbnail(imagePath, { width: 100, height: 100, quality: 70 }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        assert(info.quality === 70);
        done();
      });
    });

    it("thumbnail with autocrop should produce valid image", function(done) {
      magickwand.thumbnail(imagePath, { width: 100, height: 100, autocrop: true }, function(err, data, info) {
        assert(err === undefined, err ? err.message : '');
        assert(data !== undefined);
        assert(data.length > 0);
        assert(info !== undefined);
        assert(info.width === 100);
        assert(info.height === 100);
        done();
      });
    });

    it("both thumbnail and resize should produce valid output for same dimensions", function(done) {
      var resizeLength, thumbnailLength;

      magickwand.resize(imagePath, { width: 100, height: 100, quality: 80 }, function(err, data) {
        assert(err === undefined);
        assert(data !== undefined);
        assert(data.length > 0);
        resizeLength = data.length;

        magickwand.thumbnail(imagePath, { width: 100, height: 100, quality: 80 }, function(err, data) {
          assert(err === undefined);
          assert(data !== undefined);
          assert(data.length > 0);
          thumbnailLength = data.length;
          // Both should produce valid output (size may vary)
          assert(resizeLength > 0 && thumbnailLength > 0);
          done();
        });
      });
    });
  });
});
