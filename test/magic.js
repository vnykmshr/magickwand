"use strict";

var fs = require('fs');
var magickwand = require('../index');

// Define the input image path (assuming /tmp/abc.jpg exists as per original test)
var inputImagePath = '/tmp/abc.jpg';
// For buffer tests, we'll read this file.
// In a real test suite, we'd ensure this file exists or create a dummy one.
// For now, we proceed assuming it exists. If readFileSync fails, tests will indicate this.
var inputImageBuffer;
try {
  inputImageBuffer = fs.readFileSync(inputImagePath);
  console.log("Successfully read input image for buffer tests: " + inputImagePath);
} catch (e) {
  console.error("COULD NOT READ TEST IMAGE: " + inputImagePath + ". Some tests will fail.", e);
  // Fallback to an empty buffer to prevent crashes, though tests will be meaningless.
  inputImageBuffer = Buffer.alloc(0); 
}

console.log("--- Starting Resize Tests (File Input) ---");
magickwand.resize(inputImagePath, {
  width: 1000,
  quality: 80
}, function (err, data, info) {
  console.log("Resize Test 1 (File Input):");
  if (err) {
    console.error("Error:", err);
  } else {
    console.log("Blob returned, length: " + data.length);
    console.log("Info:", info);
    fs.writeFile('/tmp/resized_file_1.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  }
});

magickwand.resize(inputImagePath, {
  width: 200,
  height: 300,
  autocrop: true
}, function (err, data, info) {
  console.log("Resize Test 2 (File Input, Autocrop):");
  if (err) {
    console.error("Error:", err);
  } else {
    console.log("Blob returned, length: " + data.length);
    console.log("Info:", info);
    fs.writeFile('/tmp/resized_file_autocrop.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  }
});


console.log("\n--- Starting Thumbnail Tests (File Input) ---");
magickwand.thumbnail(inputImagePath, {
  width: 126
}, function (err, data, info) {
  console.log("Thumbnail Test 1 (File Input):");
  if (err) {
    console.error("Error:", err);
  } else {
    console.log("Blob returned, length: " + data.length);
    console.log("Info:", info);
    fs.writeFile('/tmp/thumb_file_1.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  }
});

magickwand.thumbnail(inputImagePath, {
  height: 100,
  autocrop: true
}, function (err, data, info) {
  console.log("Thumbnail Test 2 (File Input, Autocrop):");
  if (err) {
    console.error("Error:", err);
  } else {
    console.log("Blob returned, length: " + data.length);
    console.log("Info:", info);
    fs.writeFile('/tmp/thumb_file_autocrop.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  }
});


console.log("\n--- Starting Rotation Tests (File Input) ---");
magickwand.rotate(inputImagePath, { degrees: 90 }, function(err, data, info) {
  console.log("Rotate Test 1 (File Input, 90 degrees):");
  if (err) {
    console.error("Error:", err);
  } else {
    console.log("Blob returned, length: " + data.length);
    console.log("Info:", info);
    fs.writeFile('/tmp/rotated_file_90.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  }
});

magickwand.rotate(inputImagePath, { degrees: -45 }, function(err, data, info) {
  console.log("Rotate Test 2 (File Input, -45 degrees):");
  if (err) {
    console.error("Error:", err);
  } else {
    console.log("Blob returned, length: " + data.length);
    console.log("Info:", info);
    fs.writeFile('/tmp/rotated_file_neg45.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
  }
});

// Test with invalid degrees (string instead of number)
magickwand.rotate(inputImagePath, { degrees: "90" }, function(err, data, info) {
  console.log("Rotate Test 3 (File Input, Invalid Degrees Type):");
  if (err) {
    console.error("Error (expected for invalid degrees type):", err.message); // err.message for cleaner log
  } else {
    console.error("FAIL: Expected an error for invalid degrees type, but got success.");
    console.log("Blob returned, length: " + data.length);
    console.log("Info:", info);
  }
});


if (inputImageBuffer.length > 0) {
  console.log("\n--- Starting Resize Tests (Buffer Input) ---");
  magickwand.resize(inputImageBuffer, {
    width: 150,
    quality: 70,
    format: 'png' // Test format conversion with buffer
  }, function (err, data, info) {
    console.log("Resize Test 3 (Buffer Input, to PNG):");
    if (err) {
      console.error("Error:", err);
    } else {
      console.log("Blob returned (PNG), length: " + data.length);
      console.log("Info:", info);
      fs.writeFile('/tmp/resized_buffer_1.png', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
    }
  });

  console.log("\n--- Starting Thumbnail Tests (Buffer Input) ---");
  magickwand.thumbnail(inputImageBuffer, {
    width: 50,
    autocrop: true
  }, function (err, data, info) {
    console.log("Thumbnail Test 3 (Buffer Input, Autocrop):");
    if (err) {
      console.error("Error:", err);
    } else {
      console.log("Blob returned, length: " + data.length);
      console.log("Info:", info);
      fs.writeFile('/tmp/thumb_buffer_autocrop.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
    }
  });

  console.log("\n--- Starting Rotation Tests (Buffer Input) ---");
  magickwand.rotate(inputImageBuffer, { degrees: 180 }, function(err, data, info) {
    console.log("Rotate Test 4 (Buffer Input, 180 degrees):");
    if (err) {
      console.error("Error:", err);
    } else {
      console.log("Blob returned, length: " + data.length);
      console.log("Info:", info);
      fs.writeFile('/tmp/rotated_buffer_180.jpg', data, "binary", function(e){ if(e) console.error("Write failed:",e);});
    }
  });

} else {
  console.warn("\n--- SKIPPING BUFFER INPUT TESTS due to missing or empty input image ---");
}

// Add a small delay to allow async operations to complete before test script finishes.
// This is a simple way for this test style; proper test runners handle this.
setTimeout(function() {
  console.log("\n--- Test Script Finished ---");
}, 5000); // 5 seconds delay
