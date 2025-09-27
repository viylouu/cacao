#include "node.h"

CCnode* cc_scenes;
u32 cc_scene_amt;

CCnode cc_root;

void cc_nodesInit(void) {
    cc_scenes = malloc(sizeof(CCnode));    
}

void cc_nodesUninit(void) {
    free(cc_scenes);
}
