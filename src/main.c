/* 
 *  Description:    Automatization of a pastry's delivery system, order taking system and warehouse inventory
 *                  by writing an efficient code with a reasonably low memory usage.
 * 
 *  Author:         ransomware3301 (https://www.github.com/ransomware3301)
 */


/*------------------------------------------*/
/////////// INCLUDES /////////////////////////
/*------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------*/
/////////// DEFINES //////////////////////////
/*------------------------------------------*/

/*--------- GENERIC DEFINES ----------------*/

#define COMMAND_1 "aggiungi_ricetta"
#define COMMAND_2 "rimuovi_ricetta"
#define COMMAND_3 "rifornimento"
#define COMMAND_4 "ordine"

#define SUCCESS_OUTPUT_1 "aggiunta"
#define SUCCESS_OUTPUT_2 "rimossa"
#define SUCCESS_OUTPUT_3 "rifornito"
#define SUCCESS_OUTPUT_4 "accettato"

#define FAILURE_OUTPUT_1 "ignorato"
#define FAILURE_OUTPUT_2 "ordini in sospeso"
#define FAILURE_OUTPUT_3 "non presente"
#define FAILURE_OUTPUT_4 "rifiutato"
#define FAILURE_OUTPUT_5 "camioncino vuoto"

#define FALSE 0
#define TRUE 1
#define BUFSIZE 255
#define END_OF_STRING '\0'
#define NEWLINE_CHAR '\n'
#define UINT_MAX 0xFFFFFFFF
#define MALLOC_ERROR "ERROR: Memory allocation was unsuccessful"
#define WRONG_SCANF_INPUT_ERROR "ERROR: Couldn't process command"

/*--------- HASHMAP DEFINES ----------------*/

#define HASH_PRIME 67
#define HASHMAP_BLOCK_SIZE 32
#define END_OF_STRING '\0'
#define GOLDEN_RATIO 1.618033989
#define LOAD_FACTOR (GOLDEN_RATIO - 1)

/*------------------------------------------*/
/////////// TYPE DEFINITIONS /////////////////
/*------------------------------------------*/

/*--------- GENERIC TYPES ------------------*/

typedef unsigned char uint8_t;

typedef struct recipe
{
    char *nameptr;
    unsigned int *weights;
    void **buckets;
    size_t weight;
    size_t len;
}
recipe_t;

typedef struct order
{
    recipe_t *recipe;
    unsigned int qty;
    unsigned int timestamp;
    size_t weight;
}
order_t;

typedef struct pallet
{
    unsigned int qty;
    unsigned int expr_date;
}
pallet_t;

/*--------- COOKBOOK -----------------------*/

typedef struct cb_bucket
{
    char *str;
    recipe_t *recipe;
    struct cb_bucket *next;
}
cb_bucket_t;

typedef struct cookbook
{
    cb_bucket_t **buckets;
    size_t size;
    size_t occupied;
    size_t elements;
}
cookbook_t;

/*--------- ORDERS -------------------------*/

typedef struct queue_node
{
    order_t *order;
    struct queue_node *next;
}
queue_node_t;

typedef struct queue
{
    queue_node_t *head;
    queue_node_t *tail;
}
queue_t;

/*--------- WAREHOUSE ----------------------*/

typedef struct heap_node
{
    unsigned int key;
    pallet_t *pallet;
    size_t qleft;
    size_t qright;
    size_t qlastlayer;
    struct heap_node *parent;
    struct heap_node *left;
    struct heap_node *right;
}
heap_node_t;

typedef struct minheap
{
    heap_node_t *root;
    size_t totqty;
}
minheap_t;

typedef struct wh_bucket
{
    char *str;
    minheap_t *heap;
    struct wh_bucket *next;
}
wh_bucket_t;

typedef struct warehouse
{
    wh_bucket_t **buckets;
    size_t size;
    size_t occupied;
    size_t elements;
}
warehouse_t;

/*------------------------------------------*/
/////////// FUNCTION DECLARATIONS ////////////
/*------------------------------------------*/

/*------------- I/O ------------------------*/

void print_queue(queue_t*);
void print_cookbook(cookbook_t*);
void print_2D_heap(heap_node_t*, size_t);
void print_warehouse(warehouse_t*);

/*--------- GENERIC FUNCTIONS --------------*/

size_t      hash(char*, size_t);
void        merge(queue_node_t**, unsigned int, unsigned int, unsigned int);
void        mergesort_orders_array(queue_node_t**, unsigned int, unsigned int);
queue_t *   delivery_truck_load(queue_t*);
uint8_t     enough_ingredients(warehouse_t**, order_t*);

/*--------- RECIPE FUNCTIONS ---------------*/

recipe_t *  create_recipe(warehouse_t**);

/*--------- COOKBOOK FUNCTIONS -------------*/

cookbook_t *  cookbook_init(size_t);
cookbook_t *  cookbook_clear(cookbook_t*);
cookbook_t *  cookbook_insert(cookbook_t*, char*, recipe_t*);
cookbook_t *  cookbook_delete(cookbook_t*, char*);
cookbook_t *  cookbook_extend(cookbook_t*, size_t);
cookbook_t *  cookbook_rehash(cookbook_t*, size_t);
cb_bucket_t * cookbook_search(cookbook_t*, char*);

/*--------- ORDERS FUNCTIONS ---------------*/

order_t *      create_order(recipe_t*, unsigned int, unsigned int);
queue_t *      queue_init();
queue_t *      queue_clear(queue_t*);
queue_t *      queue_add_back(queue_t*, order_t*);
queue_t *      queue_add_in_order(queue_t*, queue_node_t*);
queue_t *      queue_pop_front(queue_t*, order_t**);
queue_node_t * queue_search_order(queue_t*, char*);

/*--------- WAREHOUSE FUNCTIONS ------------*/

pallet_t *    create_pallet(unsigned int, unsigned int);
minheap_t *   minheap_init();
minheap_t *   minheap_insert(minheap_t*, pallet_t*);
minheap_t *   minheap_delete_top(minheap_t*, pallet_t**);
minheap_t *   minheap_clear(minheap_t*);
void          minheap_clear_root(heap_node_t*);
warehouse_t * warehouse_init(size_t);
warehouse_t * warehouse_clear(warehouse_t*);
warehouse_t * warehouse_insert(warehouse_t*, pallet_t*, char*);
warehouse_t * warehouse_insert_section(warehouse_t*, char*);
warehouse_t * warehouse_extend(warehouse_t*, size_t);
warehouse_t * warehouse_rehash(warehouse_t*, size_t);
wh_bucket_t * warehouse_search(warehouse_t*, char*);
warehouse_t * warehouse_delete_expired(warehouse_t*, unsigned int*);
warehouse_t * get_pallets(warehouse_t*, unsigned int*);

/*------------------------------------------*/
/////////// GLOBAL VARIABLES /////////////////
/*------------------------------------------*/

// Timekeeping variable
unsigned int global_time_counter = 0;

// Delivery truck Parameters
unsigned int delivery_truck_period, delivery_truck_capacity;

// Flag for first initialization of "requireds" static 
// array used in function "enough_ingredients()"
uint8_t requireds_init = FALSE;

/*------------------------------------------*/
/////////// MAIN /////////////////////////////
/*------------------------------------------*/

