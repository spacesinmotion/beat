#ifndef TEXTURES_H
#define TEXTURES_H

#include "engine/TextureDesc.h"

typedef enum GTexture {
  Img_tilemap = 0,
  Img_house_map,
  Img_wearisome,
  Img_weapons,
  Img_marker,
  Img_bling,
  Img_overlay_images,
  Img_street,
  Img_house,
  Img_menubar,
  Img_coin,
  Img_maze_pointer,
  Img_connections,

  NB_Img,
} GTexture;

#ifdef ENGINE_IMPLEMENTATION
static const TextureDesc texture_list[NB_Img] = {
    {"assets/tilemap.png"},        //
    {"assets/house_map.png"},      //
    {"assets/wearisome.png"},      //
    {"assets/weapons.png"},        //
    {"assets/marker.png"},         //
    {"assets/bling.png"},          //
    {"assets/overlay_images.png"}, //
    {"assets/street.png"},         //
    {"assets/house.png"},          //
    {"assets/menubar.png"},        //
    {"assets/coin.png"},           //
    {"assets/maze_pointer.png"},   //
    {"assets/connections.png"},    //
};
#define G_TEXTURE GTexture
#define G_TEXTURE_COUNT NB_Img
#define G_TEXTURE_LIST texture_list
#endif

typedef enum MenuIcon {
  MI_Street,
  MI_Marketplace,
  MI_House,
  MI_Water,
  MI_Food,
  MI_Click,
  MI_Entertainment,
  MI_ConstructionMaterial,
  MI_Science,
  MI_Manager,
  MI_Combinator,
  MI___empty,
  MI_Industry,
  MI_Logistics,
  MI_WareHouse,
  Nb_MI,
} MenuIcon;

#endif