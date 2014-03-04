#include "./magickwand.h"

typedef struct thumbnailReq {
  Persistent<Function> cb;
  unsigned char *resizedImage;
  char *exception;
  size_t resizedImageLen;

  bool autocrop;

  unsigned int width;
  unsigned int height;
  unsigned int quality;
  char imagefilepath[1];
} ThumbnailReq;

/* Resize image here */
static void thumbnail(uv_work_t *req) {
  ThumbnailReq *mgr = (ThumbnailReq *)req->data;
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

    MagickThumbnailImage(magick_wand, newImageWidth, newImageHeight);
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
      MagickThumbnailImage(magick_wand, mgr->width, mgr->height);
    }
  }

  if (mgr->quality)
    MagickSetImageCompressionQuality(magick_wand,mgr->quality);

  mgr->resizedImage = MagickGetImageBlob(magick_wand, &mgr->resizedImageLen);

  if (!mgr->resizedImage) {
    mgr->exception = MagickGetException(magick_wand, &severity);
  }

  DestroyMagickWand(magick_wand);
}

static void postThumbnail(uv_work_t *req) {
  HandleScope scope;
  ThumbnailReq *mgr = (ThumbnailReq *)req->data;

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
    Local<Integer> quality = Integer::New(mgr->quality);
    info->Set(String::NewSymbol("width"), width);
    info->Set(String::NewSymbol("height"), height);
    info->Set(String::NewSymbol("quality"), quality);
    argv[2] = info;
  }

  TryCatch try_catch;

  mgr->cb->Call(Context::GetCurrent()->Global(), 3, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  mgr->cb.Dispose();
  MagickRelinquishMemory(mgr->resizedImage);
  free(mgr);

  delete req;
}

Handle<Value> thumbnailAsync (const Arguments &args) {
  HandleScope scope;
  const char *usage = "Too few arguments: Usage: thumbnail(pathtoimgfile, width, height, quality, autocrop, cb)";
  int argc = 0;
  if (args.Length() != 5) {
    return ThrowException(Exception::Error(String::New(usage)));
  }

  String::Utf8Value name(args[argc++]);
  int width = args[argc++]->Int32Value();
  int height = args[argc++]->Int32Value();
  int quality = args[argc++]->Int32Value();
  bool autocrop = args[argc++]->BooleanValue();

  Local<Function> cb = Local<Function>::Cast(args[argc++]);

  if (width < 0 || height < 0) {
    return ThrowException(Exception::Error(String::New("Invalid width/height arguments")));
  }

  if (quality < 0 || quality > 100) {
    return ThrowException(Exception::Error(String::New("Invalid quality parameter")));
  }

  uv_work_t *req = new uv_work_t;
  ThumbnailReq *mgr = (ThumbnailReq *) calloc(1, sizeof(ThumbnailReq) + name.length());
  req->data = mgr;

  mgr->cb = Persistent<Function>::New(cb);
  mgr->width = width;
  mgr->height = height;
  mgr->quality = quality;
  mgr->autocrop = autocrop;

  strncpy(mgr->imagefilepath, *name, name.length() + 1);

  uv_queue_work(uv_default_loop(), req, thumbnail, (uv_after_work_cb)postThumbnail);

  return Undefined();
}
