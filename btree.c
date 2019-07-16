//
// Created by Once on 2019/7/12.
//

#include "btree.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

BTree *btree_init(){
    BTree *btree = (BTree*)malloc(sizeof(BTree));
    if(!btree){
        perror("init B Tree error.");
        return NULL;
    }
    btree->root = NULL;
    btree->size = 0;
    return btree;
}

int btree_is_full(BTree *btree){
    BNode *node = (BNode*)malloc(sizeof(BNode));
    if(!node)
        return 1;
    free(node);
    return 0;
}

int btree_is_empty(BTree *btree){
    if(btree == NULL)
        return 1;
    return btree->size == 0;
}

/**
 * void*指针指向数组中的元素，如果数组元素是int值，那么该指针为int*指针，
 * 如果数组元素是(int*)指针，那么该指针为(int*)*，取出指针值使用*(int**)
 * */
static int compare(const void *a, const void *b){
    Post *pa = *(Post**)a;
    Post *pb = *(Post**)b;
    return pa->id - pb->id;
}

static int compare_child(const void *a, const void *b){
    BNode *na = *(BNode**)a;
    BNode *nb = *(BNode**)b;
    return na->posts[0]->id - nb->posts[0]->id;
}

static BNode *split_node_right(BNode *root){
    int position = (int)ceil(DEGREE / 2.0);
    BNode *node = (BNode*)malloc(sizeof(BNode));
    node->leaf = root->leaf;
    node->size = 0;
    node->parent = NULL;
    int j = 0;
    for (int i = position; i < DEGREE; ++i) {
        node->posts[j] = root->posts[i];
        root->posts[i] = NULL;
        root->size--;
        node->size++;
    }
    return node;
}

static BNode *split_node_left(BNode *root){
    int position = (int)ceil(DEGREE / 2.0) - 1;
    BNode *node = (BNode*)malloc(sizeof(BNode));
    node->leaf = root->leaf;
    node->size = 0;
    node->parent = NULL;
    int j = 0;
    for (int i = 0; i < position; ++i) {
        node->posts[j] = root->posts[i];
        root->size--;
        node->size++;
    }
    int i = 0;
    for (int k = position; k < DEGREE; ++k) {
        root->posts[i] = root->posts[k];
        i++;
    }
    for (int l = 1; l <= DEGREE - 1 - position; ++l) {
        root->posts[position + l] = NULL;
    }
    return node;
}

// 使用二分法确定访问路径
static int binary(BNode *root, int start, int end, unsigned int id){
    if(start == end)
        return start;
    int position = (start + end) / 2;
    if(root->posts[position]->id == id)
        return position;
    else if(root->posts[position]->id > id) {
        if(root->posts[0]->id >= id)
            return 0;
        return binary(root, start, position - 1, id);
    }
    else {
        if(root->posts[root->size - 1]->id <= id)
            return root->size - 1;
        return binary(root, position + 1, end, id);
    }
}

static int path(BNode *root, unsigned int id){
    return binary(root, 0, root->size - 1, id);
}

// root.size == 0
static int binary_child(BNode *root, int start, int end){
    if(start == end)
        return start;
    int center = (start + end) / 2;
    if(root->posts[0] == root->parent->children[center]->posts[0])
        return center;
    else if(root->posts[0] > root->parent->children[center]->posts[0]){
        if(root->posts[0] >= root->parent->children[root->parent->size]->posts[0])
            return root->parent->size;
        return binary_child(root, center + 1, end);
    }
    else{
        if(root->posts[0] <= root->parent->children[0]->posts[0])
            return 0;
        return binary_child(root, start, center - 1);
    }
}

static void copy_to_array(BNode *parent, int root_index, BNode *node){
    int index = node->size;
    for (int i = root_index + 1; i < parent->size + 1; ++i) {
        node->children[index] = parent->children[i];
        parent->children[i]->parent = node;
        parent->children[i] = NULL;
        index++;
    }
}

