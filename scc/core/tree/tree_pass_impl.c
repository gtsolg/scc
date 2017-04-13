#include "tree_pass_impl.h"
#include "tree_pass.h"

const tree_insert_dispatch_table_impl std_tree_insert_table =
{
        tree_insert_none_impl,      // tnk_null
        tree_insert_stmt_impl,      // tnk_stmt
        tree_insert_none_impl,      // tnk_id
        tree_insert_decl_impl,      // tnk_type_decl
        tree_insert_decl_impl,      // tnk_var_decl
        tree_insert_func_decl_impl, // tnk_func_decl
        tree_insert_exp_lr_impl,    // tnk_exp
        tree_insert_list_impl,      // tnk_list
        tree_insert_list_node_impl, // tnk_list_node
        tree_insert_none_impl,      // tnk_const_int
        tree_insert_none_impl,      // tnk_const_float
        tree_insert_none_impl,      // tnk_const_real
        tree_insert_none_impl,      // tnk_const_string
        tree_insert_vec_impl,       // tnk_vector_type
        tree_insert_sign_impl,      // tnk_sign_type
        tree_insert_type_impl,      // tnk_type
        tree_insert_none_impl,      // tnk_attrib,
};

extern tree tree_queue_node_create_impl(struct allocator* tree_alloc, tree node, tree entry)
{
        tree n = tree_list_node_create(tree_alloc, node);
        if (!n)
                return NULL;
        tree_queue_node_list_entry(n) = entry;
        return n;
}

extern void tree_queue_add(struct allocator* tree_alloc, tree queue, tree node, tree entry)
{
        tree_list_push_back(queue, tree_queue_node_create_impl(tree_alloc, node, entry));
}

extern void tree_insert_none_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        return;
}

extern void tree_insert_stmt_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        scc_unreachable(); // todo
}

extern void tree_insert_decl_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        scc_unreachable(); // todo
        //tree decl = tree_list_node_base(entry);
        //tree id_node = tree_list_node_create(tree_alloc, tree_decl_id(decl));
        //tree base_node = tree_list_node_create(tree_alloc, tree_decl_base(decl));
        //tree_list_push_back_node(queue, tree_alloc, id_node);
        //tree_list_push_back_node(queue, tree_alloc, base_node);
        //tree_list_node_insert_after(entry, base_node);
        //tree_list_node_insert_after(entry, id_node);
}

extern void tree_insert_func_decl_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        scc_unreachable(); // todo
}

static inline void insert_exp(struct allocator* tree_alloc, tree queue, tree entry, const int lr)
{
        tree exp = tree_list_node_base(entry);
        tree left = tree_exp_left(exp);
        tree right = tree_exp_right(exp);

        tree left_node = tree_list_node_create(tree_alloc, left);
        tree right_node = tree_list_node_create(tree_alloc, right);

        tree_queue_add(tree_alloc, queue, left, left_node);
        tree_queue_add(tree_alloc, queue, right, right_node);

        if (!lr)
                tree_swap(&left_node, &right_node);

        tree_list_node_insert_before(entry, left_node);
        tree_list_node_insert_after(entry, right_node);
}

extern void tree_insert_exp_lr_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        insert_exp(tree_alloc, queue, entry, 1);
}

extern void tree_insert_exp_rl_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        insert_exp(tree_alloc, queue, entry, 0);
}

extern void tree_insert_list_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        scc_unreachable(); // todo
        //tree list = tree_list_node_base(entry);
        //struct tree_iterator it = tree_list_reverse_iterator_init(list);
        //while (tree_list_iterator_valid(&it))
        //{
        //        tree list_node = tree_list_node_create(tree_alloc, tree_iterator_pos(&it));
        //        tree_list_push_back_node(queue, tree_alloc, list_node);
        //        tree_list_node_insert_after(entry, list_node);
        //        tree_list_iterator_rewind(&it);
        //}
}