int main(int argc, char **argv)
{
    cookbook_t *cookbook;
    warehouse_t *warehouse;
    queue_t *orders_ready, *orders_wait;
    queue_node_t *prev, *curr;
    cb_bucket_t *recipe_bucket;
    unsigned int next_expr_date;
    order_t *new_order;
    char *buf1;
    size_t tmp;
    char c;


    if (scanf("%u %u\n", &delivery_truck_period, &delivery_truck_capacity) == 2)
    {
        // Initialization
        cookbook = cookbook_init(HASHMAP_BLOCK_SIZE);
        warehouse = warehouse_init(HASHMAP_BLOCK_SIZE);
        orders_ready = queue_init();
        orders_wait = queue_init();
        next_expr_date = UINT_MAX;

        if (( buf1 = (char*)malloc(sizeof(char) * (BUFSIZE + 1)) ))
        {
            // Initialization
            *(buf1 + BUFSIZE) = END_OF_STRING;

            // Main loop
            while ( !feof(stdin) )
            {
                if (scanf(" %s ", buf1) == 1)
                {
                    if (strcmp(buf1, COMMAND_1) == 0)
                    {
                        //////// (1) aggiungi_ricetta ////////

                        if (scanf(" %s ", buf1) == 1)
                        {
                            if (cookbook_search(cookbook, buf1) == NULL)
                            {
                                cookbook = cookbook_insert(
                                    cookbook, 
                                    buf1,
                                    create_recipe(&warehouse)
                                );

                                printf("%s\n", SUCCESS_OUTPUT_1);
                            }
                            else
                            {
                                while (scanf("%c", &c) == 1 && c != NEWLINE_CHAR);
                                printf("%s\n", FAILURE_OUTPUT_1);
                            }
                        }
                        else
                        {
                            printf("[main() -> aggiungi_ricetta] %s\n", WRONG_SCANF_INPUT_ERROR);
                        }
                    }
                    else if (strcmp(buf1, COMMAND_2) == 0)
                    {
                        //////// (2) rimuovi_ricetta ////////

                        if (scanf(" %s ", buf1) == 1)
                        {
                            if (cookbook_search(cookbook, buf1) != NULL)
                            {
                                if (
                                    queue_search_order(orders_ready, buf1) == NULL
                                    &&
                                    queue_search_order(orders_wait, buf1) == NULL
                                )
                                {
                                    cookbook = cookbook_delete(cookbook, buf1);

                                    printf("%s\n", SUCCESS_OUTPUT_2);
                                }
                                else
                                {
                                    printf("%s\n", FAILURE_OUTPUT_2);
                                }
                            }
                            else
                            {
                                printf("%s\n", FAILURE_OUTPUT_3);
                            }
                        }
                        else
                        {
                            printf("[main() -> rimuovi_ricetta] %s\n", WRONG_SCANF_INPUT_ERROR);
                        }
                    }
                    else if (strcmp(buf1, COMMAND_3) == 0)
                    {
                        //////// (3) rifornimento ////////

                        warehouse = get_pallets(warehouse, &next_expr_date);

                        if (warehouse && warehouse->elements > 0 && orders_wait->head && orders_wait->tail)
                        {
                            prev = NULL;
                            curr = orders_wait->head;

                            while (curr)
                            {
                                if (enough_ingredients(&warehouse, curr->order) == TRUE)
                                {
                                    if (curr == orders_wait->head)
                                    {
                                        orders_wait->head = orders_wait->head->next;
                                        orders_ready = queue_add_in_order(orders_ready, curr);
                                        curr = orders_wait->head;
                                    }
                                    else if (curr == orders_wait->tail)
                                    {
                                        prev->next = NULL;
                                        orders_wait->tail = prev;
                                        orders_ready = queue_add_in_order(orders_ready, curr);
                                        curr = NULL;
                                    }
                                    else
                                    {
                                        prev->next = curr->next;
                                        orders_ready = queue_add_in_order(orders_ready, curr);
                                        curr = prev->next;
                                    }
                                }
                                else
                                {
                                    prev = curr;
                                    curr = curr->next;
                                }
                            }
                        }

                        printf("%s\n", SUCCESS_OUTPUT_3);
                    }
                    else if (strcmp(buf1, COMMAND_4) == 0)
                    {
                        //////// (4) ordine ////////

                        if (scanf(" %s %lu ", buf1, &tmp) == 2)
                        {
                            recipe_bucket = cookbook_search(cookbook, buf1);

                            if (recipe_bucket && recipe_bucket->recipe && tmp > 0)
                            {
                                new_order = create_order(recipe_bucket->recipe, tmp, global_time_counter);

                                if (enough_ingredients(&warehouse, new_order) == TRUE)
                                {
                                    orders_ready = queue_add_back(
                                        orders_ready,
                                        new_order
                                    );
                                }
                                else
                                {
                                    orders_wait = queue_add_back(
                                        orders_wait,
                                        new_order
                                    );
                                }

                                printf("%s\n", SUCCESS_OUTPUT_4);
                            }
                            else
                            {
                                printf("%s\n", FAILURE_OUTPUT_4);
                            }
                        }
                        else
                        {
                            printf("[main() -> ordine] %s\n", WRONG_SCANF_INPUT_ERROR);
                        }
                    }
                    else
                    {
                        printf("[main()] ERROR: Specified command \"%s\" does not exist\n", buf1);
                    }   
                }
                
                global_time_counter++;

                // Delivery truck routine
                // When it's his time to pass, it has a higher priority than any other command
                if (global_time_counter % delivery_truck_period == 0)
                {
                    orders_ready = delivery_truck_load(orders_ready);
                }

                // Expired pallets removal
                if (next_expr_date == global_time_counter)
                {
                    warehouse = warehouse_delete_expired(warehouse, &next_expr_date);
                }

                /*
                // Debug
                printf("\n===== COOKBOOK =====\n");
                print_cookbook(cookbook);
                printf("\n===== WAREHOUSE =====\n");
                print_warehouse(warehouse);
                printf("\n===== ORDERS_READY =====\n");
                print_queue(orders_ready);
                printf("\n===== ORDERS_WAIT =====\n");
                print_queue(orders_wait);
                printf("\n\n<<<<<<<<<<<<<<<< TIME: %u >>>>>>>>>>>>>>>>\n\n\n\n\n", global_time_counter);
                */
            }

            // Freeing memory
            free(buf1);
            cookbook = cookbook_clear(cookbook);
            warehouse = warehouse_clear(warehouse);
            orders_ready = queue_clear(orders_ready);
            orders_wait = queue_clear(orders_wait);
        }
        else
        {
            printf("[main() -> buf1] %s\n", MALLOC_ERROR);
        }
    }
    else
    {
        printf("[main()] ERROR: Delivery truck couldn't be initialized\n");
    }


    return 0;
}

/*------------------------------------------*/
/////////// FUNCTION DEFINITIONS /////////////
/*------------------------------------------*/



/*------------- I/O ------------------------*/

/*
 *  Prints the given queue
 */
void print_queue(queue_t *queue)
{
    queue_node_t *ptr;


    if (queue && queue->head && queue->tail)
    {
        printf("\n[QUEUE]\n");

        ptr = queue->head;

        while (ptr)
        {
            printf(" |--> (recipe_name=\"%s\", qty=%u, t=%u)\n", ptr->order->recipe->nameptr, ptr->order->qty, ptr->order->timestamp);
            ptr = ptr->next;
        }
    }
}