/**
 * 1、添加到单叶子结点
 * 2、多索引结点的情况添加
 * 3、左边分裂、右边分裂、中间分裂
 * */
static BNode *add_node(BTree *btree, BNode *root, Post *post, int up, int split_tag){
    if(!root){
        root = (BNode*)malloc(sizeof(BNode));
        if(!root){
            perror("init btree node error.");
            return NULL;
        }
        Post *p;
        if(!up){
            p = (Post*)malloc(sizeof(Post));
            if(!p){
                perror("init post error.");
                return NULL;
            }
            p->id = post->id;
            strcpy(p->title, post->title);
            strcpy(p->content, post->content);
        }
        else{
            p = post;
        }
        root->leaf = up ? 0 : 1;
        root->parent = NULL;
        root->posts[0] = p;
        root->size = 1;
        if(btree->size == 0 || up)
            btree->root = root;
        btree->size++;
        return root;
    }
    // 分别处理叶结点和索引结点，up为索引结点的插入
    int index = path(root, post->id);
    if(root->leaf || up){
        // id相等
        if(post->id == root->posts[index]->id){
            strcpy(root->posts[root->size]->title, post->title);
            strcpy(root->posts[root->size]->content, post->content);
            return root;
        }
        Post *p = NULL;
        if(!up){
            p = (Post*)malloc(sizeof(Post));
            if(!p){
                perror("init post error.");
                return NULL;
            }
            p->id = post->id;
            strcpy(p->title, post->title);
            strcpy(p->content, post->content);
        }
        else{
            p = post;
        }
        root->posts[root->size] = p;
        root->size++;
        qsort(root->posts, root->size, sizeof(Post*), compare);
    }
    else{
        // 确定访问路径
        // 需要使用二分法
        if(post->id == root->posts[index]->id){
            // id相等
            strcpy(root->posts[index]->title, post->title);
            strcpy(root->posts[index]->content, post->content);
            return root;
        }
        if(post->id > root->posts[index]->id)
            index++;
        BNode *node = add_node(btree, root->children[index], post, up, split_tag);
        return node->parent ? node->parent : node;
    }

    // 处理结点分裂
    if(root->size == DEGREE){
        if(root->parent && root->leaf){
            int root_index = binary_child(root, 0, root->parent->size);
            // 左分裂
            if(root_index == 0){
                split_tag = -1;
            }
                // 右分裂
            else if(root_index == root->parent->size){
                split_tag = 1;
            }
                // 中间分裂
            else{
                split_tag = 0;
            }
        }
        int position = (int)ceil(DEGREE / 2.0) - 1;
        BNode *parent = add_node(btree, root->parent, root->posts[position], 1, split_tag);
        BNode *node = NULL;
        if(split_tag == -1){
            node = split_node_left(root);
        }
        else{
            node = split_node_right(root);
        }
        btree->size++;
        if(split_tag == 0 && root->parent && root->parent != parent){
            parent->children[parent->size - 1] = node;
            node->parent = parent;
            int root_index = binary_child(root, 0, root->parent->size);
            copy_to_array(root->parent, root_index, parent);
        }
        else{
            parent->children[parent->size - 1] = root;
            parent->children[parent->size] = node;
            root->parent = parent;
            node->parent = parent;
        }
        root->posts[position] = NULL;
        root->size--;
        qsort(parent->children, parent->size + 1, sizeof(BNode*), compare_child);
        return node;
    }

    return root;
}

int btree_add(BTree *btree, Post *post){
    if(btree == NULL || post == NULL)
        return 0;
    return add_node(btree, btree->root, post, 0, 1) != NULL;
}

static void *delete_from_array(void *array[], int deleted_position, int size){
    for (int i = deleted_position; i < size - 1; ++i) {
        array[i] = array[i + 1];
    }
    array[size - 1] = NULL;
    return array;
}

static void add_sibling_to_root_key(BNode *root, BNode *sibling){
    for (int k = 0; k < sibling->size; ++k) {
        root->posts[root->size] = sibling->posts[k];
        root->size++;
    }
}

