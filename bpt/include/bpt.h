#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

// Default order is 4.
#define DEFAULT_ORDER 4

// Minimum order is necessarily 3.  We set the maximum
// order arbitrarily.  You may change the maximum order.
#define MIN_ORDER 3
#define MAX_ORDER 20

// Constants for printing part or all of the GPL license.
#define LICENSE_FILE "LICENSE.txt"
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625

// TYPES.

/* Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */
typedef struct record {
    int value;
} record;

/* Type representing a node in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */
typedef struct node {
    void ** pointers;
    int * keys;
    struct node * parent;
    bool is_leaf;
    int num_keys;
    struct node * next; // Used for queue.
} node;

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
extern int order;
/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
extern node * queue;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
extern bool verbose_output;

extern FILE * file_db;
//open_db

char * value;

// FUNCTION PROTOTYPES.


int open_db(char *pathname);

//getter and setter
//getter
int64_t getRootPage();
int32_t getNumKeys(int64_t page);
int64_t getFreePage();
int64_t getKeyInLeaf(int64_t page, int i);
void getValue(int64_t page, int i, char *value);
int32_t getIsLeaf(int64_t page);
int64_t getKeyInNode(int64_t page, int i);
int64_t getOffsetInNode(int64_t page, int i);
int64_t getRightSibiling(int64_t page);
int64_t getParentPage(int64_t page);

//setter
void setRootPage(int64_t page);
void setIsLeaf(int64_t page);
void setIsNode(int64_t page);
void setKeyInLeaf(int64_t page, int i, int64_t key);
void setKeyInNode(int64_t page, int i, int64_t key);
void setValue(int64_t page, int i, char * value);
void setNumKeys(int64_t page);
void setRightSibling(int64_t page, int64_t right_sibling_offset);
void setParentPage(int64_t page, int64_t parent_page_offset);
void setNumKeysD(int64_t page);
void setFreePage(int64_t page);
void setOffsetInNode(int64_t page, int i, int64_t offset);



// Output and utility.

void license_notice( void );
void print_license( int licence_part );
void usage_1( void );
void usage_2( void );
void usage_3( void );
void enqueue( node * new_node );
node * dequeue( void );
int height( node * root );
int path_to_root( node * root, node * child );
void print_leaves( node * root );
void print_tree( node * root );
void find_and_print(node * root, int key, bool verbose); 
void find_and_print_range(node * root, int range1, int range2, bool verbose); 
int find_range( node * root, int key_start, int key_end, bool verbose,
        int returned_keys[], void * returned_pointers[]); 
int64_t find_leaf(int64_t root, int64_t key);
char * find(int64_t key);
int cut( int length );

// Insertion.

int64_t insert_into_parent(int64_t root, int64_t left, int64_t key, int64_t right);
int64_t make_node( void ); //record * make_record(int value);
int64_t make_free_page(); //make free page, parent_page_offset = 0, make last 8byte 0
int64_t make_leaf( void ); //is_leaf = 0, # of keys = 0, right_sibling_offset = 0
int get_left_index(int64_t parent, int64_t left);
int64_t insert_into_leaf( int64_t leaf, int64_t key, char *value );
int64_t insert_into_leaf_after_splitting(int64_t root, int64_t leaf, int64_t key,
                                        char * value);
int64_t insert_into_node(int64_t root, int64_t parent, 
        int left_index, int64_t key, int64_t right);
int64_t insert_into_node_after_splitting(int64_t root, int64_t parent,
                                        int left_index,
        int64_t key, int64_t right);
int64_t insert_into_new_root(int64_t left, int64_t key, int64_t right);
int64_t start_new_tree(int key, char* value);
int insert( int64_t key, char* buf );

// Deletion.

int get_neighbor_index( int64_t n );
int64_t adjust_root(int64_t root);
int64_t coalesce_nodes(int64_t n, int64_t neighbor,
                      int neighbor_index, int64_t k_prime);
int64_t redistribute_nodes(int64_t n, int64_t neighbor,
                          int neighbor_index,
        int k_prime_index, int64_t k_prime);
int64_t delete_entry( int64_t n, int64_t key);
int64_t delete( int64_t key );
int64_t remove_entry_from_node(int64_t n, int64_t key);
void destroy_tree_nodes(node * root);
int64_t destroy_tree();

#endif /* __BPT_H__*/
