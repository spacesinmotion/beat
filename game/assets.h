#ifndef ASSETS
#define ASSETS

typedef enum Image {
  Img_tilemap = 0,
  Img_wearisome,

  NB_Img,
} Image;

static const char *image_paths[NB_Img] = {
    "assets/tilemap.png",
    "assets/wearisome.png",
};

#endif