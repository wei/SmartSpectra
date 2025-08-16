{
  "targets": [
    {
      "target_name": "smartspectra",
      "sources": [ "native/smartspectra.cc" ],
      "cflags_cc": [ "-std=c++17" ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")",
        "../cpp"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "libraries": [
        "-lopencv_core",
        "-lopencv_imgcodecs",
        "-lopencv_imgproc",
        "-lprotobuf"
      ]
    }
  ]
}
