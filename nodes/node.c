#include "node.h"

CCnode* cc_scenes;
u32 cc_scene_amt;

CCnode cc_root;

void cc_nodesInit(void) {
    cc_scenes = malloc(sizeof(CCnode));  

    cc_root.name = "root";

    cc_root.child_amt = 0;
    cc_root.child_capac = 0;
    cc_root.component_amt = 0;
    cc_root.component_capac = 0;
}

void cc_nodesUninit(void) {
    free(cc_scenes);
}
