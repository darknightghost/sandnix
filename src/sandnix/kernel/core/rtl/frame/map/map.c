/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "map.h"
#include "../../../mm/mm.h"

static prbtree_node_t	get_node(pmap_t p_map, void* p_key);
static prbtree_node_t	insert_node(pmap_t p_map, void* p_key, void* p_value);
static prbtree_node_t	remove_node(pmap_t p_map, prbtree_node_t p_node);
static prbtree_node_t	next_node(prbtree_node_t p_node);
static prbtree_node_t	l_rotate(pmap_t p_map, prbtree_node_t p_node);
static prbtree_node_t	r_rotate(pmap_t p_map, prbtree_node_t p_node);
static void				remove_rebalance(pmap_t p_map, prbtree_node_t p_node,
        prbtree_node_t p_parent);

void core_rtl_map_init(pmap_t p_map, item_compare_t compare_func, pheap_t heap)
{
    p_map->p_tree = NULL;
    p_map->p_heap = heap;
    p_map->compare_func = compare_func;

    return;
}

void* core_rtl_map_set(pmap_t p_map, void* p_key, void* p_value)
{
    prbtree_node_t p_node;
    void* ret;

    p_node = get_node(p_map, p_key);

    if(p_value == NULL) {
        if(p_node != NULL) {
            ret = p_node->p_value;
            p_node = remove_node(p_map, p_node);
            core_mm_heap_free(p_node, p_map->p_heap);
            return ret;
        }

    } else {
        if(p_node == NULL) {
            p_node = insert_node(p_map, p_key, p_value);

            if(p_node != NULL) {
                return p_value;
            }

        } else {
            ret = p_node->p_value;
            p_node->p_value = p_value;
            return ret;
        }
    }

    return NULL;
}

void* core_rtl_map_get(pmap_t p_map, void* p_key)
{
    prbtree_node_t p_node;
    p_node = get_node(p_map, p_key);

    if(p_node == NULL) {
        return NULL;

    } else {
        return p_node->p_value;
    }
}

void* core_rtl_map_next(pmap_t p_map, void* p_key)
{
    prbtree_node_t p_node;

    if(p_key == NULL) {
        if(p_map->p_tree == NULL) {
            return NULL;

        } else {
            for(p_node = p_map->p_tree;
                p_node->p_lchild != NULL;
                p_node = p_node->p_lchild);

            return p_node->p_key;
        }

    } else {
        p_node = get_node(p_map, p_key);

        if(p_node == NULL) {
            return NULL;

        }

        p_node = next_node(p_node);
        return p_node->p_key;
    }
}



void core_rtl_map_destroy(pmap_t p_map, item_destroyer_t key_destroier,
                          item_destroyer_t value_destroier, void* arg)
{
    prbtree_node_t p_node;
    prbtree_node_t p_remove_node;

    if(p_map->p_tree == NULL) {
        return;
    }

    p_node = p_map->p_tree;

    while(p_node != NULL) {
        if(p_node->p_lchild != NULL) {
            p_node = p_node->p_lchild;

        } else if(p_node->p_rchild != NULL) {
            p_node = p_node->p_rchild;

        } else {
            p_remove_node = p_node;
            p_node = p_node->p_parent;

            if(p_node != NULL) {
                if(p_remove_node == p_node->p_lchild) {
                    p_node->p_lchild = NULL;

                } else {
                    p_node->p_rchild = NULL;
                }
            }

            if(key_destroier != NULL) {
                key_destroier(p_remove_node->p_key, arg);
            }

            if(value_destroier != NULL) {
                value_destroier(p_remove_node->p_value, arg);
            }

            core_mm_heap_free(p_remove_node, p_map->p_heap);
        }
    }

    return;
}

prbtree_node_t get_node(pmap_t p_map, void* p_key)
{
    int cmp_result;
    prbtree_node_t p_node;

    for(p_node = p_map->p_tree;
        p_node != NULL;) {
        cmp_result = p_map->compare_func(p_key, p_node->p_key);

        if(cmp_result > 0) {
            p_node = p_node->p_rchild;

        } else if(cmp_result == 0) {
            break;

        } else {
            p_node = p_node->p_lchild;
        }
    }

    return p_node;
}

