{
  "targets": [
    {
      "target_name": "magickwand",
       "sources": [ "src/module.cpp", "src/resize.cpp", "src/thumbnail.cpp", "src/common.cpp", "src/rotate.cpp" ],
       'libraries': [ '<!@(pkg-config --libs MagickWand)' ],
       'include_dirs': ["<!(node -e \"require('nan')\")"],
       "conditions": [
        ['OS=="mac"', {
          # cflags on OS X are stupid and have to be defined like this
          'xcode_settings': {
            'OTHER_CFLAGS': [
              '<!@(pkg-config --cflags MagickWand)'
            ],
          }
        }, {
          'cflags': [
            '<!@(pkg-config --cflags MagickWand)'
          ]
        }]
      ]
    }
  ]
}
