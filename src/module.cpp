#include <nan.h> // Required for NAN macros
#include "./magickwand.h" // Should already include nan.h

// Module initialization using NAN
NAN_MODULE_INIT(InitAll) {
  // Perform MagickWandGenesis initialization once.
  // It's safe to call this multiple times, but once is sufficient.
  MagickWandGenesis();

  // Export methods
  NAN_EXPORT(target, resizeAsync);
  NAN_EXPORT(target, thumbnailAsync);
  NAN_EXPORT(target, rotateAsync); // Add rotateAsync
}

// Entry point for the Node.js addon
// Module name "magickwand", Initializer function "InitAll"
NODE_MODULE(magickwand, InitAll)