prbtree_node_t next_node(prbtree_node_t p_node)
{
    if(p_node->p_rchild != NULL) {
        //p_node has right child
        for(p_node = p_node->p_rchild;
            p_node->p_lchild != NULL;
            p_node = p_node->p_lchild);

        return p_node;

    } else if(p_node->p_parent == NULL) {
        //p_node is root and have not right child
        return NULL;

    } else if(p_node->p_parent->p_lchild) {
        //p_node have not right child and is left child
        return p_node->p_parent;

    } else {
        //p_node is right child and have not right child
        while(true) {
            if(p_node->p_parent == NULL) {
                return NULL;
            }

            if(p_node == p_node->p_parent->p_lchild) {
                return p_node->p_parent;
            }

            p_node = p_node->p_parent;
        }
    }
}

prbtree_node_t insert_node(pmap_t p_map, void* p_key, void* p_value)
{
    prbtree_node_t p_insert_node;
    prbtree_node_t p_z;
    prbtree_node_t p_y;
    prbtree_node_t p_node;
    int cmp_result;

    p_node = core_mm_heap_alloc(sizeof(rbtree_node_t), p_map->p_heap);

    if(p_node == NULL) {
        return NULL;
    }

    p_node->p_parent = NULL;
    p_node->p_lchild = NULL;
    p_node->p_rchild = NULL;
    p_node->p_key = p_key;
    p_node->p_value = p_value;

    if(p_map->p_tree == NULL) {
        //The tree is empty
        p_map->p_tree = p_node;
        p_node->color = RBTREE_NODE_BLACK;

    } else {
        //Find where to insert the node
        p_insert_node = p_map->p_tree;

        while(true) {
            cmp_result = p_map->compare_func(p_node->p_key,
                                             p_insert_node->p_key);

            if(cmp_result <= 0) {
                if(p_insert_node->p_lchild == NULL) {
                    p_insert_node->p_lchild = p_node;
                    p_node->p_parent = p_insert_node;
                    break;

                } else {
                    p_insert_node = p_insert_node->p_lchild;
                }

            } else {
                if(p_insert_node->p_rchild == NULL) {
                    p_insert_node->p_rchild = p_node;
                    p_node->p_parent = p_insert_node;
                    break;

                } else {
                    p_insert_node = p_insert_node->p_rchild;
                }
            }
        }

        p_node->color = RBTREE_NODE_RED;

        //Fix tree
        p_z = p_node;

        while(p_z->p_parent != NULL
              && p_z->p_parent->color == RBTREE_NODE_RED) {
            if(p_z->p_parent == p_z->p_parent->p_parent->p_lchild) {
                p_y = p_z->p_parent->p_parent->p_rchild;

                if(p_y != NULL && p_y->color == RBTREE_NODE_RED) {
                    //Case 1
                    p_z->p_parent->color = RBTREE_NODE_BLACK;
                    p_y->color = RBTREE_NODE_BLACK;
                    p_z->p_parent->p_parent->color = RBTREE_NODE_RED;
                    p_z = p_z->p_parent->p_parent;
                    continue;

                } else if(p_z == p_z->p_parent->p_rchild) {
                    //Case 3
                    p_z = p_z->p_parent;
                    l_rotate(p_map, p_z);
                }

                if(p_z->p_parent != NULL
                   && p_z->p_parent->p_parent != NULL) {
                    //Case 2
                    p_z->p_parent->color = RBTREE_NODE_BLACK;
                    p_z->p_parent->p_parent->color = RBTREE_NODE_RED;
                    r_rotate(p_map, p_z->p_parent->p_parent);
                    break;
                }

            } else {
                p_y = p_z->p_parent->p_parent->p_lchild;

                if(p_y != NULL && p_y->color == RBTREE_NODE_RED) {
                    //Case 1
                    p_z->p_parent->color = RBTREE_NODE_BLACK;
                    p_y->color = RBTREE_NODE_BLACK;
                    p_z->p_parent->p_parent->color = RBTREE_NODE_RED;
                    p_z = p_z->p_parent->p_parent;
                    continue;

                } else if(p_z == p_z->p_parent->p_lchild) {
                    //Case 3
                    p_z = p_z->p_parent;
                    r_rotate(p_map, p_z);
                }

                if(p_z->p_parent != NULL
                   && p_z->p_parent->p_parent != NULL) {
                    //Case 2
                    p_z->p_parent->color = RBTREE_NODE_BLACK;
                    p_z->p_parent->p_parent->color = RBTREE_NODE_RED;
                    l_rotate(p_map, p_z->p_parent->p_parent);
                    break;
                }
            }
        }

        p_map->p_tree->color = RBTREE_NODE_BLACK;
    }

    return p_node;
}

