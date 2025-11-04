#include "./magickwand.h"

/**
 * Common image processing logic shared between resize and thumbnail
 * @param magick_wand The MagickWand instance with image loaded
 * @param width Target width (0 = auto)
 * @param height Target height (0 = auto)
 * @param autocrop Enable cropping
 * @param useThumbnail Use MagickThumbnailImage instead of MagickResizeImage
 * @returns true on success, false on failure
 */
bool processImageDimensions(MagickWand *magick_wand, unsigned int *width, unsigned int *height, bool autocrop, bool useThumbnail) {
  float imageWidth, imageHeight, newImageWidth, newImageHeight, imageAspectRatio, canvasAspectRatio;

  // Get the image sizes
  imageWidth = MagickGetImageWidth(magick_wand);
  imageHeight = MagickGetImageHeight(magick_wand);
  imageAspectRatio = (imageWidth * 1.0) / imageHeight;

  // If autocrop == true, we want to scale the image, keeping proportions and then crop
  if (autocrop) {

    // If these are not set, we will remove the autocrop and let the code further down handle the rest
    if (*width == 0 && *height == 0) {
      autocrop = false;

    } else if (*width == 0 || *height == 0) {

      // If one of canvas height or width is not set, we will assume a square is wanted
      if (*width == 0) {
        *width = *height;
      } else if (*height == 0) {
        *height = *width;
      }

    }

    canvasAspectRatio = (*width * 1.0) / *height;

    if (imageAspectRatio < canvasAspectRatio) {
      newImageWidth = *width;
      newImageHeight = newImageWidth / imageAspectRatio;
    } else {
      newImageHeight = *height;
      newImageWidth = newImageHeight * imageAspectRatio;
    }

    if (useThumbnail) {
      MagickThumbnailImage(magick_wand, newImageWidth, newImageHeight);
    } else {
      ResizeImage(magick_wand, newImageWidth, newImageHeight, LanczosFilter);
    }

    MagickCropImage(magick_wand, *width, *height, (newImageWidth - *width) / 2, (newImageHeight - *height) / 2);
    MagickResetImagePage(magick_wand, (const char *) NULL);

  }

  if (autocrop == false) {
    // If autocrop == false, we want to scale the image, only stretching if both height and width are set

    // Don't stretch, make the image fit within the given parameter
    if (*width == 0 || *height == 0) {
      if (*width == 0) {
        *width = (*height * imageAspectRatio);
      } else if (*height == 0) {
        *height = (*width / imageAspectRatio);
      }
    }

    if (*width && *height) {
      if (useThumbnail) {
        MagickThumbnailImage(magick_wand, *width, *height);
      } else {
        ResizeImage(magick_wand, *width, *height, LanczosFilter);
      }
    }
  }

  return true;
}

/**
 * Build callback arguments for resize/thumbnail operations
 * @param isolate V8 isolate
 * @param exception Error message or NULL
 * @param resizedImage Image data buffer
 * @param resizedImageLen Buffer length
 * @param width Result width
 * @param height Result height
 * @param quality Result quality (0 if not applicable)
 * @param argv Output array of 3 callback arguments
 */
void buildCallbackArgs(Isolate* isolate, char* exception, unsigned char* resizedImage, size_t resizedImageLen,
                       unsigned int width, unsigned int height, unsigned int quality, Local<Value> argv[3]) {
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> info = Object::New(isolate);

  if (exception) {
    // Sanitize error message - don't expose full paths
    argv[0] = Exception::Error(String::NewFromUtf8(isolate, "Image processing failed").ToLocalChecked());
    argv[1] = Undefined(isolate);
    argv[2] = Undefined(isolate);
    MagickRelinquishMemory(exception);
  } else {
    argv[0] = Undefined(isolate);
    MaybeLocal<Object> buf = Buffer::Copy(isolate, (const char*)resizedImage, resizedImageLen);
    argv[1] = buf.ToLocalChecked();
    info->Set(context,
              String::NewFromUtf8(isolate, "width").ToLocalChecked(),
              Integer::New(isolate, width)).Check();
    info->Set(context,
              String::NewFromUtf8(isolate, "height").ToLocalChecked(),
              Integer::New(isolate, height)).Check();
    if (quality > 0) {
      info->Set(context,
                String::NewFromUtf8(isolate, "quality").ToLocalChecked(),
                Integer::New(isolate, quality)).Check();
    }
    argv[2] = info;
  }
}
