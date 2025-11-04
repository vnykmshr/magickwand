'use strict';

const magickwand = require('../index');
const assert = require('assert');
const fs = require('fs');
const path = require('path');

// Test fixtures
const imagePath = path.join(__dirname, 'abc.jpg');
const pngPath = path.join(__dirname, 'sample.png');
const corruptPath = path.join(__dirname, 'corrupt.jpg');

// ============================================================================
// Test Helpers
// ============================================================================

/**
 * Assert that an image operation succeeded with valid result
 */
function assertValidImageResult(err, data, info, expectedWidth, expectedHeight) {
  assert.strictEqual(err, undefined, err ? err.message : '');
  assert(data !== undefined, 'Image data should be returned');
  assert(data.length > 0, 'Image data should not be empty');
  assert(info !== undefined, 'Info object should be returned');

  if (expectedWidth !== undefined) {
    assert.strictEqual(info.width, expectedWidth, `Width should be ${expectedWidth}`);
  }
  if (expectedHeight !== undefined) {
    assert.strictEqual(info.height, expectedHeight, `Height should be ${expectedHeight}`);
  }
}

/**
 * Assert that an operation failed with expected error
 */
function assertErrorContains(err, data, expectedMessage) {
  assert(err !== undefined, 'Error should be returned');
  assert.strictEqual(data, undefined, 'Data should be undefined on error');
  assert(err.message.indexOf(expectedMessage) > -1,
    `Error message should contain "${expectedMessage}", got: ${err.message}`);
}

/**
 * Parameterized test runner for image operations
 */
function testImageOperation(operation, operationName, imagePath, options, validator) {
  return function(done) {
    operation(imagePath, options, function(err, data, info) {
      try {
        validator(err, data, info);
        done();
      } catch (e) {
        done(e);
      }
    });
  };
}

// ============================================================================
// Test Suite
// ============================================================================

