#ifndef _MAGICKWAND_H
#define _MAGICKWAND_H

#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <nan.h> // Include NAN
#include <wand/MagickWand.h> // ImageMagick
#include <cstring> // For strdup, strlen, etc.
#include <cstdlib> // For calloc, free

// Common structure for request data
struct magickReq {
  Nan::Persistent<v8::Function> cb; // NAN persistent callback
  unsigned char *resizedImage;
  char *exception;
  char *format; // Used by resize, can be NULL for thumbnail
  size_t resizedImageLen;
  int quality;

  bool autocrop;

  unsigned int width;
  unsigned int height;

  // Input can be a filepath or a buffer
  char *imagefilepath; // Now a pointer, can be NULL if buffer is used
  const unsigned char* inputBuffer;
  size_t inputBufferLen;
  // Note: The flexible array member trick for imagefilepath is removed.
  // imagefilepath will need to be allocated separately if used.
  // The magickReq struct itself will be allocated with enough space,
  // and then imagefilepath can point within that extra space or to separate memory.
  // For simplicity with current calloc, we'll assume imagefilepath points into extra space
  // allocated after the struct, or is NULL.
  // char imagefilepath_data[1]; // This would be part of the flexible allocation if only path was supported.
  double degrees; // For rotation
};
// When allocating magickReq: sizeof(struct magickReq) + (filepath ? strlen(filepath)+1 : 0)
// And then if filepath, mgr->imagefilepath = ((char*)mgr) + sizeof(struct magickReq);

// NAN Method signatures
NAN_METHOD(resizeAsync);
NAN_METHOD(thumbnailAsync);
NAN_METHOD(rotateAsync); // New method for rotation

// Declaration for the common processing function
// uv_work_t is part of libuv, which NAN includes.
// Adding degrees to processImage, or creating a new one. Let's try adding for now.
void processImage(uv_work_t *req, bool isThumbnail, bool isRotate); // Added isRotate flag

// Module initialization (not typically in .h but kept for reference if used by old module.cpp)
// extern "C" void init(v8::Local<v8::Object> exports); // Updated signature

#endif /* _MAGICKWAND_H */