/*
 *  Prints the given cookbook
 */
void print_cookbook(cookbook_t *cookbook)
{
    cb_bucket_t *ptr;
    size_t i, j;


    if (cookbook)
    {
        printf("\n[COOKBOOK]\n");

        for (i = 0; i < cookbook->size; i++)
        {
            ptr = *(cookbook->buckets + i);

            if (ptr)
            {
                printf(" |--> ");

                while (ptr && ptr->next)
                {
                    printf("[recipe_name=\"%s\"] --> ", ptr->recipe->nameptr);

                    for (j = 0; j < ptr->recipe->len - 1; j++)
                    {
                        printf("(\"%s\", %u)--", 
                            (*(((wh_bucket_t**) ptr->recipe->buckets) + j))->str, 
                            (*(ptr->recipe->weights + j))
                        );
                    }
                   
                    printf("(\"%s\", %u)\n", 
                        (*(((wh_bucket_t**) ptr->recipe->buckets) + j))->str, 
                        (*(ptr->recipe->weights + j))
                    );

                    ptr = ptr->next;

                    if (ptr)
                    {
                        printf("      ");
                    }
                }

                printf("[recipe_name=\"%s\"] --> ", ptr->recipe->nameptr);

                for (j = 0; j < ptr->recipe->len - 1; j++)
                {
                    printf("(\"%s\", %u)--", 
                        (*(((wh_bucket_t**) ptr->recipe->buckets) + j))->str, 
                        (*(ptr->recipe->weights + j))
                    );
                }
                
                printf("(\"%s\", %u)\n", 
                    (*(((wh_bucket_t**) ptr->recipe->buckets) + j))->str, 
                    (*(ptr->recipe->weights + j))
                );
            }
        }
    }
}

/*
 *  Prints the given heap in 2D
 */
void print_2D_heap(heap_node_t *root, size_t depth)
{
    size_t i;


    if (root)
    {
        print_2D_heap(root->right, ++depth);
        depth--;

        for (i = 0; i < depth; i++)
        {
            printf("\t");
        }

        if (root->parent == NULL)
        {
            printf("(r) ");
        }

        printf("[key=%u, qleft=%lu, qright=%lu, qlastlayer=%lu]\n", root->key, root->qleft, root->qright, root->qlastlayer);
        
        for (i = 0; i < depth; i++)
        {
            printf("\t");
        }

        printf("(qty=%u, expr_date=%u)", 
            root->pallet->qty,
            root->pallet->expr_date
        );

        printf("\n\n");
        
        print_2D_heap(root->left, ++depth);
    }
}

/*
 *  Prints the given warehouse
 */
void print_warehouse(warehouse_t *warehouse)
{
    wh_bucket_t *ptr;
    size_t i, j;


    if (warehouse && warehouse->buckets)
    {
        printf("\n[WAREHOUSE]\n");

        for (i = 0; i < warehouse->size; i++)
        {
            ptr = (*(warehouse->buckets + i));

            if (ptr)
            {
                j = 0;

                while (ptr)
                {
                    printf(" |-----[bucketNUM=%lu, heapNUM=%lu]--> (heap_ingr=\"%s\")\n\n", i, j, ptr->str);
                    
                    if (ptr->heap != NULL)
                    {
                        print_2D_heap(ptr->heap->root, 1);
                    }
                    else
                    {
                        printf("\t\t[NULL]\n");
                    }
                    
                    ptr = ptr->next;
                    j++;
                }
            }
            /*
            else
            {
                printf(" |---> [NULL]\n");
            }
            */   
        }

        printf("\n");
    }
}

/*------------------------------------------*/



/*--------- GENERIC FUNCTIONS --------------*/

/*
 *  Hash Function Type: Polynomial Rolling Hash Function
 *
 *  Calculates the hash of the given string and gets
 *  shortened to the addressable slots in the hashmap
 *  represented by the parameter size
 *
 *  Time Complexity:    O(n)
 */
size_t hash(char *str, size_t size)
{
    size_t hash, prime_pow, i;


    hash = 0;
    prime_pow = 1;

    if (str)
    {
        i = 0;

        while (*(str + i) != '\0')
        {
            /*
             *  Rebasing the ASCII 7-bit table to the character SPACE (HEX=0x20) such
             *  that we get the following mapping of exclusively printable characters:  
             * 
             *  (0x20) SPACE --MAP--> 1
             *  (0x21) !     --MAP--> 2
             *  (0x22) "     --MAP--> 3 
             *  etc...
             */

            hash = (hash + ((*(str + i) - 0x20 + 1) * prime_pow)) % size;
            prime_pow = (prime_pow * HASH_PRIME) % size ;
            i++;
        }
    }

    return hash;
}

/* 
 *  Merge subroutine needed for mergesort_orders_array()
 *
 *  Time Complexity:    O(n)
 */
void merge(queue_node_t **orders_array, unsigned int p, unsigned int q, unsigned int r)
{
    queue_node_t **left, **right;
    queue_node_t boundary_order;
    order_t tmp_order;
    size_t len1, len2;
    unsigned int i, j, k;


    if (orders_array)
    {
        len1 = q - p + 1;
        len2 = r - q;

        if (
            (left = (queue_node_t**)malloc(sizeof(queue_node_t*) * (len1 + 1)))
            && (right = (queue_node_t**)malloc(sizeof(queue_node_t*) * (len2 + 1)))
        )
        {
            for (i = 0; i < len1; i++)
            {
                *(left + i) = *(orders_array + i + p);
            }

            for (i = 0; i < len2; i++)
            {
                *(right + i) = *(orders_array + i + (q + 1));
            }

            tmp_order.recipe = NULL;
            tmp_order.qty = 0;
            tmp_order.timestamp = UINT_MAX;
            tmp_order.weight = 0;
            boundary_order.order = &tmp_order;
            boundary_order.next = NULL;

            *(left + len1) = &boundary_order;
            *(right + len2) = &boundary_order;

            i = 0;
            j = 0;

            for (k = p; k <= r; k++)
            {
                if (
                    *(left + i) && *(right + j)
                    &&
                    (*(left + i))->order->weight >= (*(right + j))->order->weight
                )
                {
                    *(orders_array + k) = *(left + i);
                    i++;
                }
                else
                {
                    *(orders_array + k) = *(right + j);
                    j++;
                }
            }

            free(left);
            free(right);
        }
        else
        {
            printf("[merge() -> left || right] ERROR: Memory allocation was unsuccessful\n");
        }
    }
}

/*
 *  Sorts the orders ready queue by total order weight
 *
 *  Time Complexity:    O(nlog(n))
 */
void mergesort_orders_array(queue_node_t **orders_array, unsigned int p, unsigned int r)
{
    unsigned int q;
    queue_node_t *tmp;


    if (orders_array)
    {
        if (r > 0 && p < r - 1)
        {
            q = (p + r) / 2;

            mergesort_orders_array(orders_array, p, q);
            mergesort_orders_array(orders_array, q + 1, r);
            merge(orders_array, p, q, r);
        }
        else
        {
            // NOTE: The orders_array is ordeded in descending order so that
            //       the print of these orders is correct
            if ((*(orders_array + p))->order->weight < (*(orders_array + r))->order->weight)
            {
                tmp = *(orders_array + r);
                *(orders_array + r) = *(orders_array + p);
                *(orders_array + p) = tmp;
            }
        }
    } 
}

