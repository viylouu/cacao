#ifndef CC_NODES_H
#define CC_NODES_H

#include <core/macros/macros.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// no, i am not adding node.js

typedef struct {
    void* data;
    u32 size;
} CCcomponent;

typedef struct CCnode CCnode;
struct CCnode {
    char* name;
    b8 open; // for editor

    CCcomponent* components;
    u32 component_amt;
    u32 component_capac;

    CCnode* children;
    u32 child_amt;
    u32 child_capac;
};

extern CCnode* cc_scenes;
extern u32 cc_scene_amt;

extern CCnode cc_root;

static inline void add_component(CCnode* node, CCcomponent* component) {
    if (node->component_amt == node->component_capac) {
        node->component_capac = node->component_capac == 0? 4 : node->component_capac * 2;
        node->components = realloc(node->components, node->component_capac * sizeof(CCcomponent));
        if (!node->components) {
            printf("failed to add component!\n");
            exit(1);
        }
    }
    memcpy((char*)node->components + node->component_amt * sizeof(CCcomponent), component, sizeof(CCcomponent));
    ++node->component_amt;
}

// no fucking clue if this works
static inline void remove_component(CCnode* node, u32 index) {
    if (index >= node->component_amt) return;

    if (index < node->component_amt - 1)
        memmove(&node->components[index], &node->components[index + 1], (node->component_amt - index - 1) * sizeof(CCcomponent));

    --node->component_amt;
}


void cc_nodesInit(void);
void cc_nodesUninit(void);

#endif
