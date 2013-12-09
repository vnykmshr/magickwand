#include "./magickwand.h"

struct magickReq {
  Persistent<Function> cb;
  unsigned char *resizedImage;
  char *exception;
  char *format;
  size_t resizedImageLen;
  int quality;

  bool autocrop;

  unsigned int width;
  unsigned int height;
  char imagefilepath[1];
};

/* Resize image here */
static void resize (uv_work_t *req) {
  struct magickReq *mgr = (struct magickReq *)req->data;
  ExceptionType severity;
  MagickWand *magick_wand = NewMagickWand();
  MagickBooleanType status;
  float imageWidth, imageHeight, newImageWidth, newImageHeight, imageAspectRatio, canvasAspectRatio;

  status = MagickReadImage(magick_wand, mgr->imagefilepath);

  if (status == MagickFalse) {
    mgr->exception = MagickGetException(magick_wand, &severity);
    DestroyMagickWand(magick_wand);
    return;
  }

  // Get the image sizes
  imageWidth = MagickGetImageWidth(magick_wand);
  imageHeight = MagickGetImageHeight(magick_wand);
  imageAspectRatio = (imageWidth * 1.0) / imageHeight;

  // If autcrop == true, we want to scale the image, keeping proportions and then crop
  if (mgr->autocrop) {

    // If these are not set, we will remove the autocrop and let the code further down handle the rest
    if (mgr->width == 0 && mgr->height == 0) {
      mgr->autocrop = false;

    } else if (mgr->width == 0 || mgr->height == 0) {

      // If one of canvas height or width is not set, we will assume a square is wanted
      if (mgr->width == 0) {
        mgr->width = mgr->height;
      } else if (mgr->height == 0) {
        mgr->height = mgr->width;
      }

    }

    canvasAspectRatio = (mgr->width * 1.0) / mgr->height;

    if (imageAspectRatio < canvasAspectRatio) {
      newImageWidth = mgr->width;
      newImageHeight = newImageWidth / imageAspectRatio;
    } else {
      newImageHeight = mgr->height;
      newImageWidth = newImageHeight * imageAspectRatio;
    }

    MagickResizeImage(magick_wand, newImageWidth, newImageHeight, LanczosFilter, 1.0);
    MagickCropImage(magick_wand, mgr->width, mgr->height, (newImageWidth - mgr->width) / 2, (newImageHeight - mgr->height) / 2);
    MagickResetImagePage(magick_wand, (const char *) NULL);

  }

  if (mgr->autocrop == false) {
    // If autcrop == false, we want to scale the image, only stretching if both height and width are set

    // Don't stretch, make the image fit within the given parameter
    if (!mgr->width == 0 || !mgr->height == 0) {
      if (mgr->width == 0) {
        mgr->width = (mgr->height * imageAspectRatio);
      } else if (mgr->height == 0) {
        mgr->height = (mgr->width / imageAspectRatio);
      }
    }

    if (mgr->width && mgr->height) {
      MagickResizeImage(magick_wand, mgr->width, mgr->height, LanczosFilter, 1.0);
    }
  }

  if (mgr->format) {
    MagickSetImageFormat(magick_wand, mgr->format);
  }

  if (mgr->quality) {
    MagickSetImageCompressionQuality(magick_wand, mgr->quality);
  }

  mgr->resizedImage = MagickGetImageBlob(magick_wand, &mgr->resizedImageLen);
  if (!mgr->resizedImage) {
    mgr->exception = MagickGetException(magick_wand, &severity);
  }

  DestroyMagickWand(magick_wand);
}

/* this is for the callback */
static void postResize(uv_work_t *req) {
  HandleScope scope;
  struct magickReq *mgr = (struct magickReq *)req->data;

  Handle<Value> argv[3];
  Local<Object> info = Object::New();

  if (mgr->exception) {
    argv[0] = Exception::Error(String::New(mgr->exception));
    argv[1] = argv[2] = Undefined();
    MagickRelinquishMemory(mgr->exception);
  } else {
    argv[0] = Undefined();
    Buffer *buf = Buffer::New(mgr->resizedImageLen + 1);
    memcpy(Buffer::Data(buf), mgr->resizedImage, mgr->resizedImageLen);
    argv[1] = buf->handle_;
    Local<Integer> width = Integer::New(mgr->width);
    Local<Integer> height = Integer::New(mgr->height);
    info->Set(String::NewSymbol("width"), width);
    info->Set(String::NewSymbol("height"), height);
    argv[2] = info;
  }

  TryCatch try_catch;

  mgr->cb->Call(Context::GetCurrent()->Global(), 3, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  mgr->cb.Dispose();
  MagickRelinquishMemory(mgr->resizedImage);
  if (mgr->format)
    free(mgr->format);
  free(mgr);

  delete req;
}

Handle<Value> resizeAsync (const Arguments &args) {
  HandleScope scope;
  const char *usage = "Too few arguments: Usage: resize(imagefile, width, height, quality, format, autocrop, cb)";
  if (args.Length() != 7) {
    return ThrowException(Exception::Error(String::New(usage)));
  }

  String::Utf8Value name(args[0]);
  String::Utf8Value format(args[4]);
  int width = args[1]->Int32Value();
  int height = args[2]->Int32Value();
  int quality = args[3]->Int32Value();

  bool autocrop = args[5]->BooleanValue();

  Local<Function> cb = Local<Function>::Cast(args[6]);

  if (width < 0 || height < 0) {
    return ThrowException(Exception::Error(String::New("Invalid width/height arguments")));
  }

  if (quality < 0 || quality > 100) {
    return ThrowException(Exception::Error(String::New("Invalid quality parameter")));
  }

  uv_work_t *req = new uv_work_t;
  struct magickReq *mgr = (struct magickReq *) calloc(1, sizeof(struct magickReq) + name.length());
  req->data = mgr;

  mgr->cb = Persistent<Function>::New(cb);
  mgr->width = width;
  mgr->height = height;
  mgr->quality = quality;
  mgr->autocrop = autocrop;

  if (*format)
    mgr->format = strdup(*format);

  strncpy(mgr->imagefilepath, *name, name.length() + 1);

  uv_queue_work(uv_default_loop(), req, resize, (uv_after_work_cb)postResize);

  return Undefined();
}
