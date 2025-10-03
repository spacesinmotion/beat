#ifndef ASSETS
#define ASSETS

typedef enum Image {
  Img_menubar = 0,
  Img_bling,
  Img_emo,

  NB_Img,
} Image;

static const char *image_paths[NB_Img] = {
    "assets/menubar.png", //
    "assets/bling.png",   //
    "assets/emo.png",     //
};

typedef enum G_Font {
  Assistant_Regular_8 = 0,
  Assistant_Regular_12,
  Oswald_Regular_8,
  Oswald_Regular_12,
  Nb_Font,
} G_Font;

static const char *font_paths[Nb_Font] = {
    "assets/Assistant-Regular.ttf", //
    "assets/Assistant-Regular.ttf", //
    "assets/Oswald-Regular.ttf",    //
    "assets/Oswald-Regular.ttf",    //
};
static int font_size[Nb_Font] = {
    8,  //
    12, //
    8,  //
    12, //
};
#endif