/*
 *  Loads the delivery truck with the earliest orders in the orders_ready list, and
 *  also they are loaded from heaviest to lightest
 *  (NOTE: The orders_ready queue is already chronologically ordered by construction)
 */
queue_t * delivery_truck_load(queue_t *orders_ready)
{
    queue_node_t **orders_array;
    queue_node_t *ptr;
    order_t *tmp_order;
    unsigned int len, i;
    int tmp_cap;


    if (orders_ready && orders_ready->head && orders_ready->tail)
    {
        ptr = orders_ready->head;   
        len = 0;
        tmp_order = NULL;

        while (ptr)
        {
            len++;
            ptr = ptr->next;
        }

        if (( orders_array = (queue_node_t**)malloc(sizeof(queue_node_t*) * len) ))
        {   
            tmp_cap = delivery_truck_capacity;
            ptr = orders_ready->head;
            i = 0;

            tmp_cap -= ptr->order->weight;
            
            while (tmp_cap >= 0 && ptr)
            {   
                *(orders_array + i) = ptr;
                i++;
                ptr = ptr->next;

                if (ptr)
                {
                    tmp_cap -= ptr->order->weight;
                }
            }

            len = i;
            i = 0;

            mergesort_orders_array(orders_array, 0, len - 1);

            while (i < len)
            {
                printf("%u %s %u\n",
                    (*(orders_array + i))->order->timestamp,
                    (*(orders_array + i))->order->recipe->nameptr,
                    (*(orders_array + i))->order->qty
                );

                i++;
            }

            i = 0;

            while (i < len)
            {   
                orders_ready = queue_pop_front(orders_ready, &tmp_order);
                free(tmp_order);
                i++;
            }

            free(orders_array);
        }
        else
        {   
            printf("[delivery_truck_load() -> orders_array] %s\n", MALLOC_ERROR);
        }
    }
    else
    {
        printf("%s\n", FAILURE_OUTPUT_5);
    }

    return orders_ready;
}

/*
 *  Returns TRUE if the given order can be prepared, meaning that
 *  the warehouse has enough ingredients to prepare it, FALSE otherwise.
 *  In case it can be prepared, the function also subtracts the required
 *  resources to fulfill such order.
 */
uint8_t enough_ingredients(warehouse_t **whptr, order_t *order)
{
    wh_bucket_t *bucket_ptr;
    pallet_t *tmp_pallet;
    size_t i, required;
    static size_t *requireds;
    static size_t curr_len;
    uint8_t is_enough;

    
    if (whptr && *whptr && order && order->recipe)
    {
        // Global variable needed for first initialization
        if (requireds_init == FALSE)
        {
            curr_len = order->recipe->len;
            requireds = (size_t*)malloc(sizeof(size_t) * curr_len);

            if (requireds != NULL)
            {
                requireds_init = TRUE;
            }
        }
        else if (order->recipe->len > curr_len)
        {
            curr_len = order->recipe->len;
            free(requireds);
            requireds = (size_t*)malloc(sizeof(size_t) * curr_len);
        }

        if (requireds != NULL)
        {
            is_enough = TRUE;
            i = 0;

            while (
                (i < order->recipe->len) 
                && 
                (*(((wh_bucket_t**) order->recipe->buckets) + i))->heap->totqty >= *(order->recipe->weights + i) * order->qty 
            )
            {
                *(requireds + i) = *(order->recipe->weights + i) * order->qty;
                i++;
            }

            if (i != order->recipe->len)
            {
                is_enough = FALSE;
            }

            if (is_enough)
            {
                i = 0;
                
                while (i < order->recipe->len)
                {
                    required = (*(requireds + i));
                    bucket_ptr = (*(((wh_bucket_t**) order->recipe->buckets) + i));

                    while (required > 0)
                    {
                        if (bucket_ptr->heap->root->pallet->qty > required)
                        {
                            bucket_ptr->heap->root->pallet->qty -= required;     
                            bucket_ptr->heap->totqty -= required;
                            required = 0;
                        }
                        else
                        {
                            bucket_ptr->heap = minheap_delete_top(
                                bucket_ptr->heap,
                                &tmp_pallet
                            );

                            required -= tmp_pallet->qty;
                            free(tmp_pallet);
                        }
                    }

                    i++;
                }
            }
        }
        else
        {
            printf("[enough_ingredients() -> requireds] %s\n", MALLOC_ERROR);
            is_enough = FALSE;
        }
    }
    else
    {
        is_enough = FALSE;
    }

    return is_enough;
}

/*------------------------------------------*/



/*--------- RECIPE FUNCTIONS ---------------*/

/*
 *  Creates a recipe_t object with the given name, then reads from stdin the
 *  required weights for each ingredient and also creates a wh_bucket_t object 
 *  for each ingredient in the recipe
 */
recipe_t * create_recipe(warehouse_t **warehouse)
{
    recipe_t *new_recipe;
    size_t curr_stream_pos, assigned_args, i;
    size_t start_stream_pos;
    wh_bucket_t *bucket_ptr;
    unsigned int val;
    char *buf;
    char c;


    if (warehouse)
    {
        if ((*warehouse) == NULL)
        {
            (*warehouse) = warehouse_init(HASHMAP_BLOCK_SIZE);
        }

        if ((*warehouse))
        {
            if (( new_recipe = (recipe_t*)malloc(sizeof(recipe_t)) ))
            {
                if (( buf = (char*)malloc(sizeof(char) * (BUFSIZE + 1)) ))
                {
                    // Initializing temporary values
                    *(buf + BUFSIZE) = END_OF_STRING;
                    start_stream_pos = ftell(stdin);
                    curr_stream_pos = start_stream_pos;
                    c = getchar();

                    new_recipe->len = 0;
                    new_recipe->weight = 0;

                    if (c != NEWLINE_CHAR)
                    {
                        // Calculating the amount of ingredients in the 
                        // recipe from data read from stdin
                        do
                        {
                            fseek(stdin, curr_stream_pos, SEEK_SET);
                            assigned_args = scanf("%s %u", buf, &val);
                            curr_stream_pos = ftell(stdin);
                            c = getchar();

                            if (assigned_args == 2 && val > 0)
                            {
                                new_recipe->weight += val;
                                new_recipe->len++;
                            }
                        } 
                        while (assigned_args == 2 && c != NEWLINE_CHAR);
                    }

                    if (new_recipe->len > 0)
                    {
                        // If the recipe is comprised of at least one ingredient, then
                        // initialize the arrays wh_buckets and weights to track, both of
                        // length equal to the previously calculated new_recipe->len variable
                        if (( new_recipe->buckets = (void**)malloc(sizeof(wh_bucket_t*) * new_recipe->len) ))
                        {
                            if (( new_recipe->weights = (unsigned int*)malloc(sizeof(unsigned int) * new_recipe->len) ))
                            {
                                curr_stream_pos = start_stream_pos;
                                i = 0;

                                do
                                {
                                    fseek(stdin, curr_stream_pos, SEEK_SET);
                                    assigned_args = scanf("%s %u", buf, &val);
                                    curr_stream_pos = ftell(stdin);
                                    c = getchar();

                                    if (assigned_args == 2 && val > 0)
                                    {
                                        bucket_ptr = warehouse_search((*warehouse), buf);

                                        if (bucket_ptr == NULL)
                                        {
                                            (*warehouse) = warehouse_insert_section((*warehouse), buf);
                                            bucket_ptr = warehouse_search((*warehouse), buf);
                                        }

                                        *((wh_bucket_t**) new_recipe->buckets + i) = bucket_ptr;
                                        *(new_recipe->weights + i) = val;
                                        i++;
                                    }
                                } 
                                while (assigned_args == 2 && c != NEWLINE_CHAR);
                            }
                            else
                            {
                                printf("[create_recipe() -> new_recipe->weights] %s\n", MALLOC_ERROR);
                                free(new_recipe->buckets);
                                free(new_recipe);
                                new_recipe = NULL;
                            }
                        }
                        else
                        {
                            printf("[create_recipe() -> new_recipe->buckets] %s\n", MALLOC_ERROR);
                            free(new_recipe);
                            new_recipe = NULL;
                        }
                    }
                    else
                    {
                        printf("[create_recipe() -> new_recipe->len] ERROR: Resulting recipe length is 0\n");
                        free(new_recipe);
                        new_recipe = NULL;
                    }
                    
                    free(buf);
                }
                else
                {
                    printf("[create_recipe() -> buf] %s\n", MALLOC_ERROR);
                    free(new_recipe);
                    new_recipe = NULL;
                }
            }
            else
            {
                printf("[create_recipe() -> new_recipe] %s\n", MALLOC_ERROR);
            }
        }
        else
        {
            printf("[create_recipe() -> name] ERROR: Given name is NULL\n");
            new_recipe = NULL;
        }

        return new_recipe;
    }

    return NULL;
}