static int add_sibling_to_root_children(BNode *root, BNode *sibling){
    int index = root->size + 1;
    for (int k = 0; k < sibling->size + 1; ++k) {
        root->children[index] = sibling->children[k];
        sibling->children[k]->parent = root;
        index++;
    }
    return index;
}

static Post *find_max(BNode *root){
    if(root->leaf)
        return root->posts[0];
    return find_max(root->children[0]);
}

/**
 * 1、正常 => root
 * 2、根 => root, size=0 => NULL (会改变根结点, 根结点被合并或单叶结点为空)
 * 3、借 => root
 * 4、合并 => root, 调整parent => NULL|root (会改变根结点, 根结点被合并)
 * */
static BNode *adjust_node(BTree *btree, BNode *root, int root_index){
    BNode *parent = root->parent;
    int count = (int)ceil(DEGREE / 2.0) - 1;
    // 检查当前的关键字数
    if(root->size >= count){
        return root; // 1、正常情况，，不会发生结点结构改变
    }

    // 检查兄弟的关键字数
    // 使用二分法
//    int root_index = binary_child(root, 0, parent->size);
    int sibling_index = root_index + 1 > parent->size ? root_index - 1 : root_index + 1;
    BNode *sibling = parent->children[sibling_index];

    // 从兄弟中借一个
    if(sibling->size > count){
        // 从右边借
        if(sibling_index > root_index){
            root->posts[root->size] = parent->posts[root_index];
            root->size++;
            parent->posts[root_index] = sibling->posts[0];
            if(!root->leaf){
                root->children[root->size] = sibling->children[0];
                delete_from_array((void*)sibling->children, 0, sibling->size + 1);
            }
            // 删除
            delete_from_array((void*)sibling->posts, 0, sibling->size);
            sibling->size--;
        }
        // 从左边借
        else{
            root->posts[root->size] = parent->posts[root_index - 1];
            root->size++;
            qsort(root->posts, root->size, sizeof(Post*), compare);
            parent->posts[root_index - 1] = sibling->posts[sibling->size - 1];
            sibling->posts[sibling->size - 1] = NULL;
            sibling->size--;
            if(!root->leaf){
                root->children[root->size] = sibling->children[sibling->size];
                delete_from_array((void*)sibling->children, sibling->size, sibling->size + 1);
                qsort(root->children, root->size + 1, sizeof(BNode*), compare_child);
            }
        }
        return root; // 2、借，不会发生结点结构改变
    }
    // 合并
    else{
        int parent_index = -1;
        int children_size = parent->size + 1;
        if(parent->parent)
            parent_index = binary_child(parent, 0, parent->parent->size);
        // 和右边合并
        if(sibling_index > root_index){
            if(!root->leaf){
                add_sibling_to_root_children(root, sibling);
//                qsort(root->children, size, sizeof(BNode*), compare_child);
            }

            root->posts[root->size] = parent->posts[root_index];
            root->size++;
            // 从父结点中删除关键字
            delete_from_array((void*)parent->posts, root_index, parent->size);
            // 从父结点删除儿子
            delete_from_array((void*)parent->children, sibling_index, parent->size + 1);
            parent->size--;

            // 添加关键字
            add_sibling_to_root_key(root, sibling);
            free(sibling);
        }
        // 和左边合并
        else{
            // 索引结点添加儿子结点
            if(!root->leaf) {
                int size = add_sibling_to_root_children(root, sibling);
                qsort(root->children, size, sizeof(BNode*), compare_child);
            }
            // 添加关键字
            add_sibling_to_root_key(root, sibling);

            root->posts[root->size] = parent->posts[root_index - 1];
            root->size++;
            // 从父结点中删除关键字
            delete_from_array((void*)parent->posts, root_index - 1, parent->size);
            // 从父结点删除儿子
            delete_from_array((void*)parent->children, sibling_index, parent->size + 1);
            parent->size--;

            free(sibling);
            qsort(root->posts, root->size, sizeof(Post*), compare);
        }
        btree->size--;
        children_size--;
        // 重新检查父结点的关键字数 =>>>>> root | NULL
        // 根结点被合并
        if(!parent->parent){
            if(parent->size == 0 && children_size == 1 && parent == btree->root){
                free(parent);
                btree->size--;
                root->parent = NULL;
                return root;
            }
        }
//        root_index = binary_child(parent, 0, parent->parent->size);
        return adjust_node(btree, parent, parent_index); // 4、正常向上检查父结点(->正常情况->借 ||　->继续合并)
    }
}

