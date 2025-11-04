{
  "targets": [
    {
      "target_name": "magickwand",
      "sources": [
        "src/module.cpp",
        "src/common.cpp",
        "src/resize.cpp",
        "src/thumbnail.cpp"
      ],
      "conditions": [
        ['OS=="win"', {
          "variables": {
            "MAGICK_ROOT%": "<!(python tools/get_magick_root.py)"
          },
          "libraries": [
            "<(MAGICK_ROOT)/lib/CORE_RL_MagickCore_.lib",
            "<(MAGICK_ROOT)/lib/CORE_RL_MagickWand_.lib"
          ],
          "include_dirs": [
            "<(MAGICK_ROOT)/include"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalIncludeDirectories": [
                "<(MAGICK_ROOT)/include"
              ],
              "DisableSpecificWarnings": [
                "4251",  # class needs to have dll-interface
                "4275"   # non dll-interface class used as base
              ]
            },
            "VCLinkerTool": {
              "AdditionalLibraryDirectories": [
                "<(MAGICK_ROOT)/lib"
              ]
            }
          },
          "conditions": [
            ['target_arch!="x64"', {
              "defines": ["_SSIZE_T_"]
            }]
          ]
        }],
        ['OS=="mac"', {
          "libraries": [
            "<!@(MagickWand-config --libs)"
          ],
          "xcode_settings": {
            "OTHER_CFLAGS": [
              "<!@(MagickWand-config --cflags)"
            ]
          }
        }],
        ['OS!="win" and OS!="mac"', {
          "libraries": [
            "<!@(pkg-config --libs Wand)"
          ],
          "cflags": [
            "<!@(pkg-config --cflags Wand)"
          ]
        }]
      ]
    }
  ]
}