/*------------------------------------------*/



/*--------- COOKBOOK FUNCTIONS -------------*/

/*
 *  Initializes a cookbook element
 */
cookbook_t * cookbook_init(size_t size)
{
    cookbook_t *cookbook;
    size_t i;


    if (( cookbook = (cookbook_t*)malloc(sizeof(cookbook_t)) ))
    {
        if (( cookbook->buckets = (cb_bucket_t**)malloc(sizeof(cb_bucket_t*) * size) ))
        {
            cookbook->size = size;
            cookbook->occupied = 0;
            cookbook->elements = 0;

            // Initializing each bucket to NULL
            for (i = 0; i < cookbook->size; i++)
            {
                (*(cookbook->buckets + i)) = NULL;
            }
        }
        else
        {
            printf("[cookbook_init() -> cookbook->buckets] %s\n", MALLOC_ERROR);
            free(cookbook);
            cookbook = NULL;
        }
    }
    else
    {
        printf("[cookbook_init() -> cookbook] %s\n", MALLOC_ERROR);
    }

    return cookbook;
}

/*
 *  Clears the entire given cookbook
 */
cookbook_t * cookbook_clear(cookbook_t *cookbook)
{
    cb_bucket_t *ptr, *del;
    size_t i;


    if (cookbook)
    {
        i = 0;

        while (i < cookbook->size)
        {
            ptr = *(cookbook->buckets + i);
            
            while (ptr)
            {
                del = ptr;
                ptr = ptr->next;
                free(del->str);
                free(del->recipe->buckets);
                free(del->recipe->weights);
                free(del->recipe);
                free(del);
            }

            i++;
        }

        free(cookbook->buckets);
        free(cookbook);
        cookbook = NULL;
    }

    return cookbook;
}

/*
 *  Inserts a new recipe in the cookbook
 */
cookbook_t * cookbook_insert(cookbook_t *cookbook, char *name, recipe_t *recipe)
{
    cb_bucket_t *new_cb_bucket;
    size_t hashval;


    if (cookbook && name && recipe)
    {
        if (( new_cb_bucket = (cb_bucket_t*)malloc(sizeof(cb_bucket_t)) ))
        {
            if (( new_cb_bucket->str = (char*)malloc(sizeof(char) * (strlen(name) + 1)) ))
            {
                strcpy(new_cb_bucket->str, name);
                new_cb_bucket->recipe = recipe;

                // Storing the pointer to the cb_bucket's name 
                // (a.k.a. the actual recipe name).
                // This is done to avoid calling "cookbook_search()"" many 
                // times just to discover the actual recipe's name.
                recipe->nameptr = new_cb_bucket->str;

                // If the load factor is surpassed, then the hashmap is extended,
                // which also triggers the rehashing of all elements
                if ((((double) cookbook->occupied) / cookbook->size) > LOAD_FACTOR)
                {
                    cookbook = cookbook_extend(cookbook, HASHMAP_BLOCK_SIZE);
                }

                hashval = hash(new_cb_bucket->str, cookbook->size);

                if ((*(cookbook->buckets + hashval)) == NULL)
                {
                    cookbook->occupied++;
                }

                // Adding the current element to the count of all   
                // the stored element in the given hashmap
                cookbook->elements++;

                // Pushing the element in the bucket_t list
                new_cb_bucket->next = (*(cookbook->buckets + hashval));
                (*(cookbook->buckets + hashval)) = new_cb_bucket;
            }
            else
            {
                printf("[cookbook_insert() -> new_cb_bucket->str] %s\n", MALLOC_ERROR);
                free(new_cb_bucket);
            }
        }
        else
        {
            printf("[cookbook_insert() -> new_cb_bucket] %s\n", MALLOC_ERROR);
        }
    }

    return cookbook;
}

/*
 *  Deletes the recipe from the given cookbook that has the given name
 */
cookbook_t * cookbook_delete(cookbook_t* cookbook, char *name)
{
    cb_bucket_t *prev, *curr;
    size_t hashval;


    if (cookbook && cookbook->elements > 0 && name)
    {
        hashval = hash(name, cookbook->size);
        prev = NULL;
        curr = *(cookbook->buckets + hashval);

        while (curr && strcmp(curr->str, name) != 0)
        {
            prev = curr;
            curr = curr->next;
        }

        if (curr)
        {
            if (prev)
            {
                prev->next = curr->next;

                free(curr->str);
                free(curr->recipe->buckets);
                free(curr->recipe->weights);
                free(curr->recipe);
                free(curr);
            }
            else
            {
                *(cookbook->buckets + hashval) = curr->next;

                free(curr->str);
                free(curr->recipe->buckets);
                free(curr->recipe->weights);
                free(curr->recipe);
                free(curr);
            }

            cookbook->elements--;
        }
    }
    
    return cookbook;
}

/*
 *  Extends the given cookbook hashmap
 */
cookbook_t * cookbook_extend(cookbook_t *cookbook, size_t size)
{
    if (size > 0)
    {
        if (cookbook)
        {
            cookbook = cookbook_rehash(cookbook, cookbook->size + size);
        }
        else
        {
            cookbook = cookbook_init(size);
        }
    }

    return cookbook;
}

/*
 *  Creates a new cookbook hashmap of the given size and
 *  rehashes all the old recipes in the new cookbook.
 */
