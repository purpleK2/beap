#include "beap.h"

beap_memnode_t *root_node = NULL;

beap_maj_t *beap_maj = NULL;

beap_maj_t *beapmajor_init()
{
    return beap_alloc(sizeof(beap_maj_t), NULL);
}

beap_memnode_t *heapnode_init(size_t size)
{
    beap_memnode_t *node;

    size_t actual;
    node = beap_alloc(sizeof(beap_memnode_t) + size, &actual);

    node->magic = HEAPMAGIC_AVAIL;
    node->len = actual - sizeof(beap_memnode_t);
    node->allocated = false;
    node->next = NULL;

    return node;
}

void heap_split_node(beap_memnode_t *node, size_t where)
{
    if (!node)
        return;

    if (!where || node->len <= where)
        return;

    size_t rem = node->len - where;

    beap_memnode_t *new = heapnode_init(rem);

    new->next = node->next;

    node->len -= rem;
    node->next = new;
}

bool heap_check_header(beap_memnode_t *node)
{
    int dead = (node->magic == HEAPMAGIC_UNAV);
    int good = (node->magic == HEAPMAGIC_AVAIL);

    return (dead == 0) || (good == 0);
}

// TODO: print initialization & other debug

void *PREFIX(malloc)(size_t size)
{
    beap_lock();

    if (!beap_maj)
    {
#ifdef BEAP_DEBUG
        beap_debug("Initializing beap version %d.%d\n", HEAPVER_MAJOR,
                   HEAPVER_MINOR);
#endif
        beap_maj = beapmajor_init();

#ifdef BEAP_DEBUG
        beap_debug("Beap maj %p initialized OK\n", beap_maj);
#endif
    }

    if (!root_node)
    {
        root_node = heapnode_init(BEAP_PAGES * BEAP_PAGE);
        beap_maj->root = root_node;
        beap_maj->last_checked = root_node;
#ifdef BEAP_DEBUG
        beap_debug("Root node %p(%#zx) initialized OK\n", beap_maj->root,
                   beap_maj->root->len);
#endif
    }

#ifdef BEAP_DEBUG
    beap_debug("malloc(%#zx)\n", size);
#endif

    beap_memnode_t *cur;
    for (cur = beap_maj->last_checked; cur != NULL; cur = cur->next)
    {
        if (!heap_check_header(cur))
        {
#ifdef BEAP_DEBUG
            beap_debug("Header %#lx is invalid\n", cur->magic);
#endif
            return NULL;
        }

#ifdef BEAP_DEBUG
        beap_debug("Checking node %p(%#zx)\n", cur, cur->len);
#endif

        bool is_magic_available = (cur->magic == HEAPMAGIC_AVAIL);
        bool can_size_fit = (cur->len >= size);

        if (is_magic_available && can_size_fit)
        {
            beap_maj->last_checked = cur;
            break;
        }

#ifdef BEAP_DEBUG
        beap_debug("%p(%#zx) is already allocated\n", cur, cur->len);
#endif

        // here: either it's already allocated or it doesn't fit
        if (!cur->next)
        {
            // we do this IF there's no ->next
            beap_memnode_t *new = heapnode_init(size);
            cur->next = new;
#ifdef BEAP_DEBUG
            beap_debug("New node %p(%#zx) created\n", cur->next,
                       cur->next->len);
#endif
        }
    }

    if (!cur) // (for some unknown reason)
        return NULL;

    cur->magic = HEAPMAGIC_UNAV;

    if (cur->len > size)
    {
        heap_split_node(cur, size);

#ifdef BEAP_DEBUG
        beap_debug("Node %p(%#zx) has been split: ->next=(%p;%#zx)\n", cur,
                   cur->len, cur->next, cur->next->len);
#endif
    }

    cur->allocated = true;

    beap_unlock();

    void *p = HEAP_ALIGN((void *)cur);

    memset(p, 0, size);

#ifdef BEAP_DEBUG
    beap_debug("Return %p\n", p);
#endif

    return p;
}

void PREFIX(free)(void *ptr)
{
#ifdef BEAP_DEBUG
    beap_debug("free(%p)\n", ptr);
#endif

    beap_memnode_t *deallocated = HEAP_UNALIGN(ptr);

    if (!heap_check_header(deallocated))
    {
#ifdef BEAP_DEBUG
        beap_debug("%p doesn't have a valid header\n", ptr);
#endif
        return;
    }

    bool is_magic_allocated = (deallocated->magic == HEAPMAGIC_UNAV);

    if (!is_magic_allocated && !deallocated->allocated)
    {
#ifdef BEAP_DEBUG
        beap_debug("Node %p is not allocated\n", ptr);
#endif
        return;
    }
    beap_lock();

#ifdef BEAP_DEBUG
    beap_debug("Deallocating node %p\n", beap_maj->root);
#endif

    deallocated->magic = HEAPMAGIC_AVAIL;
    deallocated->allocated = false;

    // TODO: coalesce

    // destroy the node
    beap_memnode_t *prev;
    beap_memnode_t *next;
    // is deallocated not the root node?
    if (deallocated != beap_maj->root)
    {
        for (prev = beap_maj->root; prev->next != deallocated;
             prev = prev->next)
            ;
        
        next = deallocated->next;
    }
    else
    {
        // deallocated is maj->root
        prev = NULL;
        next = root_node->next;
    }
#ifdef BEAP_DEBUG
        beap_debug("%p->prev = %p\n", deallocated, prev);
#endif

    if (prev)
    {
#ifdef BEAP_DEBUG
        beap_debug("deallocated(%p)->prev(%p)->next(%p) is now %p\n", deallocated, prev, prev->next, next);
#endif
        prev->next = next;

        beap_maj->last_checked = prev;
    }
    else
    {
#ifdef BEAP_DEBUG
        beap_debug("root(%p) is now %p\n", beap_maj->root,
                   beap_maj->root->next);
#endif
        root_node = root_node->next;
        beap_maj->root = root_node;

        beap_maj->last_checked = root_node;
    }

    size_t tounmap_aligned =
        ROUND_UP(deallocated->len + sizeof(beap_memnode_t), BEAP_PAGE);
    beap_dealloc(deallocated, tounmap_aligned / BEAP_PAGE);
#ifdef BEAP_DEBUG
    beap_debug("node %p deallocated\n", deallocated);
#endif

#ifdef BEAP_DEBUG
    beap_debug("Last checked node is %p(%d)\n", beap_maj->last_checked,
               beap_maj->last_checked->allocated);
#endif

    beap_unlock();

    // we're done :D
}

void *PREFIX(calloc)(size_t times, size_t size)
{
#ifdef BEAP_DEBUG
    beap_debug("calloc(%#zx * %#zx)\n", times, size);
#endif
    return PREFIX(malloc)(times * size);
}

void *PREFIX(realloc)(void *p_old, size_t size)
{
#ifdef BEAP_DEBUG
    beap_debug("realloc(%p, %#zx)\n", p_old, size);
#endif

    void *p_new = PREFIX(malloc)(size);

    void *p_old_realigned = HEAP_UNALIGN(p_old);
    beap_memnode_t *cur;
    for (cur = beap_maj->root; cur != NULL || p_old_realigned != cur;
         cur = cur->next)
        ;

    size_t copy_size = MIN(size, cur->len);

    memcpy(p_new, p_old, copy_size);

    PREFIX(free)(p_old);

    return NULL;
}