prbtree_node_t remove_node(pmap_t p_map, prbtree_node_t p_node)
{
    prbtree_node_t p_tmp_node;
    prbtree_node_t p_x;
    prbtree_node_t p_parent;
    void* p_tmp;
    prbtree_node_t ret;

    //Remove node
    //Find node to swap
    if(p_node->p_lchild != NULL && p_node->p_rchild != NULL) {
        p_tmp_node = p_node->p_lchild;

        while(p_tmp_node->p_rchild != NULL) {
            p_tmp_node = p_tmp_node->p_rchild;
        }

        //Swap
        p_tmp = p_tmp_node->p_key;
        p_tmp_node->p_key = p_node->p_key;
        p_node->p_key = p_tmp;
        p_tmp = p_tmp_node->p_value;
        p_tmp_node->p_value = p_node->p_value;
        p_node->p_value = p_tmp;

        p_node = p_tmp_node;
    }

    ret = p_node;

    //Remove node
    if(p_node->color == RBTREE_NODE_RED) {
        //Red
        if(p_node->p_lchild != NULL) {
            if(p_node->p_parent->p_lchild == p_node) {
                p_node->p_parent->p_lchild = p_node->p_lchild;

            } else {
                p_node->p_parent->p_rchild = p_node->p_lchild;
            }

            p_node->p_lchild->p_parent = p_node->p_parent;

        } else {
            if(p_node->p_parent->p_lchild == p_node) {
                p_node->p_parent->p_lchild = NULL;

            } else {
                p_node->p_parent->p_rchild = NULL;
            }

        }

    } else {
        //Black
        if(p_node == p_map->p_tree) {
            if(p_node->p_lchild == NULL
               && p_node->p_rchild == NULL) {
                p_map->p_tree = NULL;

            } else if(p_node->p_lchild != NULL) {
                p_map->p_tree = p_node->p_lchild;
                p_node->p_lchild->p_parent = NULL;

            } else {
                p_map->p_tree = p_node->p_rchild;
                p_node->p_rchild->p_parent = NULL;
            }

            if(p_map->p_tree != NULL) {
                (p_map->p_tree)->color = RBTREE_NODE_BLACK;
            }

        } else {
            if(p_node->p_lchild != NULL) {
                if(p_node->p_parent->p_lchild == p_node) {
                    p_node->p_parent->p_lchild = p_node->p_lchild;
                    p_x = p_node->p_parent->p_rchild;

                } else {
                    p_node->p_parent->p_rchild = p_node->p_lchild;
                    p_x = p_node->p_parent->p_lchild;
                }

                p_x = p_node->p_lchild;
                p_node->p_lchild->p_parent = p_node->p_parent;

            } else if(p_node->p_rchild != NULL) {
                if(p_node->p_parent->p_lchild == p_node) {
                    p_node->p_parent->p_lchild = p_node->p_rchild;

                } else {
                    p_node->p_parent->p_rchild = p_node->p_rchild;
                }

                p_x = p_node->p_rchild;
                p_node->p_rchild->p_parent = p_node->p_parent;

            } else {
                if(p_node->p_parent->p_lchild == p_node) {
                    p_node->p_parent->p_lchild = NULL;

                } else {
                    p_node->p_parent->p_rchild = NULL;
                }

                p_x = NULL;
                p_parent = p_node->p_parent;
            }

            remove_rebalance(p_map, p_x, p_parent);
        }
    }

    return ret;
}

prbtree_node_t l_rotate(pmap_t p_map, prbtree_node_t p_node)
{
    prbtree_node_t p_new_parent;

    p_new_parent = p_node->p_rchild;

    p_new_parent->p_parent = p_node->p_parent;

    if(p_node->p_parent == NULL) {
        p_map->p_tree = p_new_parent;

    } else {
        if(p_node == p_node->p_parent->p_lchild) {
            p_node->p_parent->p_lchild = p_new_parent;

        } else {
            p_node->p_parent->p_rchild = p_new_parent;
        }
    }

    p_node->p_rchild = p_new_parent->p_lchild;

    if(p_node->p_rchild != NULL) {
        p_node->p_rchild->p_parent = p_node;
    }

    p_new_parent->p_lchild = p_node;
    p_node->p_parent = p_new_parent;

    return p_new_parent;
}