cookbook_t * cookbook_rehash(cookbook_t *old_cookbook, size_t new_size)
{
    cookbook_t *new_cookbook;
    cb_bucket_t *ptr, *move;
    size_t i, hashval;


    if (old_cookbook && new_size > 0)
    {
        if (( new_cookbook = cookbook_init(new_size)) )
        {
            for (i = 0; i < old_cookbook->size; i++)
            {
                ptr = *(old_cookbook->buckets + i);

                while (ptr)
                {
                    hashval = hash(ptr->str, new_cookbook->size);

                    move = ptr;
                    ptr = ptr->next;
                    
                    move->next = (*(new_cookbook->buckets + hashval));
                    (*(new_cookbook->buckets + hashval)) = move;
                }
            }

            free(old_cookbook->buckets);
            free(old_cookbook);

            return new_cookbook;
        }
    }
    
    return old_cookbook;
}

/*
 *  Searches the given recipe name in the cookbook, returns the address
 *  of the cb_bucket_t element that holds it if found, NULL otherwise
 */
cb_bucket_t * cookbook_search(cookbook_t *cookbook, char *recipe_name)
{
    cb_bucket_t *ptr;


    if (cookbook && recipe_name)
    {
        ptr = *(cookbook->buckets + hash(recipe_name, cookbook->size));

        while (ptr && strcmp(recipe_name, ptr->str) != 0)
        {
            ptr = ptr->next;
        }
    }
    else
    {
        printf("[cookbook_search()] ERROR: Given cookbook and/or recipe_name is/are NULL\n");
        ptr = NULL;
    }

    return ptr;
}

/*------------------------------------------*/



/*--------- ORDERS FUNCTIONS ---------------*/

/*
 *  Creates an order_t object from the given data
 */
order_t * create_order(recipe_t *recipe, unsigned int qty, unsigned int timestamp)
{
    order_t *new_order;


    if (recipe && qty > 0)
    {
        if (( new_order = (order_t*)malloc(sizeof(order_t)) ))
        {
            new_order->recipe = recipe;
            new_order->qty = qty;
            new_order->timestamp = timestamp;
            new_order->weight = qty * recipe->weight;
        }
        else
        {
            printf("[create_order() -> new_order] %s\n", MALLOC_ERROR);
        }
    }   
    else
    {
        printf("[create_order() -> recipe || qty] ERROR: Given recipe is NULL or given qty is <= 0\n");
        new_order = NULL;
    }

    return new_order;
}

/*
 *  Initializes a queue element
 */
queue_t * queue_init()
{
    queue_t *queue;


    if (( queue = (queue_t*)malloc(sizeof(queue_t)) ))
    {
        queue->head = NULL;
        queue->tail = NULL;
    }
    else
    {
        printf("[queue_init() -> queue] %s\n", MALLOC_ERROR);
    }

    return queue;
}

/*
 *  Clears the given queue
 */
queue_t * queue_clear(queue_t *queue)
{
    queue_node_t *del, *ptr;


    if (queue)
    {
        ptr = queue->head;

        while (ptr)
        {
            del = ptr;
            ptr = ptr->next;
            free(del->order);
            free(del);
        }

        free(queue);
        queue = NULL;
    }

    return queue;
}

/*
 *  Appends the given order to the back of the queue
 */
queue_t * queue_add_back(queue_t *queue, order_t *order)
{
    queue_node_t *new_order;


    if (order && queue)
    {
        if (( new_order = (queue_node_t*)malloc(sizeof(queue_node_t)) ))
        {
            new_order->order = order;
            new_order->next = NULL;

            if (queue->head && queue->tail)
            {
                queue->tail->next = new_order;
                queue->tail = new_order;
            }
            else
            {
                queue->head = queue->tail = new_order;
            }
        }
        else
        {
            printf("[queue_push() -> new_order] %s\n", MALLOC_ERROR);
        }
    }

    return queue;
}

/*
 *  Adds in order of timestampt the given node in the given queue
 */
queue_t * queue_add_in_order(queue_t *queue, queue_node_t *node)
{
    queue_node_t *prev, *curr;


    if (node && queue)
    {
        if (queue->head == NULL && queue->tail == NULL)
        {
            queue->head = queue->tail = node;
            node->next = NULL;  
        }
        else
        {
            prev = NULL;
            curr = queue->head;

            while (curr && curr->order->timestamp < node->order->timestamp)
            {
                prev = curr;
                curr = curr->next;
            }

            if (curr)
            {
                if (prev)
                {
                    prev->next = node;
                    node->next = curr;
                }
                else
                {
                    node->next = queue->head;
                    queue->head = node;
                }
            }
            else
            {
                prev->next = node;
                node->next = NULL;
                queue->tail = node;
            }
        }
    }

    return queue;
}

/*
 *  Pops the first element at the head of the given queue
 */
queue_t * queue_pop_front(queue_t *queue, order_t **order)
{
    queue_node_t *del;

    if (order && queue && queue->head && queue->tail)
    {
        del = queue->head;
        queue->head = queue->head->next;

        if (queue->head == NULL)
        {
            queue->tail = NULL;
        }

        (*order) = del->order;
        free(del);
    }

    return queue;
}

/*
 *  Returns the address of the given queue's node that
 *  contains the order with the given name, NULL otherwise
 */
queue_node_t * queue_search_order(queue_t *queue, char *name)
{
    queue_node_t *ptr;


    if (queue && queue->head)
    {
        ptr = queue->head;

        while (ptr && strcmp(name, ptr->order->recipe->nameptr) != 0)
        {
            ptr = ptr->next;
        }
    }   
    else
    {
        ptr = NULL;
    }

    return ptr;
}

/*------------------------------------------*/



/*--------- WAREHOUSE FUNCTIONS ------------*/

/*
 *  Creates a pallet_t object from the given data
 */
pallet_t * create_pallet(unsigned int qty, unsigned int expr_date)
{
    pallet_t *new_pallet;

    
    if (qty > 0 && expr_date > global_time_counter)
    {
        if (( new_pallet = (pallet_t*)malloc(sizeof(pallet_t)) ))
        {
            new_pallet->qty = qty;
            new_pallet->expr_date = expr_date;
        }
        else
        {
            printf("[create_pallet() -> new_pallet] %s\n", MALLOC_ERROR);
        }
    }
    else
    {
        new_pallet = NULL;
    }

    return new_pallet;
}

/*
 *  Initializes a minheap_t object
 */
minheap_t * minheap_init()
{
    minheap_t *minheap;

    if (( minheap = (minheap_t*)malloc(sizeof(minheap_t)) ))
    {
        minheap->root = NULL;
        minheap->totqty = 0;
    }

    return minheap;
}

/*
 *  Inserts a pallet in the given min-heap
 */
