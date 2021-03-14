#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list.h>
#include <stdint.h>

typedef struct __element {
    char *value;
    struct list_head list;
} list_ele_t;


// 傳入參數是 list
// 回傳此 list 中間的 node pointer
static list_ele_t *get_middle(struct list_head *list)
{
    struct list_head *fast = list->next, *slow;
    list_for_each (slow, list) {
        if ( fast->next == list || fast->next->next == list )
            break;
        fast = fast->next->next;
    }
    return list_entry(slow, list_ele_t, list);
}

static void list_merge(struct list_head *lhs,
                       struct list_head *rhs,
                       struct list_head *head)
{
    INIT_LIST_HEAD(head);


    while (!list_empty(lhs) && !list_empty(rhs)) {
        char *lv = list_entry(lhs->next, list_ele_t, list)->value;
        char *rv = list_entry(rhs->next, list_ele_t, list)->value;
        struct list_head *tmp = strcmp(lv, rv) <= 0 ? lhs->next : rhs->next;
        list_del(tmp);
        list_add_tail(tmp, head);
    }
    list_splice_tail(list_empty(lhs) ? rhs : lhs, head);
}

void list_merge_sort(list_ele_t *eleList)
{
    if (list_is_singular(&eleList->list))
        return;

    list_ele_t right;
    INIT_LIST_HEAD(&right.list);
    struct list_head sorted;
    
    /*
     有個新的 right
     取出 eleList 中間的 node，並把此 node  assign 給 right
     example : 
      1.eleList         : a->b->c->d->e->f->g
      2.取出中間的 node   : (d)->e->f->g
      3.assign to right : 
        right   = (d)->e->f->g
        eleList = a->b->c
    */ 
    list_cut_position(&right.list, &eleList->list, &get_middle(&eleList->list)->list);
    
    
    // 將 2 個 list 都排序
    list_merge_sort(&right);
    list_merge_sort(eleList);
    
    
    // 將 2 個 list 合併並 assign to sorted list
    list_merge(&right.list, &eleList->list, &sorted);
    
    
    INIT_LIST_HEAD(&eleList->list);
    list_splice_tail(&sorted, &eleList->list);
}

static bool validate(list_ele_t *eleList)
{
    struct list_head *node;
    list_for_each (node, &eleList->list) {
        if (node->next == &eleList->list)
            break;
        if (strcmp(list_entry(node, list_ele_t, list)->value,
                   list_entry(node->next, list_ele_t, list)->value) > 0)
            return false;
    }
    return true;
}

static void list_show_val(list_ele_t *eleList)
{
    struct list_head *node;
    int i = 0;
    list_for_each (node, &eleList->list) {
        printf("(%2d) %s", ++i, list_entry(node, list_ele_t, list)->value);
    }
}

static void q_free(struct list_head *head)
{
    struct list_head *curr, *next;
    list_for_each_safe(curr, next, head) {
        list_del(curr);
        list_ele_t *target = list_entry(curr, list_ele_t, list);
        free(target->value);
        free(target);
    }
}

bool q_insert_head(list_ele_t *eleList, char *s)
{
    if (!eleList) return false;

    // New node
    list_ele_t *new = malloc(sizeof(list_ele_t));
    if (!new)
        return false;
        
    char *new_value = strdup(s);
    if (!new_value) {
        free(new);
        return false;
    }
    new->value  = new_value;

    // Add New node
    list_add_tail(&new->list, &eleList->list);
    return true;
}

int main(void)
{
    FILE *fp = fopen("cities.txt", "r");  
    if (!fp) {
        perror("failed to open cities.txt");
        exit(EXIT_FAILURE);
    }

    list_ele_t eleList;
    INIT_LIST_HEAD(&eleList.list);

    char buf[256];
    while (fgets(buf, 256, fp))
        q_insert_head(&eleList, buf);
    fclose(fp);

    //list_show_val(&eleList);
    list_merge_sort(&eleList);
    assert(validate(&eleList));
    //list_show_val(&eleList);
    q_free(&eleList.list);
    return 0;
}
