#include "./magickwand.h" // This now includes nan.h
// #include <cstring> // For strdup, strlen, etc. - Already in magickwand.h
// #include <cstdlib> // For calloc, free - Already in magickwand.h

// processImage function remains largely the same as it's C++/ImageMagick logic.
// magickReq struct is defined in magickwand.h
void processImage(uv_work_t *req, bool isThumbnail, bool isRotate) { // Added isRotate
  magickReq *mgr = (magickReq *)req->data;
  ExceptionType severity;
  MagickWand *magick_wand = NewMagickWand();
  MagickBooleanType status = MagickFalse; // Initialize status
  float imageWidth, imageHeight, newImageWidth, newImageHeight, imageAspectRatio, canvasAspectRatio;

  // Read image from buffer or filepath
  if (mgr->inputBuffer && mgr->inputBufferLen > 0) {
    status = MagickReadImageBlob(magick_wand, mgr->inputBuffer, mgr->inputBufferLen);
  } else if (mgr->imagefilepath && strlen(mgr->imagefilepath) > 0) {
    status = MagickReadImage(magick_wand, mgr->imagefilepath);
  } else {
    // No valid input provided
    mgr->exception = strdup("No image filepath or input buffer provided.");
    DestroyMagickWand(magick_wand);
    return;
  }

  if (status == MagickFalse) {
    char *magick_error = MagickGetException(magick_wand, &severity);
    if (magick_error) {
      mgr->exception = strdup(magick_error);
      MagickRelinquishMemory(magick_error);
    } else {
      mgr->exception = strdup("Unknown ImageMagick error during MagickReadImage.");
    }
    DestroyMagickWand(magick_wand);
    return;
  }

  // Apply rotation if specified
  if (isRotate && mgr->degrees != 0.0) {
    PixelWand *background = NewPixelWand();
    // TODO: Make background color configurable, transparent for formats that support it.
    // Using white for now, as it's a common default. For PNG/GIF, "none" (transparent) is better.
    PixelSetColor(background, "white"); 
    MagickBooleanType rotate_status = MagickRotateImage(magick_wand, background, mgr->degrees);
    DestroyPixelWand(background);
    if (rotate_status == MagickFalse) {
      // If rotation fails, capture exception and return
      char *magick_error = MagickGetException(magick_wand, &severity);
      if (magick_error) {
        mgr->exception = strdup(magick_error);
        MagickRelinquishMemory(magick_error);
      } else {
        mgr->exception = strdup("Unknown error during MagickRotateImage.");
      }
      DestroyMagickWand(magick_wand);
      return;
    }
    // Update width and height in mgr after rotation
    mgr->width = MagickGetImageWidth(magick_wand);
    mgr->height = MagickGetImageHeight(magick_wand);
  }

  imageWidth = MagickGetImageWidth(magick_wand); // This will be the width after rotation if rotation happened
  imageHeight = MagickGetImageHeight(magick_wand); // This will be the height after rotation if rotation happened
  imageAspectRatio = (imageWidth * 1.0) / imageHeight;

  if (mgr->autocrop) {
    if (mgr->width == 0 && mgr->height == 0) {
      mgr->autocrop = false;
    } else if (mgr->width == 0 || mgr->height == 0) {
      if (mgr->width == 0) {
        mgr->width = mgr->height;
      } else if (mgr->height == 0) {
        mgr->height = mgr->width;
      }
    }

    if (mgr->autocrop) { // Check again as it might have been set to false
        canvasAspectRatio = (mgr->width * 1.0) / mgr->height;

        if (imageAspectRatio < canvasAspectRatio) {
          newImageWidth = mgr->width;
          newImageHeight = newImageWidth / imageAspectRatio;
        } else {
          newImageHeight = mgr->height;
          newImageWidth = newImageHeight * imageAspectRatio;
        }

        if (isThumbnail) {
            // For thumbnail with autocrop, ImageMagick's MagickThumbnailImage often does a better job
            // if used before manual cropping, as it might strip metadata and optimize.
            // However, the original logic was to resize then crop. To maintain behavior:
            // MagickResizeImage(magick_wand, newImageWidth, newImageHeight, LanczosFilter, 1.0);
            // Or, if MagickThumbnailImage is preferred for this step (might be more efficient):
            MagickThumbnailImage(magick_wand, newImageWidth, newImageHeight);

        } else {
            MagickResizeImage(magick_wand, newImageWidth, newImageHeight, LanczosFilter, 1.0);
        }
        MagickCropImage(magick_wand, mgr->width, mgr->height, (newImageWidth - mgr->width) / 2, (newImageHeight - mgr->height) / 2);
        MagickResetImagePage(magick_wand, (const char *) NULL); // Important after crop
    }
  }

  if (mgr->autocrop == false) { // Standard resize/thumbnail without cropping
    // If one dimension is 0, calculate it to maintain aspect ratio
    if (mgr->width == 0 && mgr->height != 0) {
        mgr->width = (mgr->height * imageAspectRatio);
    } else if (mgr->height == 0 && mgr->width != 0) {
        mgr->height = (mgr->width / imageAspectRatio);
    }
    // If both are still 0 (or were initially 0), this means no resize, use original dimensions
    // However, MagickResizeImage/MagickThumbnailImage with 0,0 might behave unexpectedly or error.
    // It's better to use original dimensions if no target width/height is specified.
    // But the original code implies if both are 0, no resize happens IF not stretching.
    // This part needs to be careful: if width/height are not set, image might not be changed.
    // Forcing a resize to original dimensions if no operation happened yet can be an option,
    // or rely on MagickGetImageBlob to get the original image if no resize ops were effective.

    if (mgr->width > 0 && mgr->height > 0) { // Only resize if width and height are specified
        if (isThumbnail) {
            MagickThumbnailImage(magick_wand, mgr->width, mgr->height);
        } else {
            MagickResizeImage(magick_wand, mgr->width, mgr->height, LanczosFilter, 1.0);
        }
    }
  }

  if (!isThumbnail && mgr->format && strlen(mgr->format) > 0) {
    MagickSetImageFormat(magick_wand, mgr->format);
  }

  if (mgr->quality > 0 && mgr->quality <= 100) {
    MagickSetImageCompressionQuality(magick_wand, mgr->quality);
  } else if (mgr->quality == 0 && !isThumbnail) {
    // For resize, quality 0 might mean "auto" or default. ImageMagick handles this.
    // For thumbnail, quality 0 was explicitly passed. If it means "default", it's fine.
    // Let's assume quality 0 is handled by ImageMagick appropriately.
    // If specific handling for quality=0 is needed, it can be added here.
    // For example, MagickSetImageCompressionQuality(magick_wand, 75); // a default
  }


  mgr->resizedImage = MagickGetImageBlob(magick_wand, &mgr->resizedImageLen);
  if (!mgr->resizedImage || mgr->resizedImageLen == 0) {
    // If there was already an exception, don't overwrite it.
    if (!mgr->exception) {
      char *magick_error = MagickGetException(magick_wand, &severity);
      if (magick_error) {
        mgr->exception = strdup(magick_error);
        MagickRelinquishMemory(magick_error);
      } else {
        mgr->exception = strdup("Unknown error getting image blob or image is empty.");
      }
    }
  }

  DestroyMagickWand(magick_wand);
  // Ensure mgr->exception is null-terminated if strdup failed, though strdup itself should handle this.
  // However, if strdup returns NULL due to OOM, mgr->exception would be NULL.
  // The callback functions should handle mgr->exception being NULL (meaning no error string available).
}
