#include "./magickwand.h"

struct magickReq {
  Persistent<Function> cb;
  Isolate* isolate;
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

    MagickResizeImage(magick_wand, newImageWidth, newImageHeight, LanczosFilter);
    MagickCropImage(magick_wand, mgr->width, mgr->height, (newImageWidth - mgr->width) / 2, (newImageHeight - mgr->height) / 2);
    MagickResetImagePage(magick_wand, (const char *) NULL);

  }

  if (mgr->autocrop == false) {
    // If autcrop == false, we want to scale the image, only stretching if both height and width are set

    // Don't stretch, make the image fit within the given parameter
    if (mgr->width == 0 || mgr->height == 0) {
      if (mgr->width == 0) {
        mgr->width = (mgr->height * imageAspectRatio);
      } else if (mgr->height == 0) {
        mgr->height = (mgr->width / imageAspectRatio);
      }
    }

    if (mgr->width && mgr->height) {
      MagickResizeImage(magick_wand, mgr->width, mgr->height, LanczosFilter);
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
static void postResize(uv_work_t *req, int status) {
  struct magickReq *mgr = (struct magickReq *)req->data;
  Isolate* isolate = mgr->isolate;
  HandleScope scope(isolate);

  Local<Value> argv[3];
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> info = Object::New(isolate);

  if (mgr->exception) {
    argv[0] = Exception::Error(String::NewFromUtf8(isolate, mgr->exception).ToLocalChecked());
    argv[1] = Undefined(isolate);
    argv[2] = Undefined(isolate);
    MagickRelinquishMemory(mgr->exception);
  } else {
    argv[0] = Undefined(isolate);
    MaybeLocal<Object> buf = Buffer::Copy(isolate, (const char*)mgr->resizedImage, mgr->resizedImageLen);
    argv[1] = buf.ToLocalChecked();
    info->Set(context,
              String::NewFromUtf8(isolate, "width").ToLocalChecked(),
              Integer::New(isolate, mgr->width)).Check();
    info->Set(context,
              String::NewFromUtf8(isolate, "height").ToLocalChecked(),
              Integer::New(isolate, mgr->height)).Check();
    argv[2] = info;
  }

  Local<Function> cb = Local<Function>::New(isolate, mgr->cb);
  TryCatch try_catch(isolate);

  cb->Call(context, Null(isolate), 3, argv).IsEmpty();

  if (try_catch.HasCaught()) {
    node::FatalException(isolate, try_catch);
  }

  mgr->cb.Reset();
  MagickRelinquishMemory(mgr->resizedImage);
  if (mgr->format)
    free(mgr->format);
  free(mgr);

  delete req;
}

void resizeAsync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();

  const char *usage = "Too few arguments: Usage: resize(imagefile, width, height, quality, format, autocrop, cb)";
  if (args.Length() != 7) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, usage).ToLocalChecked()));
    return;
  }

  String::Utf8Value name(isolate, args[0]);
  String::Utf8Value format(isolate, args[4]);
  int width = args[1]->Int32Value(context).ToChecked();
  int height = args[2]->Int32Value(context).ToChecked();
  int quality = args[3]->Int32Value(context).ToChecked();

  bool autocrop = args[5]->BooleanValue(isolate);

  Local<Function> cb = Local<Function>::Cast(args[6]);

  if (width < 0 || height < 0) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid width/height arguments").ToLocalChecked()));
    return;
  }

  if (quality < 0 || quality > 100) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Invalid quality parameter").ToLocalChecked()));
    return;
  }

  uv_work_t *req = new uv_work_t;
  struct magickReq *mgr = (struct magickReq *) calloc(1, sizeof(struct magickReq) + name.length());
  req->data = mgr;

  mgr->cb.Reset(isolate, cb);
  mgr->isolate = isolate;
  mgr->width = width;
  mgr->height = height;
  mgr->quality = quality;
  mgr->autocrop = autocrop;

  if (*format)
    mgr->format = strdup(*format);

  strncpy(mgr->imagefilepath, *name, name.length() + 1);

  uv_queue_work(uv_default_loop(), req, resize, postResize);
}
