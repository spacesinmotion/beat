#ifndef ASSETS
#define ASSETS

typedef enum Image {
  Img_tilemap = 0,
  Img_wearisome,
  Img_marker,
  Img_street,
  Img_house,
  Img_menubar,

  NB_Img,
} Image;

static const char *image_paths[NB_Img] = {
    "assets/tilemap.png",   //
    "assets/wearisome.png", //
    "assets/marker.png",    //
    "assets/street.png",    //
    "assets/house.png",     //
    "assets/menubar.png",   //
};

#endif