describe('magickwand', function() {

  // ==========================================================================
  // Error Handling Tests
  // ==========================================================================

  describe('Error Handling', function() {

    describe('Non-existent files', function() {
      it('resize should return error for non-existent file', function(done) {
        magickwand.resize('./nonexistent.jpg', { width: 128, quality: 80 }, function(err, data) {
          assertErrorContains(err, data, 'Image file not found');
          done();
        });
      });

      it('thumbnail should return error for non-existent file', function(done) {
        magickwand.thumbnail('./nonexistent.jpg', { width: 128, quality: 80 }, function(err, data) {
          assertErrorContains(err, data, 'Image file not found');
          done();
        });
      });
    });

    describe('Invalid file types', function() {
      it('should reject corrupt/invalid image files', function(done) {
        magickwand.resize(corruptPath, { width: 100 }, function(err, data) {
          assert(err !== undefined, 'Should error on corrupt image');
          assert.strictEqual(data, undefined);
          done();
        });
      });

      it('should reject directory paths', function(done) {
        magickwand.resize(__dirname, { width: 100 }, function(err, data) {
          assertErrorContains(err, data, 'must be a file');
          done();
        });
      });
    });

    describe('Invalid parameters', function() {
      it('should throw error for negative width', function(done) {
        try {
          magickwand.resize(imagePath, { width: -10, height: 0 }, function(_err, _data) {
            assert.fail('Should have thrown an error');
          });
        } catch(e) {
          assert(e !== undefined);
          assert(e.message.indexOf('Invalid width/height') > -1);
          done();
        }
      });

      it('should throw error for negative height', function(done) {
        try {
          magickwand.resize(imagePath, { width: 100, height: -5 }, function(_err, _data) {
            assert.fail('Should have thrown an error');
          });
        } catch(e) {
          assert(e !== undefined);
          assert(e.message.indexOf('Invalid width/height') > -1);
          done();
        }
      });

      it('should throw error for invalid quality (> 100)', function(done) {
        try {
          magickwand.resize(imagePath, { width: 100, height: 100, quality: 150 }, function(_err, _data) {
            assert.fail('Should have thrown an error');
          });
        } catch(e) {
          assert(e !== undefined);
          assert(e.message.indexOf('Invalid quality') > -1);
          done();
        }
      });

      it('should throw error for empty options object', function(done) {
        try {
          magickwand.resize(imagePath, {}, function(_err, _data) {
            assert.fail('Should have thrown an error');
          });
        } catch(e) {
          assert(e !== undefined);
          assert(e.message.indexOf('Invalid options') > -1);
          done();
        }
      });
    });
  });

  // ==========================================================================
  // Security Tests (v1.0.0 Features)
  // ==========================================================================

  describe('Security - Path Validation', function() {

    it('should reject paths with null bytes', function(done) {
      magickwand.resize('image\0.jpg', { width: 100 }, function(err, data) {
        assertErrorContains(err, data, 'null byte');
        done();
      });
    });

    it('should handle non-string paths safely', function(done) {
      magickwand.resize(null, { width: 100 }, function(err, data) {
        assert(err !== undefined, 'Should error on null path');
        assert.strictEqual(data, undefined);
        done();
      });
    });

    it('should validate file is readable', function(done) {
      // Test assumes we can create an unreadable file temporarily
      // Skip if running as root or on systems where this isn't possible
      const unreadableFile = path.join(__dirname, 'unreadable.jpg');

      // Copy a valid file
      fs.copyFileSync(imagePath, unreadableFile);

      try {
        // Make it unreadable
        fs.chmodSync(unreadableFile, 0o000);

        magickwand.resize(unreadableFile, { width: 100 }, function(err, data) {
          // Restore permissions before asserting
          try { fs.chmodSync(unreadableFile, 0o644); } catch(_e) { /* ignore */ }
          try { fs.unlinkSync(unreadableFile); } catch(_e) { /* ignore */ }

          assert(err !== undefined, 'Should error on unreadable file');
          assert.strictEqual(data, undefined);
          done();
        });
      } catch(_permError) {
        // Cleanup and skip test if we can't change permissions
        try { fs.chmodSync(unreadableFile, 0o644); } catch(_e) { /* ignore */ }
        try { fs.unlinkSync(unreadableFile); } catch(_e) { /* ignore */ }
        this.skip();
      }
    });
  });

  describe('Security - Resource Limits', function() {

    it('should enforce default max dimension (16384px) for width', function(done) {
      magickwand.resize(imagePath, { width: 20000, height: 100 }, function(err, data) {
        assertErrorContains(err, data, 'exceeds maximum allowed dimension');
        done();
      });
    });

    it('should enforce default max dimension (16384px) for height', function(done) {
      magickwand.resize(imagePath, { width: 100, height: 20000 }, function(err, data) {
        assertErrorContains(err, data, 'exceeds maximum allowed dimension');
        done();
      });
    });

    it('should allow custom maxDimension override', function(done) {
      magickwand.resize(imagePath, {
        width: 500,
        height: 500,
        maxDimension: 1000
      }, function(err, data, info) {
        assertValidImageResult(err, data, info, 500, 500);
        done();
      });
    });

    it('should respect custom maxDimension when enforcing limits', function(done) {
      magickwand.resize(imagePath, {
        width: 300,
        maxDimension: 200  // Lower than requested width
      }, function(err, data) {
        assertErrorContains(err, data, 'exceeds maximum allowed dimension');
        done();
      });
    });

    it('thumbnail should also enforce resource limits', function(done) {
      magickwand.thumbnail(imagePath, { width: 20000 }, function(err, data) {
        assertErrorContains(err, data, 'exceeds maximum allowed dimension');
        done();
      });
    });
  });

  // ==========================================================================
  // Resize Operations
  // ==========================================================================

  describe('Resize Operations', function() {

    it('should resize to exact dimensions when both specified',
      testImageOperation(magickwand.resize, 'resize', imagePath,
        { width: 50, height: 50 },
        (err, data, info) => assertValidImageResult(err, data, info, 50, 50)
      )
    );

    it('should maintain aspect ratio when height is 0', function(done) {
      magickwand.resize(imagePath, { width: 100, height: 0 }, function(err, data, info) {
        assertValidImageResult(err, data, info, 100);
        assert(info.height > 0, 'Height should be calculated from aspect ratio');
        done();
      });
    });

    it('should maintain aspect ratio when width is 0', function(done) {
      magickwand.resize(imagePath, { width: 0, height: 100 }, function(err, data, info) {
        assertValidImageResult(err, data, info);
        assert(info.width > 0, 'Width should be calculated from aspect ratio');
        assert.strictEqual(info.height, 100);
        done();
      });
    });

    it('should apply quality parameter', function(done) {
      // Test that different quality levels produce different file sizes
      magickwand.resize(imagePath, { width: 100, height: 100, quality: 10 }, function(err, lowQualityData) {
        assert.strictEqual(err, undefined);

        magickwand.resize(imagePath, { width: 100, height: 100, quality: 95 }, function(err, highQualityData) {
          assert.strictEqual(err, undefined);

          // Higher quality should produce larger file (usually)
          assert(highQualityData.length > lowQualityData.length,
            'Higher quality should produce larger file size');
          done();
        });
      });
    });

    it('should handle autocrop correctly',
      testImageOperation(magickwand.resize, 'resize', imagePath,
        { width: 100, height: 100, autocrop: true },
        (err, data, info) => assertValidImageResult(err, data, info, 100, 100)
      )
    );

    it('should convert to PNG format', function(done) {
      magickwand.resize(imagePath, { width: 100, height: 100, format: 'png' }, function(err, data, info) {
        assertValidImageResult(err, data, info);
        // PNG files start with 0x89504E47
        assert.strictEqual(data[0], 0x89, 'PNG magic number byte 1');
        assert.strictEqual(data[1], 0x50, 'PNG magic number byte 2');
        assert.strictEqual(data[2], 0x4E, 'PNG magic number byte 3');
        assert.strictEqual(data[3], 0x47, 'PNG magic number byte 4');
        done();
      });
    });

    it('should handle PNG input files', function(done) {
      magickwand.resize(pngPath, { width: 100, height: 100 }, function(err, data, info) {
        assertValidImageResult(err, data, info, 100, 100);
        done();
      });
    });

    it('should convert PNG to JPEG', function(done) {
      magickwand.resize(pngPath, { width: 100, format: 'jpg' }, function(err, data, info) {
        assertValidImageResult(err, data, info, 100);
        // JPEG files start with 0xFFD8
        assert.strictEqual(data[0], 0xFF, 'JPEG magic number byte 1');
        assert.strictEqual(data[1], 0xD8, 'JPEG magic number byte 2');
        done();
      });
    });
  });

  // ==========================================================================
  // Thumbnail Operations
  // ==========================================================================

  describe('Thumbnail Operations', function() {

    it('should create thumbnail with exact dimensions',
      testImageOperation(magickwand.thumbnail, 'thumbnail', imagePath,
        { width: 50, height: 50 },
        (err, data, info) => assertValidImageResult(err, data, info, 50, 50)
      )
    );

    it('should maintain aspect ratio when height is 0', function(done) {
      magickwand.thumbnail(imagePath, { width: 100, height: 0 }, function(err, data, info) {
        assertValidImageResult(err, data, info, 100);
        assert(info.height > 0, 'Height should be calculated from aspect ratio');
        done();
      });
    });

    it('should apply and report quality parameter', function(done) {
      magickwand.thumbnail(imagePath, { width: 100, height: 100, quality: 70 }, function(err, data, info) {
        assertValidImageResult(err, data, info);
        assert.strictEqual(info.quality, 70, 'Should report quality in info object');
        done();
      });
    });

    it('should handle autocrop correctly',
      testImageOperation(magickwand.thumbnail, 'thumbnail', imagePath,
        { width: 100, height: 100, autocrop: true },
        (err, data, info) => assertValidImageResult(err, data, info, 100, 100)
      )
    );

    it('should work with optional options parameter', function(done) {
      // thumbnail(imagePath, callback) - no options
      magickwand.thumbnail(imagePath, function(err, data, info) {
        assertValidImageResult(err, data, info);
        done();
      });
    });

    it('should produce smaller files than resize for same dimensions', function(done) {
      const options = { width: 100, height: 100, quality: 80 };

      magickwand.resize(imagePath, options, function(err, resizeData) {
        assert.strictEqual(err, undefined);

        magickwand.thumbnail(imagePath, options, function(err, thumbnailData) {
          assert.strictEqual(err, undefined);

          // Thumbnail should generally produce smaller or equal file sizes
          // (this is the optimization benefit of using MagickThumbnailImage)
          assert(thumbnailData.length <= resizeData.length * 1.1,
            'Thumbnail should not be significantly larger than resize');
          done();
        });
      });
    });
  });

  // ==========================================================================
  // Edge Cases
  // ==========================================================================

  describe('Edge Cases', function() {

    it('should handle very small dimensions (1x1)', function(done) {
      magickwand.resize(imagePath, { width: 1, height: 1 }, function(err, data, info) {
        assertValidImageResult(err, data, info, 1, 1);
        done();
      });
    });

    it('should handle large but valid dimensions', function(done) {
      magickwand.resize(imagePath, { width: 2000, height: 2000 }, function(err, data, info) {
        assertValidImageResult(err, data, info, 2000, 2000);
        done();
      });
    });

    it('should handle quality at boundaries (1)', function(done) {
      magickwand.resize(imagePath, { width: 100, quality: 1 }, function(err, data, info) {
        assertValidImageResult(err, data, info);
        done();
      });
    });

    it('should handle quality at boundaries (100)', function(done) {
      magickwand.resize(imagePath, { width: 100, quality: 100 }, function(err, data, info) {
        assertValidImageResult(err, data, info);
        done();
      });
    });

    it('should handle autocrop with only width specified', function(done) {
      magickwand.resize(imagePath, { width: 100, autocrop: true }, function(err, data, info) {
        assertValidImageResult(err, data, info);
        // When only width is set with autocrop, it should create a square
        assert.strictEqual(info.width, info.height, 'Should create square when one dimension + autocrop');
        done();
      });
    });

    it('should handle autocrop with only height specified', function(done) {
      magickwand.resize(imagePath, { height: 100, autocrop: true }, function(err, data, info) {
        assertValidImageResult(err, data, info);
        // When only height is set with autocrop, it should create a square
        assert.strictEqual(info.width, info.height, 'Should create square when one dimension + autocrop');
        done();
      });
    });
  });
});
