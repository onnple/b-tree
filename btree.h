//
// Created by Once on 2019/7/12.
//

#ifndef DATALGORITHM_BTREE_H
#define DATALGORITHM_BTREE_H

// 阶数或度数
#define DEGREE 3

// 数据表的一行记录，业务数据
typedef struct post{
    unsigned int id; // 关键字
    char title[128];
    char content[256];
} Post;

// B树结点，DEGREE阶B树，DEGREE-1个关键字，DEGREE个儿子，关键字使用多一个位置用于保存溢出的关键字
typedef struct bnode{
    unsigned int leaf; // 是否是叶子
    unsigned int size; // 关键字数量/记录数量
//    int keys[M_MAX]; // 关键字域，这里使用数据记录中的关键字，这样会比较好操作
    Post *posts[DEGREE]; // 数据记录集(Records)，B树的主要特点是数据记录存储在所有结点
    struct bnode *children[DEGREE]; // 儿子结点集
    struct bnode *parent; // 父结点
} BNode;

// 查找结果集
typedef struct resultset{
    unsigned int size;
    unsigned int id;
    Post *posts[]; // 结果记录
} ResultSet;

// B树ADT对外接口
typedef struct btree{
    BNode *root;
    unsigned int size;
} BTree;

// 算法声明
extern BTree *btree_init();
extern int btree_is_full(BTree *btree);
extern int btree_is_empty(BTree *btree);
extern int btree_add(BTree *btree, Post *post);
extern int btree_delete_by_id(BTree *btree, unsigned int id);
extern Post *btree_get_by_id(BTree *btree, unsigned int id);
extern void btree_traverse(BTree *btree, void(*traverse)(Post*));
extern int btree_clear(BTree *btree);

#endif //DATALGORITHM_BTREE_H