minheap_t * minheap_insert(minheap_t *minheap, pallet_t *pallet)
{
    heap_node_t *item, *prev, *curr;
    pallet_t *tmp_pallet;
    unsigned int tmp;
    uint8_t pos;


    if (minheap && pallet)
    {
        if (( item = (heap_node_t*)malloc(sizeof(heap_node_t)) ))
        {
            item->key = pallet->expr_date;
            item->pallet = pallet;
            item->qleft = 0;
            item->qright = 0;
            item->qlastlayer = 2;
            item->left = NULL;
            item->right = NULL;

            minheap->totqty += pallet->qty;

            if (minheap->root == NULL)
            {
                item->parent = NULL;
                minheap->root = item;
            }
            else
            {
                prev = NULL;
                curr = minheap->root;
                pos = 0;

                // Inserting the element at the right place  
                while (curr)
                {
                    prev = curr;

                    if (curr->qleft == curr->qright && curr->left > 0)
                    {
                        prev->qlastlayer += prev->qlastlayer;
                    }
                    
                    if (curr->qleft < curr->qlastlayer - 1)
                    {
                        curr->qleft++;
                        curr = curr->left;
                        pos = 0;
                    }
                    else
                    {
                        curr->qright++;
                        curr = curr->right;
                        pos = 1;
                    }
                }

                // Adjusting the child-parent links
                item->parent = prev;

                if (pos == 0)
                {
                    prev->left = item;
                }
                else
                {
                    prev->right = item;
                }

                // Adjusting the keys (waterfall fashion)
                // From the inserted leaf, the keys will fall down like water in a waterfall
                // until the given key is bigger/smaller than the current one
                curr = item->parent;
                prev = item;

                // MIN_HEAP: From leaves to roots, the keys are descending
                while (curr && prev->key < curr->key)
                {
                    // Swapping the key
                    tmp = curr->key;
                    curr->key = prev->key;
                    prev->key = tmp;

                    // Swapping the pallet
                    tmp_pallet = curr->pallet;
                    curr->pallet = prev->pallet;
                    prev->pallet = tmp_pallet;

                    prev = curr;
                    curr = curr->parent;
                }
            }
        }
        else
        {
            printf("[minheap_insert() -> item] %s\n", MALLOC_ERROR);
        }
    }

    return minheap;
}

/*
 *  Deletes the pallet at the top (root) of the given heap
 */
minheap_t * minheap_delete_top(minheap_t *minheap, pallet_t **pallet)
{
    heap_node_t *prev, *curr;
    uint8_t pos;


    if (minheap && minheap->root && minheap->root->parent == NULL)
    {
        (*pallet) = minheap->root->pallet;

        minheap->totqty -= minheap->root->pallet->qty;

        pos = 0;
        prev = NULL;
        curr = minheap->root;

        while (curr)
        {
            prev = curr;

            if (curr->left && curr->right)
            {
                // Case 1:  Confronting the next two keys is needed to 
                //          estabilish the correct key as the new root

                if (curr->left->key < curr->right->key)
                {
                    curr->key = curr->left->key;
                    curr->pallet = curr->left->pallet;
                    curr->qleft--;
                    curr = curr->left;
                    pos = 0;
                }
                else
                {
                    curr->key = curr->right->key;
                    curr->pallet = curr->right->pallet;
                    curr->qright--;
                    curr = curr->right;
                    pos = 1;
                }
            }
            else if (curr->left && curr->right == NULL)
            {
                // Case 2:  There is only the left branch, the next 
                //          key just shift by one position upwards

                curr->key = curr->left->key;
                curr->pallet = curr->left->pallet;
                curr->qleft--;
                curr = curr->left;
                pos = 0;
            }
            else if (curr->left == NULL && curr->right)
            {
                // Case 3:  There is only the right branch, the next 
                //          key just shift by one position upwards

                curr->key = curr->right->key;
                curr->pallet = curr->right->pallet;
                curr->qright--;
                curr = curr->right;
                pos = 1;
            }
            else
            {
                // Case 4:  The key waterfall has reached the leaves, thus
                //          it's time to delete prev, which is now pointing to the
                //          leaf that was pointed by curr before being set to NULL
                
                curr = NULL;
            }

            if (curr && curr->qleft == curr->qright && curr->left > 0)
            {
                curr->qlastlayer /= 2;
            }
        }

        // Adjusting child-parent links
        // NOTE: prev points to the leaf to be deleted
        if (prev->parent)
        {
            if (pos == 0)
            {
                prev->parent->left = NULL;
            }
            else
            {
                prev->parent->right = NULL;
            }
        }
        else
        {
            minheap->root = NULL;
        }

        // Freeing memory
        free(prev);
    }

    return minheap;
}

/*
 *  Clears the given min-heap completely
 */
minheap_t * minheap_clear(minheap_t *minheap)
{
    if (minheap)
    {
        if (minheap->root && minheap->root->parent == NULL)
        {
            minheap_clear_root(minheap->root);
        }

        free(minheap);
        minheap = NULL;
    }

    return minheap;
}

/*
 *  Clears the given root, corresponding to a min-heap, of
 *  all its nodes leaving just the minheap_t "shell" in memory
 */
void minheap_clear_root(heap_node_t *root)
{
    if (root)
    {
        minheap_clear_root(root->left);
        minheap_clear_root(root->right);
        free(root->pallet);
        free(root);
    }
}

/*
 *  Initializes a warehouse element
 */
warehouse_t * warehouse_init(size_t size)
{
    warehouse_t *warehouse;
    size_t i;


    if (( warehouse = (warehouse_t*)malloc(sizeof(warehouse_t)) ))
    {
        if (( warehouse->buckets = (wh_bucket_t**)malloc(sizeof(wh_bucket_t*) * size) ))
        {
            warehouse->size = size;
            warehouse->occupied = 0;
            warehouse->elements = 0;

            // Initializing each bucket to NULL
            for (i = 0; i < warehouse->size; i++)
            {
                (*(warehouse->buckets + i)) = NULL;
            }
        }
        else
        {
            printf("[warehouse_init() -> warehouse->buckets] %s\n", MALLOC_ERROR);
            free(warehouse);
            warehouse = NULL;
        }
    }
    else
    {
        printf("[warehouse_init() -> warehouse] %s\n", MALLOC_ERROR);
    }

    return warehouse;
}

/*
 *  Clears the entire given cookbook
 */
warehouse_t * warehouse_clear(warehouse_t *warehouse)
{
    wh_bucket_t *ptr, *del;
    size_t i;


    if (warehouse)
    {
        i = 0;

        while (i < warehouse->size)
        {
            ptr = *(warehouse->buckets + i);
            
            while (ptr)
            {
                del = ptr;
                ptr = ptr->next;
                del->heap = minheap_clear(del->heap);
                free(del->str);
                free(del);
            }

            i++;
        }

        free(warehouse->buckets);
        free(warehouse);
        warehouse = NULL;
    }

    return warehouse;
}

/*
 *  Adds a new pallet to the warehouse
 */
warehouse_t * warehouse_insert(warehouse_t *warehouse, pallet_t *pallet, char *ingredient)
{
    wh_bucket_t *ptr, *new_heap_bucket;
    size_t hashval;
    char *copy;


    if (warehouse && pallet && ingredient)
    {
        if (( ptr = warehouse_search(warehouse, ingredient) ))
        {
            ptr->heap = minheap_insert(
                ptr->heap,
                pallet
            );
        }
        else
        {
            if (( new_heap_bucket = (wh_bucket_t*)malloc(sizeof(wh_bucket_t)) ))
            {
                if (( copy = (char*)malloc(sizeof(char) * (strlen(ingredient) + 1)) ))
                {
                    strcpy(copy, ingredient);

                    new_heap_bucket->str = copy;
                    new_heap_bucket->heap = minheap_init();
                    new_heap_bucket->heap = minheap_insert(
                        new_heap_bucket->heap,
                        pallet
                    );

                    // If the load factor is surpassed, then the hashmap is extended,
                    // which also triggers the rehashing of all elements
                    if ((((double) warehouse->occupied) / warehouse->size) > LOAD_FACTOR)
                    {
                        warehouse = warehouse_extend(warehouse, HASHMAP_BLOCK_SIZE);
                    }

                    hashval = hash(copy, warehouse->size);

                    if ((*(warehouse->buckets + hashval)) == NULL)
                    {
                        warehouse->occupied++;
                    }

                    // Adding the current element to the count of all   
                    // the stored element in the given hashmap
                    warehouse->elements++;

                    // Pushing the element in the bucket_t list
                    new_heap_bucket->next = *(warehouse->buckets + hashval);
                    *(warehouse->buckets + hashval) = new_heap_bucket;
                }
                else
                {
                    printf("[warehouse_insert() -> copy] %s\n", MALLOC_ERROR);
                    free(new_heap_bucket);
                }
            }   
            else
            {
                printf("[warehouse_insert() -> item] %s\n", MALLOC_ERROR);
            }
        }
    }

    return warehouse;
}