prbtree_node_t r_rotate(pmap_t p_map, prbtree_node_t p_node)
{
    prbtree_node_t p_new_parent;

    p_new_parent = p_node->p_lchild;

    p_new_parent->p_parent = p_node->p_parent;

    if(p_node->p_parent == NULL) {
        p_map->p_tree = p_new_parent;

    } else {
        if(p_node == p_node->p_parent->p_lchild) {
            p_node->p_parent->p_lchild = p_new_parent;

        } else {
            p_node->p_parent->p_rchild = p_new_parent;
        }
    }

    p_node->p_lchild = p_new_parent->p_rchild;

    if(p_node->p_lchild != NULL) {
        p_node->p_lchild->p_parent = p_node;
    }

    p_new_parent->p_rchild = p_node;
    p_node->p_parent = p_new_parent;

    return p_new_parent;
}

void remove_rebalance(pmap_t p_map, prbtree_node_t p_node,
                      prbtree_node_t p_parent)
{
    prbtree_node_t p_sibling;

#define color(p) (((p) == NULL || (p)->color == RBTREE_NODE_BLACK) \
                  ? RBTREE_NODE_BLACK \
                  : RBTREE_NODE_RED)

    while(true) {
        if(p_node != NULL) {
            p_parent = p_node->p_parent;
        }

        if(color(p_node) == RBTREE_NODE_RED) {
            p_node->color = RBTREE_NODE_BLACK;
            break;

        } else if(p_node == p_map->p_tree) {
            break;
        }

        if(p_parent->p_lchild == p_node) {
            p_sibling = p_parent->p_rchild;

            //Case 1:
            if(p_sibling->color == RBTREE_NODE_RED) {
                p_sibling->color = RBTREE_NODE_BLACK;
                p_parent->color = RBTREE_NODE_RED;
                l_rotate(p_map, p_parent);
            }

            p_sibling = p_parent->p_rchild;

            //Case 2:
            if(p_sibling->color == RBTREE_NODE_BLACK
               && color(p_sibling->p_lchild) == RBTREE_NODE_BLACK
               && color(p_sibling->p_rchild) == RBTREE_NODE_BLACK) {
                p_sibling->color = RBTREE_NODE_RED;
                p_node = p_parent;
                continue;
            }

            //Case 3:
            if(color(p_sibling) == RBTREE_NODE_BLACK
               && color(p_sibling->p_lchild) == RBTREE_NODE_RED) {
                p_sibling->color = RBTREE_NODE_RED;
                p_sibling->p_lchild->color = RBTREE_NODE_BLACK;
                r_rotate(p_map, p_sibling);
                p_sibling = p_parent->p_rchild;
            }

            //Case 4:
            if(color(p_sibling) == RBTREE_NODE_BLACK
               && color(p_sibling->p_rchild) == RBTREE_NODE_RED) {
                l_rotate(p_map, p_parent);
                p_sibling->color = p_parent->color;

                if(p_sibling->p_lchild != NULL) {
                    p_sibling->p_lchild->color = RBTREE_NODE_BLACK;
                }

                if(p_sibling->p_rchild != NULL) {
                    p_sibling->p_rchild->color = RBTREE_NODE_BLACK;
                }

                break;
            }

        } else {
            p_sibling = p_parent->p_lchild;

            //Case 1:
            if(p_sibling->color == RBTREE_NODE_RED) {
                p_sibling->color = RBTREE_NODE_BLACK;
                p_parent->color = RBTREE_NODE_RED;
                r_rotate(p_map, p_parent);
            }

            p_sibling = p_parent->p_lchild;

            //Case 2:
            if(p_sibling->color == RBTREE_NODE_BLACK
               && color(p_sibling->p_lchild) == RBTREE_NODE_BLACK
               && color(p_sibling->p_rchild) == RBTREE_NODE_BLACK) {
                p_sibling->color = RBTREE_NODE_RED;
                p_node = p_parent;
                continue;
            }

            //Case 3:
            if(color(p_sibling) == RBTREE_NODE_BLACK
               && color(p_sibling->p_rchild) == RBTREE_NODE_RED) {
                p_sibling->color = RBTREE_NODE_RED;
                p_sibling->p_rchild->color = RBTREE_NODE_BLACK;
                l_rotate(p_map, p_sibling);
                p_sibling = p_parent->p_lchild;
            }

            //Case 4:
            if(color(p_sibling) == RBTREE_NODE_BLACK
               && color(p_sibling->p_lchild) == RBTREE_NODE_RED) {
                r_rotate(p_map, p_parent);
                p_sibling->color = p_parent->color;

                if(p_sibling->p_lchild != NULL) {
                    p_sibling->p_lchild->color = RBTREE_NODE_BLACK;
                }

                if(p_sibling->p_rchild != NULL) {
                    p_sibling->p_rchild->color = RBTREE_NODE_BLACK;
                }

                break;
            }
        }
    }

    return;
}