extern void tree_insert_list_node_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        scc_unreachable(); // todo
        //tree node = tree_list_node_base(entry);
        //tree base_node = tree_list_node_create(tree_alloc, tree_list_node_base(node));
        //tree_list_push_back_node(queue, tree_alloc, base_node);
        //tree_list_node_insert_after(entry, base_node);
}

extern void tree_insert_vec_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        scc_unreachable(); // todo
        //tree vec = tree_list_node_base(entry);
        //tree type_node = tree_list_node_create(tree_alloc, tree_vector_type(vec));
        //tree_list_push_back_node(queue, tree_alloc, type_node);
        //tree_list_node_insert_after(entry, type_node);
}

extern void tree_insert_sign_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        scc_unreachable(); // todo
}

extern void tree_insert_type_impl(struct allocator* tree_alloc, tree queue, tree entry)
{
        tree type = tree_list_node_base(entry);
        tree next = tree_type_next(type);
        tree next_node = tree_list_node_create(tree_alloc, next);
        tree_queue_add(tree_alloc, queue, next, next_node);
        tree_list_node_insert_after(entry, next_node);
}

static inline tree_foreach_init(struct allocator* tree_alloc
        , tree_insert_dispatch_table_impl insert
        , tree_match_fn match, void* match_data
        , tree list, tree node
        , const int clip)
{
        tree_list_initf(list);
        if (clip && !match(node, match_data))
                return;

        union tree_node queue = tree_list_initf(&queue);
        tree first = tree_list_node_create(tree_alloc, node);
        tree_list_push_back(list, first);
        insert[tree_kind(node)](tree_alloc, &queue, first);

        while (!tree_list_empty(&queue))
        {
                tree front = tree_list_pop_front(&queue);
                tree front_node = tree_list_node_base(front);
                tree front_entry = tree_queue_node_list_entry(front);
                tree_delete(tree_alloc, front);

                if (clip && !match(front_node, match_data))
                {
                        tree_list_node_remove(front_entry);
                        tree_delete(tree_alloc, front_entry);
                        continue;
                }
                insert[tree_kind(front_node)](tree_alloc, &queue, front_entry);
        }

        if (clip)
                return;

        struct tree_iterator it = tree_list_forward_iterator_init(&queue);
        while (tree_list_iterator_valid(&it))
        {
                tree node = tree_iterator_pos(&it);
                tree_list_iterator_advance(&it);
                if (!match(tree_list_node_base(node), match_data))
                        tree_delete(tree_alloc, node);
        }
}

extern void tree_foreach_clip_init_impl(struct allocator* tree_alloc
        , tree_insert_dispatch_table_impl insert
        , tree_match_fn match, void* match_data
        , tree list, tree node)
{
        tree_foreach_init(tree_alloc, insert, match, match_data, list, node, 1);
}

extern void tree_foreach_noclip_init_impl(struct allocator* tree_alloc
        , tree_insert_dispatch_table_impl insert
        , tree_match_fn match, void* match_data
        , tree list, tree node)
{
        tree_foreach_init(tree_alloc, insert, match, match_data, list, node, 0);
}

void tree_foreach_dispose_impl(struct allocator* tree_alloc, tree queue)
{
        tree_delete_list_nodes(tree_alloc, queue);
}

void tree_foreach_forward_pass_impl(tree queue, tree_pass_fn pass, void* pass_data)
{
        struct tree_iterator it = tree_list_forward_iterator_init(queue);
        while (tree_list_iterator_valid(&it))
        {
                switch (pass(tree_list_iterator_node_base(&it), pass_data))
                {
                        case PASS_SHOULD_EXIT:
                                return PASS_SHOULD_EXIT;

                        case PASS_SHOULD_CONTINUE:
                                break;

                        case PASS_SHOULD_RESTART:
                                return PASS_SHOULD_RESTART;

                        default:
                                // unknown state
                                scc_unreachable();
                                return PASS_SHOULD_EXIT;
                }
                tree_list_iterator_advance(&it);
        }
        return PASS_SHOULD_EXIT;
}
