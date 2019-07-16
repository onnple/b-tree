#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "btree.h"


static void traverse_btree(Post *post){
    printf("%u ", post->id);
}

void btree(void){
    BTree *btree = btree_init();
    for (int i = 0; i < 1000; ++i) {
        Post post;
        post.id = i + 1;
        strcpy(post.title, "The Differences Between Killers And Leaders");
        strcpy(post.content, "The Differences Between Killers And Leaders");
        btree_add(btree, &post);
    }
    btree_traverse(btree, traverse_btree);
    printf("\n");

    btree_delete_by_id(btree, 240);
    for (int k = 1000; k > 0; --k) {
        if(k == 240)
            continue;
        Post *post = btree_get_by_id(btree, k);
        printf("%u %s\n", post->id, post->title);
    }

    btree_delete_by_id(btree, 360);
    printf("deleted 2 size: %u\n", btree->size);
    for (int j = 0; j < 1000; ++j) {
        btree_delete_by_id(btree, j + 1);
    }
    btree_traverse(btree, traverse_btree);

    btree_clear(btree);
}

int main() {
    btree();
    return 0;
}