/*
 *  Adds an empty section to the warehouse
 */
warehouse_t * warehouse_insert_section(warehouse_t *warehouse, char *ingredient)
{
    wh_bucket_t *new_section;
    size_t hashval;
    char *copy;


    if (warehouse && ingredient)
    {
        if (( new_section = (wh_bucket_t*)malloc(sizeof(wh_bucket_t)) ))
        {
            if (( copy = (char*)malloc(sizeof(char) * (strlen(ingredient) + 1)) ))
            {
                strcpy(copy, ingredient);

                new_section->str = copy;
                new_section->heap = minheap_init();

                // If the load factor is surpassed, then the hashmap is extended,
                // which also triggers the rehashing of all elements
                if ((((double) warehouse->occupied) / warehouse->size) > LOAD_FACTOR)
                {
                    warehouse = warehouse_extend(warehouse, HASHMAP_BLOCK_SIZE);
                }

                hashval = hash(copy, warehouse->size);

                if ((*(warehouse->buckets + hashval)) == NULL)
                {
                    warehouse->occupied++;
                }

                // Adding the current element to the count of all   
                // the stored element in the given hashmap
                warehouse->elements++;

                // Pushing the element in the bucket_t list
                new_section->next = *(warehouse->buckets + hashval);
                *(warehouse->buckets + hashval) = new_section;
            }
            else
            {
                printf("[warehouse_insert_section() -> copy] %s\n", MALLOC_ERROR);
                free(new_section);
            }
        }
        else
        {
            printf("[warehouse_insert_section() -> new_section] %s\n", MALLOC_ERROR);
        }
    }

    return warehouse;
}

/*
 *  Extends the given warehouse hashmap by the given size amount
 */
warehouse_t * warehouse_extend(warehouse_t *warehouse, size_t size)
{
    if (size > 0)
    {
        if (warehouse)
        {
            warehouse = warehouse_rehash(warehouse, warehouse->size + size);
        }
        else
        {
            warehouse = warehouse_init(size);
        }
    }

    return warehouse;
}

/*
 *  Creates a new warehouse hashmap of the given size and
 *  rehashes all the old pallets in the new warehouse.
 */
warehouse_t * warehouse_rehash(warehouse_t *old_warehouse, size_t new_size)
{
    warehouse_t *new_warehouse;
    wh_bucket_t *ptr, *move;
    size_t i, hashval;

    if (old_warehouse && new_size > 0)
    {
        if (( new_warehouse = warehouse_init(new_size) ))
        {
            for (i = 0; i < old_warehouse->size; i++)
            {
                ptr = *(old_warehouse->buckets + i);

                while (ptr)
                {
                    hashval = hash(ptr->str, new_size);

                    move = ptr;
                    ptr = ptr->next;

                    move->next = *(new_warehouse->buckets + hashval);
                    *(new_warehouse->buckets + hashval) = move;
                }
            }

            free(old_warehouse->buckets);
            free(old_warehouse);

            return new_warehouse;
        }
    }

    return old_warehouse;
}

/*
 *  Returns the address of the bucket containing the ingredient
 *  and the corresponding pallet heap, NULL otherwise
 */
wh_bucket_t * warehouse_search(warehouse_t *warehouse, char *ingredient)
{
    wh_bucket_t *ptr;


    if (warehouse && ingredient)
    {   
        ptr = *(warehouse->buckets + hash(ingredient, warehouse->size));

        while (ptr && strcmp(ptr->str, ingredient) != 0)
        {
            ptr = ptr->next;
        }
    }
    else
    {
        ptr = NULL;
    }

    return ptr;
}

/*
 *  Deletes expired pallets from the existing wh_buckets
 */
warehouse_t * warehouse_delete_expired(warehouse_t *warehouse, unsigned int *next_expr_date)
{
    wh_bucket_t *bucket_ptr;
    pallet_t *pallet;
    size_t i;
    

    if (warehouse && next_expr_date)
    {
        *(next_expr_date) = UINT_MAX;
        i = 0;

        while (i < warehouse->size)
        {
            bucket_ptr = *(warehouse->buckets + i);

            while (bucket_ptr)
            {
                while (bucket_ptr->heap->root && bucket_ptr->heap->root->pallet->expr_date == global_time_counter)
                {
                    bucket_ptr->heap = minheap_delete_top(bucket_ptr->heap, &pallet);
                    free(pallet);
                }

                if (bucket_ptr && bucket_ptr->heap->root)
                {
                    if (bucket_ptr->heap->root->pallet->expr_date < *(next_expr_date))
                    {
                        *(next_expr_date) = bucket_ptr->heap->root->pallet->expr_date;
                    }
                }

                bucket_ptr = bucket_ptr->next;
            }

            i++;
        }
    }

    return warehouse;
}

/*
 *  Inserts newly created pallets with data read from stdin
 */
warehouse_t * get_pallets(warehouse_t *warehouse, unsigned int *next_expr_date)
{
    unsigned int val1, val2;
    unsigned int curr_stream_pos, assigned_args;
    wh_bucket_t *ptr;
    char *buf;
    char c;


    if (warehouse == NULL)
    {
        warehouse = warehouse_init(HASHMAP_BLOCK_SIZE);
    }

    if (( buf = (char*)malloc(sizeof(char) * (BUFSIZE + 1)) ))
    {
        *(buf + BUFSIZE) = END_OF_STRING;

        curr_stream_pos = ftell(stdin);
        c = getchar();

        if (c != NEWLINE_CHAR)
        {
            do
            {
                fseek(stdin, curr_stream_pos, SEEK_SET);
                assigned_args = scanf("%s %u %u", buf, &val1, &val2);
                curr_stream_pos = ftell(stdin);
                c = getchar();

                if (assigned_args == 3 && val1 > 0 && val2 > global_time_counter)
                {
                    ptr = warehouse_search(warehouse, buf);

                    if (ptr == NULL)
                    {
                        warehouse = warehouse_insert_section(warehouse, buf);
                        ptr = warehouse_search(warehouse, buf);
                    }

                    if (val2 < *(next_expr_date))
                    {
                        *(next_expr_date) = val2;
                    }

                    ptr->heap = minheap_insert(
                        ptr->heap,
                        create_pallet(val1, val2)
                    );
                }
            }
            while (assigned_args == 3 && c != NEWLINE_CHAR);
        }

        free(buf);
    }
    else
    {
        printf("[get_pallets() ->buf] %s\n", MALLOC_ERROR);
    }

    return warehouse;
}

/*------------------------------------------*/
