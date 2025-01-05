#ifndef ASSETS
#define ASSETS

typedef enum Image {
  Img_tilemap = 0,
  Img_house_map,
  Img_wearisome,
  Img_weapons,
  Img_marker,
  Img_overlay_images,
  Img_street,
  Img_house,
  Img_menubar,
  Img_maze_pointer,

  NB_Img,
} Image;

static const char *image_paths[NB_Img] = {
    "assets/tilemap.png",        //
    "assets/house_map.png",      //
    "assets/wearisome.png",      //
    "assets/weapons.png",        //
    "assets/marker.png",         //
    "assets/overlay_images.png", //
    "assets/street.png",         //
    "assets/house.png",          //
    "assets/menubar.png",        //
    "assets/maze_pointer.png",   //
};

typedef enum Font {
  Assistant_Regular_8 = 0,
  Assistant_Regular_12,
  Nb_Font,
} Font;

static const char *font_paths[Nb_Font] = {
    "assets/Assistant-Regular.ttf", //
    "assets/Assistant-Regular.ttf", //
};
static int font_size[Nb_Font] = {
    8,  //
    12, //
};
#endif