/**
 * 1、删除索引结点
 * 2、删除叶子结点
 * 3、两个结点(3)
 * 4、只有一个结点，结点为空 -
 * */
static BNode *delete_node(BTree *btree, BNode *root, unsigned int id){
    // 确定访问路径
    // 需要使用二分法
    int index = path(root, id);
    BNode *node = NULL;
    if(id == root->posts[index]->id){
        // 叶子结点
        if(root->leaf){
            // 删除
            int root_index = -1;
            if(root->parent)
                root_index = binary_child(root, 0, root->parent->size);
            free(root->posts[index]); // 叶结点需要free
            delete_from_array((void*)root->posts, index, root->size);
            root->size--;
            // 根结点
            if(!root->parent){
                if(root->size == 0){
                    free(root);
                    btree->size--;
                    return NULL;
                }
            }
            node = adjust_node(btree, root, root_index);
        }

        // 索引结点
        else{
            // 右子树中最小关键字
            Post *post = find_max(root->children[index + 1]);
            root->posts[index]->id = post->id;
            strcpy(root->posts[index]->title, post->title);
            strcpy(root->posts[index]->content, post->content);
            node = delete_node(btree, root->children[index + 1], root->posts[index]->id);
        }
    }
    else{
        // 向下传递路径
        if(!root->leaf){
            if(id > root->posts[index]->id)
                index++;
            node = delete_node(btree, root->children[index], id);
        }
        else
            node = root; // 一直到叶子没找到
    }
    return node->parent ? node->parent : node;
}

int btree_delete_by_id(BTree *btree, unsigned int id){
    if(btree == NULL || btree->size == 0)
        return 0;
    btree->root = delete_node(btree, btree->root, id);
    return 1;
}

static void *get_node(BNode *root, unsigned int id){
    int index = path(root, id);
    if(root->posts[index]->id == id)
        return root->posts[index];
    if(root->leaf)
        return NULL;
    if(id > root->posts[index]->id)
        index++;
    return get_node(root->children[index], id);
}

Post *btree_get_by_id(BTree *btree, unsigned int id){
    if(btree == NULL || btree->size == 0)
        return NULL;
    return get_node(btree->root, id);
}

static int traverse_node(BNode *root, void(*traverse)(Post*)){
    int count = 0;
    printf("[");
    for (int i = 0; i < root->size; ++i) {
        traverse(root->posts[i]);
        count++;
    }
    printf("]");
    if(!root->leaf){
        for (int i = 0; i < root->size + 1; ++i) {
            count = count + traverse_node(root->children[i], traverse);
        }
    }
    return count;
}

void btree_traverse(BTree *btree, void(*traverse)(Post*)){
    if(btree == NULL || btree->size == 0 || traverse == NULL)
        return;
    int count = traverse_node(btree->root, traverse);
    printf("\ncount: %d\n", count);
}

static int clear_node(BNode *root){
    if(!root)
        return 0;
    if(!root->leaf){
        for (int i = 0; i < root->size + 1; ++i) {
            clear_node(root->children[i]);
        }
    }
    for (int i = 0; i < root->size; ++i) {
        free(root->posts[i]);
        root->posts[i] = NULL;
    }
    free(root);
    return 1;
}

int btree_clear(BTree *btree){
    if(btree == NULL)
        return 0;
    clear_node(btree->root);
    free(btree);
    return 1;
}

