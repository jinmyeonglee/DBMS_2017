/*
 *  bpt.c  
 */
#define Version "1.14"
/*
 *
 *  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation 
 *  and/or other materials provided with the distribution.
 
 *  3. Neither the name of the copyright holder nor the names of its 
 *  contributors may be used to endorse or promote products derived from this 
 *  software without specific prior written permission.
 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 
 *  Author:  Amittai Aviram 
 *    http://www.amittai.com
 *    amittai.aviram@gmail.edu or afa13@columbia.edu
 *  Original Date:  26 June 2010
 *  Last modified: 17 June 2016
 *
 *  This implementation demonstrates the B+ tree data structure
 *  for educational purposes, includin insertion, deletion, search, and display
 *  of the search path, the leaves, or the whole tree.
 *  
 *  Must be compiled with a C99-compliant C compiler such as the latest GCC.
 *
 *  Usage:  bpt [order]
 *  where order is an optional argument
 *  (integer MIN_ORDER <= order <= MAX_ORDER)
 *  defined as the maximal number of pointers in any node.
 *
 */

#include "bpt.h"

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
int leaf_order = 32;
int node_order = 249;
FILE * file_db;
int fd;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
node * queue = NULL;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
bool verbose_output = false;

//value = (char*)malloc(sizeof(char) * 120);


// FUNCTION DEFINITIONS.

// OUTPUT AND UTILITIES

/* Copyright and license notice to user at startup. 
 */
void license_notice( void ) {
    printf("bpt version %s -- Copyright (C) 2010  Amittai Aviram "
            "http://www.amittai.com\n", Version);
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details "
            "type `show w'.\n"
            "This is free software, and you are welcome to redistribute it\n"
            "under certain conditions; type `show c' for details.\n\n");
}


/* Routine to print portion of GPL license to stdout.
 */
void print_license( int license_part ) {
    int start, end, line;
    FILE * fp;
    char buffer[0x100];

    switch(license_part) {
    case LICENSE_WARRANTEE:
        start = LICENSE_WARRANTEE_START;
        end = LICENSE_WARRANTEE_END;
        break;
    case LICENSE_CONDITIONS:
        start = LICENSE_CONDITIONS_START;
        end = LICENSE_CONDITIONS_END;
        break;
    default:
        return;
    }

    fp = fopen(LICENSE_FILE, "r");
    if (fp == NULL) {
        perror("print_license: fopen");
        exit(EXIT_FAILURE);
    }
    for (line = 0; line < start; line++)
        fgets(buffer, sizeof(buffer), fp);
    for ( ; line < end; line++) {
        fgets(buffer, sizeof(buffer), fp);
        printf("%s", buffer);
    }
    fclose(fp);
}


/* First message to the user.
 */
void usage_1( void ) {
    //printf("B+ Tree of Order %d.\n", order);
    printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
           "5th ed.\n\n"
           "To build a B+ tree of a different order, start again and enter "
           "the order\n"
           "as an integer argument:  bpt <order>  ");
    printf("(%d <= order <= %d).\n", MIN_ORDER, MAX_ORDER);
    printf("To start with input from a file of newline-delimited integers, \n"
           "start again and enter the order followed by the filename:\n"
           "bpt <order> <inputfile> .\n");
}


/* Second message to the user.
 */
void usage_2( void ) {
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <k>  -- Insert <k> (an integer) as both key and value).\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\tp <k> -- Print the path from the root to key k and its associated "
           "value.\n"
    "\tr <k1> <k2> -- Print the keys and values found in the range "
            "[<k1>, <k2>\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    "\tx -- Destroy the whole tree.  Start again with an empty tree of the "
           "same order.\n"
    "\tt -- Print the B+ tree.\n"
    "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
    "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and "
           "leaves.\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help message.\n");
}


/* Brief usage note.
 */
void usage_3( void ) {
    printf("Usage: ./bpt [<order>]\n");
    printf("\twhere %d <= order <= %d .\n", MIN_ORDER, MAX_ORDER);
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
void enqueue( node * new_node ) {
    node * c;
    if (queue == NULL) {
        queue = new_node;
        queue->next = NULL;
    }
    else {
        c = queue;
        while(c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
        new_node->next = NULL;
    }
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
node * dequeue( void ) {
    node * n = queue;
    queue = queue->next;
    n->next = NULL;
    return n;
}


/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
/*void print_leaves( node * root ) {
    int i;
    node * c = root;
    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    while (!c->is_leaf)
        c = c->pointers[0];
    while (true) {
        for (i = 0; i < c->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)c->pointers[i]);
            printf("%d ", c->keys[i]);
        }
        if (verbose_output)
            //printf("%lx ", (unsigned long)c->pointers[order - 1]);
        if (c->pointers[order - 1] != NULL) {
            printf(" | ");
            c = c->pointers[order - 1];
        }
        else
            break;
    }
    printf("\n");
}*?


/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */
int height( node * root ) {
    int h = 0;
    node * c = root;
    while (!c->is_leaf) {
        c = c->pointers[0];
        h++;
    }
    return h;
}


/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
int path_to_root( node * root, node * child ) {
    int length = 0;
    node * c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
}


/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
/*void print_tree( node * root ) {

    node * n = NULL;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    queue = NULL;
    enqueue(root);
    while( queue != NULL ) {
        n = dequeue();
        if (n->parent != NULL && n == n->parent->pointers[0]) {
            new_rank = path_to_root( root, n );
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        if (verbose_output) 
            printf("(%lx)", (unsigned long)n);
        for (i = 0; i < n->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)n->pointers[i]);
            printf("%d ", n->keys[i]);
        }
        if (!n->is_leaf)
            for (i = 0; i <= n->num_keys; i++)
                enqueue(n->pointers[i]);
        if (verbose_output) {
            if (n->is_leaf) 
                printf("%lx ", (unsigned long)n->pointers[order - 1]);
            else
                printf("%lx ", (unsigned long)n->pointers[n->num_keys]);
        }
        printf("| ");
    }
    printf("\n");
}*/

// open_db
int open_db(char *pathname) {
    file_db = fopen(pathname, "r+");
    fflush(file_db);
    if(file_db == NULL) { //create new file
        file_db = fopen(pathname, "w+");
        fflush(file_db);
        fseek(file_db, 0, SEEK_SET);
        int64_t offs[3];
        offs[0] = 0; offs[1] = 4096; offs[2] = 2;
        fwrite(offs, 8, 3, file_db);
        fseek(file_db, 4096, SEEK_SET);
        int64_t leaf_h = 0;//root's parent page offset
        fwrite(&leaf_h, 8, 1, file_db);
        int32_t is_leaf = 1; //is leaf & # of keys
        int32_t num_keys = 0;
        fseek(file_db, 4096 + 8, SEEK_SET);
        fwrite(&is_leaf, 4, 1, file_db);
        fwrite(&num_keys, 4, 1, file_db);
        fseek(file_db, 120 + 4096, SEEK_SET);
        fwrite(&leaf_h, 8, 1, file_db);
    }

    if(file_db == NULL) {
        printf("Can't make file\n");
    }
    fd = fileno(file_db);
    value = (char *) malloc(sizeof(char) * 120);
    return 0;
}
//getter and setter

int64_t getRootPage() {
    int64_t root_page_offset;
    fseek(file_db, 8, SEEK_SET);
    fread(&root_page_offset, 8, 1, file_db);
    return root_page_offset;
}

int64_t getFreePage() {
    int64_t free_page_offset;
    fseek(file_db, 0, SEEK_SET);
    fread(&free_page_offset, 8, 1, file_db);
    return free_page_offset;
}

int32_t getNumKeys(int64_t page) {
    int32_t num_keys;
    fseek(file_db, page + 12, SEEK_SET);
    fread(&num_keys, 4, 1, file_db);
    return num_keys;
}

int64_t getKeyInLeaf(int64_t page, int i) {
    int64_t key;
    fseek(file_db, page + 128 + i * 128, SEEK_SET);
    fread(&key, 8, 1, file_db);
    return key;
}
void getValue(int64_t page, int i, char *value) {
    fseek(file_db, page + 128 + i * 128 + 8, SEEK_SET);
    fread(value, 120, 1, file_db);
}

int32_t getIsLeaf(int64_t page) {
    int32_t is_leaf;
    fseek(file_db, page + 8, SEEK_SET);
    fread(&is_leaf, 4, 1, file_db);
    return is_leaf;
}

int64_t getKeyInNode(int64_t page, int i) {
    int64_t key;
    fseek(file_db, page + 128 + i * 16, SEEK_SET);
    fread(&key, 8, 1, file_db);
    return key;
}

int64_t getOffsetInNode(int64_t page, int i) {
    int64_t offset;
    if(i == -1) {
        fseek(file_db, page + 120, SEEK_SET);
        fread(&offset, 8, 1, file_db);
    }
    else {
        fseek(file_db, page + 128 + i * 16 + 8, SEEK_SET);
        fread(&offset, 8, 1, file_db); }
    return offset;
}

int64_t getRightSibiling(int64_t page) {
    int64_t right_sibling_offset;
    fseek(file_db, page + 120, SEEK_SET);
    fread(&right_sibling_offset, 8, 1, file_db);
    return right_sibling_offset;
}

int64_t getParentPage(int64_t page) {
    int64_t parent_page_offset;
    fseek(file_db, page, SEEK_SET);
    fread(&parent_page_offset, 8, 1, file_db);
    return parent_page_offset;
}

void setRootPage(int64_t page) {
    fseek(file_db, 8, SEEK_SET);
    fwrite(&page, 8, 1, file_db);
}

void setParentPage(int64_t page, int64_t parent_page_offset) {
    fseek(file_db, page, SEEK_SET);
    fwrite(&parent_page_offset, 8, 1, file_db);
}

void setRightSibling(int64_t page, int64_t right_sibling_offset) {
    fseek(file_db, page + 120, SEEK_SET);
    fwrite(&right_sibling_offset, 8, 1, file_db);
}

void setIsLeaf(int64_t page) {
    int32_t is_leaf = 1;
    int32_t num_keys = 0;
    int64_t right_sibling_offset = 0;
    fseek(file_db, page + 8, SEEK_SET);
    fwrite(&is_leaf, 4, 1, file_db);
    fwrite(&num_keys, 4, 1, file_db);

    fseek(file_db, page + 120, SEEK_SET);
    fwrite(&right_sibling_offset, 8, 1, file_db);
}

void setIsNode(int64_t page) {
    int32_t is_leaf = 0;
    int32_t num_keys = 0;
    fseek(file_db, page + 8, SEEK_SET);
    fwrite(&is_leaf, 4, 1, file_db);
    fwrite(&num_keys, 4, 1, file_db);
}

void setKeyInLeaf(int64_t page, int i, int64_t key) {
    fseek(file_db, page + 128 + i * 128, SEEK_SET);
    fwrite(&key, 8, 1, file_db);
}

void setKeyInNode(int64_t page, int i, int64_t key) {
    fseek(file_db, page + 128 + i * 16, SEEK_SET);
    fwrite(&key, 8, 1, file_db);
}

void setOffsetInNode(int64_t page, int i, int64_t offset) {
    if(i == -1) {
        fseek(file_db, page + 120, SEEK_SET);
        fwrite(&offset, 8, 1, file_db);
    }
    else {
        fseek(file_db, page + 128 + i * 16 + 8, SEEK_SET);
        fwrite(&offset, 8, 1, file_db);
    }
}

void setValue(int64_t page, int i, char * value) {
    fseek(file_db, page + 128 + i * 128 + 8, SEEK_SET);
    fwrite(value, 120, 1, file_db);
}

void setNumKeys(int64_t page) {
    int32_t num_keys = getNumKeys(page);
    num_keys++;
    fseek(file_db, page + 12, SEEK_SET);
    fwrite(&num_keys, 4, 1, file_db);
}

void setNumKeysD(int64_t page) {
    int32_t num_keys = getNumKeys(page);
    num_keys--;
    fseek(file_db, page + 12, SEEK_SET);
    fwrite(&num_keys, 4, 1, file_db);
}

void setFreePage(int64_t page) {
    int64_t next_free_page;
    fseek(file_db, 0, SEEK_SET);
    fread(&next_free_page, 8, 1, file_db);
    fseek(file_db, 0, SEEK_SET);
    fwrite(&page, 8, 1, file_db);
    fseek(file_db, page, SEEK_SET);
    fwrite(&next_free_page, 8, 1, file_db);
}



/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
/*void find_and_print(node * root, int key, bool verbose) {
    record * r = find(root, key, verbose);
    if (r == NULL)
        printf("Record not found under key %d.\n", key);
    else 
        printf("Record at %lx -- key %d, value %d.\n",
                (unsigned long)r, key, r->value);
}*/


/* Finds and prints the keys, pointers, and values within a range
 * of keys between key_start and key_end, including both bounds.
 */

/*void find_and_print_range( node * root, int key_start, int key_end,
        bool verbose ) {
    int i;
    int array_size = key_end - key_start + 1;
    int returned_keys[array_size];
    void * returned_pointers[array_size];
    int num_found = find_range( root, key_start, key_end, verbose,
            returned_keys, returned_pointers );
    if (!num_found)
        printf("None found.\n");
    else {
        for (i = 0; i < num_found; i++)
            printf("Key: %d   Location: %lx  Value: %d\n",
                    returned_keys[i],
                    (unsigned long)returned_pointers[i],
                    ((record *)
                     returned_pointers[i])->value);
    }
}*/


/* Finds keys and their pointers, if present, in the range specified
 * by key_start and key_end, inclusive.  Places these in the arrays
 * returned_keys and returned_pointers, and returns the number of
 * entries found.
 */
/*int find_range( node * root, int key_start, int key_end, bool verbose,
        int returned_keys[], void * returned_pointers[]) {
    int i, num_found;
    num_found = 0;
    node * n = find_leaf( root, key_start, verbose );
    if (n == NULL) return 0;
    for (i = 0; i < n->num_keys && n->keys[i] < key_start; i++) ;
    if (i == n->num_keys) return 0;
    while (n != NULL) {
        for ( ; i < n->num_keys && n->keys[i] <= key_end; i++) {
            returned_keys[num_found] = n->keys[i];
            returned_pointers[num_found] = n->pointers[i];
            num_found++;
        }
        n = n->pointers[order - 1];
        i = 0;
    }
    return num_found;
}
*/

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
int64_t find_leaf(int64_t root, int64_t key) {
    int i = 0;
    int64_t c = root;
    //printf("c : %ld\n", c);
    if (c == 0) {
        /*if (verbose) 
            printf("Empty tree.\n");*/
        return c;
    }
    while (getIsLeaf(c) != 1) {
        /*if (verbose) {
            printf("[");
            for (i = 0; i < c->num_keys - 1; i++)
                printf("%d ", c->keys[i]);
            printf("%d] ", c->keys[i]);
        }*/
        i = 0;
        while (i < getNumKeys(c)) {
            if (key >= getKeyInNode(c, i)) i++;
            else break;
        }
        /*if (verbose)
            printf("%d ->\n", i);*/
        c = getOffsetInNode(c, i - 1);
        //printf("in find_leaf index %d, %ld c\n", i - 1, c);
    }
    /*if (verbose) {
        printf("Leaf [");
        for (i = 0; i < c->num_keys - 1; i++)
            printf("%d ", c->keys[i]);
        printf("%d] ->\n", c->keys[i]);
    }*/
    return c;
}


/* Finds and returns the record to which
 * a key refers.
 */
char * find(int64_t key) {
    int i = 0;
    //char value[120];////////////////////////////////////////
    int64_t root = getRootPage();
    int64_t c = find_leaf(root, key);
    int32_t num_keys = getNumKeys(c);
    if (c == 0) return NULL;
    for (i = 0; i < getNumKeys(c); i++)
        if (getKeyInLeaf(c, i) == key) break;
    if (i == getNumKeys(c))
        return NULL;
    else{
        getValue(c, i, value);
        return value;
    }
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}


// INSERTION

/* Creates a new record to hold the value
 * to which a key refers.
 */
/*record * make_record(int value) {
    record * new_record = (record *)malloc(sizeof(record));
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        new_record->value = value;
    }
    return new_record;
}
*/


/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
int64_t make_node( void ) {
    int64_t new_node;
    new_node = make_free_page();
    //new_node = malloc(sizeof(node));
    if (new_node == 0) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }
    setIsNode(new_node);
    return new_node;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
// first maek free page
int64_t make_free_page() {  //increase file size +4096 and return the page offset
    int64_t temp = 0;
    int64_t next_free_page;
    	if(getFreePage() == 0) {
    		fseek(file_db, 0, SEEK_END);
    		fwrite(&temp, 8, 1, file_db);

    		fseek(file_db, 4080, SEEK_END);
	    	fwrite(&temp, 8, 1, file_db);

    		temp = ftell(file_db) - 4096;
    		return temp;
	}
	else {
		temp = getFreePage();
		fseek(file_db, temp, SEEK_SET);
		fread(&next_free_page, 8, 1, file_db);
		setFreePage(next_free_page);
		return temp;
	}
}

int64_t make_leaf( void ) {
    int64_t leaf = make_free_page();
    setIsLeaf(leaf);
    return leaf;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int get_left_index(int64_t parent, int64_t left) {

    int left_index = -1;
    while (left_index < getNumKeys(parent) && getOffsetInNode(parent, left_index) != left)
        left_index++;
    return left_index;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int64_t insert_into_leaf( int64_t leaf, int64_t key, char * value ) {

    int i, insertion_point;
    int32_t num_keys = getNumKeys(leaf);
    char temp_value[120];

    insertion_point = 0;
    while (insertion_point < getNumKeys(leaf) && getKeyInLeaf(leaf, insertion_point) < key)
        insertion_point++;

    for (i = getNumKeys(leaf); i > insertion_point; i--) {
        setKeyInLeaf(leaf, i, getKeyInLeaf(leaf, i -1));
        getValue(leaf, i - 1, temp_value);
        setValue(leaf, i, temp_value);
    }
    setKeyInLeaf(leaf, insertion_point, key);
    setValue(leaf, insertion_point, value);
    setNumKeys(leaf);
    return leaf;
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
int64_t insert_into_leaf_after_splitting(int64_t root, int64_t leaf, int64_t key, char * value) {

    int64_t new_leaf;
    int32_t num_keys;
    int64_t * temp_keys;
    char temp_value[120];
    //char *temp_values[32];
    //void ** temp_pointers;
    int insertion_index, split, i, j;
    int64_t new_key;

    new_leaf = make_leaf();
    num_keys = getNumKeys(leaf);

    temp_keys = malloc( leaf_order * sizeof(int64_t) );
    if (temp_keys == NULL) {
        perror("Temporary keys array.");
        exit(EXIT_FAILURE);
    }

    /*temp_pointers = malloc( order * sizeof(void *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array.");
        exit(EXIT_FAILURE);
    }*/

    insertion_index = 0;
    while (insertion_index < leaf_order - 1 && getKeyInLeaf(leaf, insertion_index) < key)
        insertion_index++;
    //if(getKeyInLeaf(leaf, leaf_order - 1) < key) insertion_index = leaf_order;
    for (i = 0, j = 0; i < getNumKeys(leaf); i++, j++) {
        if (j == insertion_index) j++;
        temp_keys[j] = getKeyInLeaf(leaf, i);
        //temp_pointers[j] = leaf->pointers[i];
    }

    temp_keys[insertion_index] = key;
    //temp_pointers[insertion_index] = pointer;

    //leaf->num_keys = 0;
    num_keys = 0;
    fseek(file_db, leaf + 12, SEEK_SET);
    fwrite(&num_keys, 4, 1, file_db);

    split = cut(leaf_order - 1);
    //splitting keys
    for (i = 0; i < split; i++) {
        //leaf->pointers[i] = temp_pointers[i];
        setKeyInLeaf(leaf, i, temp_keys[i]);
        setNumKeys(leaf);
    }

    for (i = split, j = 0; i < leaf_order; i++, j++) {
        //new_leaf->pointers[j] = temp_pointers[i];
        setKeyInLeaf(new_leaf, j, temp_keys[i]);
        setNumKeys(new_leaf);
    }
    //splitting values
    num_keys = getNumKeys(leaf);
    if(insertion_index < split) {
        for(i = split -1, j = 0; i < leaf_order; i++, j++) {
            getValue(leaf, i, temp_value);
            setValue(new_leaf, j, temp_value);
        }        
        for (i = getNumKeys(leaf); i > insertion_index; i--) {
        		getValue(leaf, i - 1, temp_value);
        		setValue(leaf, i, temp_value);
        }
        setValue(leaf, insertion_index, value);
    }
    else {
        for(i = split, j = 0; i < leaf_order; i++, j++) {
            if(j == insertion_index - split) j++;
            getValue(leaf, i, temp_value);
            setValue(new_leaf, j, temp_value);
        }
        setValue(new_leaf, insertion_index - split, value);
    }
    //free(temp_pointers);
    //free(temp_keys);
    int64_t temp_offset;

    temp_offset = getRightSibiling(leaf);
    setRightSibling(new_leaf, temp_offset);
    setRightSibling(leaf, new_leaf);
    //new_leaf->pointers[order - 1] = leaf->pointers[order - 1];
    //leaf->pointers[order - 1] = new_leaf;

    /*for (i = leaf->num_keys; i < order - 1; i++)
        leaf->pointers[i] = NULL;
    for (i = new_leaf->num_keys; i < order - 1; i++)
        new_leaf->pointers[i] = NULL;
    */
    temp_offset = getParentPage(leaf);
    setParentPage(new_leaf, temp_offset);
    new_key = getKeyInLeaf(new_leaf, 0);

    int64_t root_page_offset = getRootPage();

    return insert_into_parent(root_page_offset, leaf, new_key, new_leaf);
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */ 
//root, parent, left_index, key, right
//inset key + right at left_index + 1
int64_t insert_into_node(int64_t root, int64_t n, int left_index,
                            int64_t key, int64_t right) {
    int i;
    int32_t num_keys = getNumKeys(n);
    int64_t temp_offset;
    for (i = getNumKeys(n) - 1; i > left_index; i--) {
        temp_offset = getOffsetInNode(n, i);
        setOffsetInNode(n, i + 1, temp_offset);
        setKeyInNode(n, i + 1, getKeyInNode(n, i));
    }
    setOffsetInNode(n, left_index + 1, right);
    setKeyInNode(n, left_index + 1, key);
    setNumKeys(n);
    return root;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
int64_t insert_into_node_after_splitting(int64_t root, int64_t old_node, int left_index, 
                                    int64_t key, int64_t right) {

    int i, j, split;
    int64_t k_prime;
    int64_t new_node, child;
    int64_t * temp_keys;
    int64_t * temp_offsets;
    //node ** temp_pointers;
    int32_t num_keys = getNumKeys(old_node);

    printf("in the insert into node after splitting\n");

    /*for(int k = -1; k < getNumKeys(old_node); k++) {
    		printf("%ld | ", getOffsetInNode(old_node, k));
    }
    printf("\n");*/

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */

    temp_offsets = malloc( (node_order + 1) * sizeof(int64_t));
    if (temp_offsets == NULL) {
        perror("Temporary pointers array for splitting nodes.");
        exit(EXIT_FAILURE);
    }
    temp_keys = malloc( node_order * sizeof(int64_t) );
    if (temp_keys == NULL) {
        perror("Temporary keys array for splitting nodes.");
        exit(EXIT_FAILURE);
    }

    for (i = -1, j = 0; i < getNumKeys(old_node); i++, j++) {
        if (j == left_index + 1) j++;
        temp_offsets[j] = getOffsetInNode(old_node, i);
    }

    for (i = 0, j = 0; i < getNumKeys(old_node); i++, j++) {
        if (j == left_index + 1) j++;
        temp_keys[j] = getKeyInNode(old_node, i);
    }

    temp_offsets[left_index + 1] = right;
    temp_keys[left_index + 1] = key;
    //printf("%d is left_index", left_index);

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(node_order);
    new_node = make_node();
    num_keys = 0;
    fseek(file_db, old_node + 12, SEEK_SET);
    fwrite(&num_keys, 4, 1, file_db);
    
    setOffsetInNode(old_node, -1, temp_offsets[0]);
    for (i = 0; i < split; i++) {
        setOffsetInNode(old_node, i, temp_offsets[i + 1]);
        setKeyInNode(old_node, i, temp_keys[i]);
        setNumKeys(old_node);
    }
    k_prime = temp_keys[split];
    setOffsetInNode(new_node, -1, temp_offsets[split]);
    for (i = split + 1, j = 0; i <= leaf_order; i++, j++) {
        setOffsetInNode(new_node, j, temp_offsets[i + 1]);
        setKeyInNode(new_node, j, temp_keys[i]);
        setNumKeys(new_node);
    }

//printf("%ld this is the k-prime \n", k_prime);
    //free(temp_offsets);
    //free(temp_keys);
    setParentPage(new_node, getParentPage(old_node));
    /*for (i = 0; i <= new_node->num_keys; i++) {
        child = new_node->pointers[i];
        child->parent = new_node;
    }*/

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */
/*    printf("it is temp_keys\n");
    for(int k = 0; k < node_order; k++) {
    		printf("%ld | ", temp_keys[k]);
    }
    printf("\n");

    for(int k = -1; k < getNumKeys(old_node); k++) {
    		printf("%ld | ", getOffsetInNode(old_node, k));
    }
    printf("\n");
    for(int k = -1; k < getNumKeys(new_node); k++) {
    		printf("%ld | ", getOffsetInNode(new_node, k));
    }
    printf("\n");
*/
    return insert_into_parent(root, old_node, k_prime, new_node);
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
int64_t insert_into_parent(int64_t root, int64_t left, int64_t key, int64_t right) {

    int left_index;
    int64_t parent;
    int k;

    parent = getParentPage(left);
    //printf("parent %ld key %ld\n", parent, key);

    /* Case: new root. */

    if (parent == 0) {
    		for(int k = 0; k < node_order; )
        return insert_into_new_root(left, key, right);
    }

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */

    left_index = get_left_index(parent, left);


    /* Simple case: the new key fits into the node. 
     */

    if (getNumKeys(parent) < node_order - 1)
        return insert_into_node(root, parent, left_index, key, right);

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */
    printf("let's split node\n");
    return insert_into_node_after_splitting(root, parent, left_index, key, right);
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
int64_t insert_into_new_root(int64_t left, int64_t key, int64_t right) {

    int64_t root = make_node();
    //printf("new root : %ld\n", root);
    setRootPage(root);

    setKeyInNode(root, 0, key);
    setOffsetInNode(root, -1, left);
    setOffsetInNode(root, 0, right);
    setNumKeys(root);
    setParentPage(left, root);
    setParentPage(right, root);
    return root;
}



/* First insertion:
 * start a new tree.
 */
int64_t start_new_tree(int key, char* value) {

    int64_t root = make_leaf();
    setKeyInLeaf(root, 0, key);
    setValue(root, 0, value);
    //root->pointers[order - 1] = NULL;
    //root->parent = NULL;
    setNumKeys(root);
    setRootPage(root);
    return root;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
int insert( int64_t key, char* buf ) {
	strcpy(value, buf);

    //record * pointer;
    int64_t leaf;
    int64_t root = getRootPage();
    int32_t num_keys;

    /* The current implementation ignores
     * duplicates.
     */

    if (find(key) != NULL)
        return -1;

    /* Create a new record for the
     * value.
     */
    //pointer = make_record(value);


    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    if (root == 0) {
        start_new_tree(key, value);
        //fdatasync(fd);
        fflush(file_db);
        return 0;
    }


    /* Case: the tree already exists.
     * (Rest of function body.)
     */
    leaf = find_leaf(root, key); //return leaf page offset that have key

    /* Case: leaf has room for key and pointer.
     */
    num_keys = getNumKeys(leaf);
    //printf("num_keys : %d", getNumKeys(leaf));
    if (getNumKeys(leaf) < leaf_order - 1) {
        leaf = insert_into_leaf(leaf, key, value);
        //fdatasync(fd);
        fflush(file_db);
        return 0;
    }


    /* Case:  leaf must be split.
     */

    insert_into_leaf_after_splitting(root, leaf, key, value);
    //fdatasync(fd);
    fflush(file_db);
    return 0;
}




// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
int get_neighbor_index( int64_t n ) {

    int i;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
    for (i = -1; i < getNumKeys(getParentPage(n)); i++)
        if (getKeyInNode(getParentPage(n), i) == n)
            return i - 1;

    // Error state.
    //printf("Search for nonexistent pointer to node in parent.\n");
    //printf("Node:  %#ld\n", (unsigned long)n);
    //exit(EXIT_FAILURE);
}


int64_t remove_entry_from_node(int64_t n, int64_t key) {

    int i, num_values, num_offsets, offset;
    char temp_value[120];
    char value[120];
    if(getIsLeaf(n)) {
        i = 0;
        strcpy(value, find(key));
        //value = find(key); erase
        while (getKeyInLeaf(n, i) != key) {
            i++;
        }
        for (++i; i < getNumKeys(n); i++) {
            setKeyInLeaf(n, i - 1, getKeyInLeaf(n, i));
        }
        num_values = getNumKeys(n);
        i = 0;
        getValue(n , i, temp_value);
        while (strcmp(temp_value, value) != 0 ) {
            i++;
            getValue(n, i, temp_value);
        }
        for (++i; i < num_values; i++) {
            getValue(n, i, temp_value);
            setValue(n, i - 1, temp_value);
        }
    }
    else {
        i = 0;
        while(getKeyInNode(n, i) != key) {
            i++;
        }
        for(++i; i < getNumKeys(n); i++) {
            setKeyInNode(n, i - 1, getKeyInNode(n ,i)); 
        }
        num_offsets = getNumKeys(n) + 1;
        i = -1;
        offset = getOffsetInNode(n, key);
        while(getOffsetInNode(n, i) != offset) {
            i++;
        }
        for(++i; i < num_offsets; i++) {
            setOffsetInNode(n, i - 1, getOffsetInNode(n, i));
        }
    }

    // One key fewer.
    setNumKeysD(n);

    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    strcpy(temp_value, ""); //ljh kill
    if (getIsLeaf(n)) {
        for (i = getNumKeys(n); i < leaf_order; i++) {
            setValue(n, i, temp_value);
            //n->pointers[i] = NULL;
        }
    }
    else {
        for (i = getNumKeys(n); i < node_order; i++) {
            setOffsetInNode(n, i, 0);
            //n->pointers[i] = NULL;
        }
    }

    return n;
}


int64_t adjust_root(int64_t root) {

    int64_t new_root;

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (getNumKeys(root) > 0)
        return root;

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.

    if (getIsLeaf(root) == 0) {
        new_root = getOffsetInNode(root, -1);
        setRootPage(new_root);
        //new_root->parent = NULL;
        setParentPage(new_root, 0);
        setFreePage(root);
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else {
        setRootPage(0);
        setFreePage(root);
        new_root = 0;
    }


    return new_root;
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
int64_t coalesce_nodes( int64_t n, int64_t neighbor, int neighbor_index, int64_t k_prime) {

    int i, j, neighbor_insertion_index, n_end;
    int64_t tmp;
    char temp_value[120];

    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     */

    if (neighbor_index == -2) {  //swap
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    neighbor_insertion_index = getNumKeys(neighbor);

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */

    if (getIsLeaf(n) != 1) { //when node

        /* Append k_prime.
         */
        setKeyInNode(neighbor, neighbor_insertion_index, k_prime);
        setNumKeys(neighbor);


        n_end = getNumKeys(n);

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            setKeyInNode(neighbor, i, getKeyInNode(n, j));
            setOffsetInNode(neighbor, i, getOffsetInNode(n, j));
            setNumKeys(neighbor);
            setNumKeysD(n);
        }

        /* The number of pointers is always
         * one more than the number of keys.
         */

        //neighbor->pointers[i] = n->pointers[j];

        /* All children must now point up to the same parent.
         */

        for (i = -1; i < getNumKeys(neighbor); i++) {
            tmp = getOffsetInNode(neighbor, i);
            setParentPage(tmp, neighbor);
            //tmp->parent = neighbor;
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     */

    else {
        for (i = neighbor_insertion_index, j = 0; j < getNumKeys(n); i++, j++) {
            setKeyInLeaf(neighbor, i, getKeyInLeaf(n, j));
            getValue(neighbor, j, temp_value);
            setValue(neighbor, i, temp_value);
            setNumKeys(neighbor);
        }
        setRightSibling(neighbor, getRightSibiling(n));
    }
    int64_t root = getRootPage();
    root = delete_entry(getParentPage(n), k_prime);
    //free(n->keys);
    //free(n->pointers);
    //free(n); 
    return root;
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
int64_t redistribute_nodes(int64_t n, int64_t neighbor, int neighbor_index, 
        int k_prime_index, int64_t k_prime) {  

    int i;
    int64_t tmp;
    char temp_value[120];
    int64_t root = getRootPage();
    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */

    if (neighbor_index != -2) {
        if (getIsLeaf(n) != 1) {
            for(i = getNumKeys(n); i > -1; i--) {
                setKeyInNode(n, i, getKeyInNode(n, i - 1));
                setOffsetInNode(n, i, getOffsetInNode(n, i - 1));
            }
        }
        for (i = getNumKeys(n); i > 0 && getIsLeaf(n); i--) {
            setKeyInLeaf(n, i, getKeyInLeaf(n, i - 1));
            getValue(n, i - 1, temp_value);
            setValue(n, i, temp_value);
        }
        if (getIsLeaf(n) != 1) {
            setOffsetInNode(n, -1, getOffsetInNode(neighbor, getNumKeys(neighbor) - 1));
            tmp = getOffsetInNode(n, -1);
            setParentPage(tmp, n);
            setOffsetInNode(neighbor, getNumKeys(neighbor) -1, 0);
            setKeyInNode(n, 0, k_prime);
            setKeyInNode(getParentPage(n), k_prime_index, getKeyInNode(neighbor, getNumKeys(neighbor) -1));
    
        }
        else {
            getValue(neighbor, getNumKeys(neighbor) - 1, temp_value);
            setValue(n, 0, temp_value);
            strcpy(temp_value, "");
            setValue(neighbor, getNumKeys(neighbor) - 1, temp_value);
            setKeyInLeaf(n, 0, getKeyInLeaf(neighbor, getNumKeys(neighbor) - 1));
            setKeyInNode(getParentPage(n), k_prime_index, getKeyInLeaf(n, 0));
            //n->parent->keys[k_prime_index] = n->keys[0];
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */

    else {  
        if (getIsLeaf(n)) {
            setKeyInLeaf(n, getNumKeys(n), getKeyInLeaf(neighbor, 0));
            getValue(neighbor, 0, temp_value);
            setValue(n, getNumKeys(n), temp_value);
            setKeyInNode(getParentPage(n), k_prime_index, getKeyInLeaf(neighbor, 1));
        }
        else {
            setKeyInNode(n, getNumKeys(n), k_prime);
            setOffsetInNode(n, getNumKeys(n), getOffsetInNode(neighbor, -1));
            tmp = getOffsetInNode(n, getNumKeys(n));
            setParentPage(tmp, n);
            setKeyInNode(getParentPage(n), k_prime_index, getKeyInNode(neighbor, 0));
        }
        for (i = 0; i < getNumKeys(neighbor) - 1 && getIsLeaf(n); i++) {
            setKeyInLeaf(neighbor, i, getKeyInLeaf(neighbor, i + 1));
            getValue(neighbor, i + 1, temp_value);
            setValue(neighbor, i, temp_value);
        }
        if (getIsLeaf(n) != 1) {
            for(i = 0; i < getNumKeys(neighbor) - 1; i++) {
                setKeyInNode(neighbor, i, getKeyInNode(neighbor, i + 1));
                setOffsetInNode(neighbor, i, getOffsetInNode(neighbor, i + 1));
            }
            setOffsetInNode(neighbor, -1, getOffsetInNode(neighbor, 0));
        }
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */
    setNumKeys(n);
    setNumKeysD(neighbor);

    return root;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
int64_t delete_entry( int64_t n, int64_t key) {

    int64_t min_keys;
    int64_t neighbor;
    int neighbor_index;
    int k_prime_index, k_prime;
    int capacity;
    int64_t root =getRootPage();

    // Remove key and pointer from node.

    n = remove_entry_from_node(n, key);

    /* Case:  deletion from the root. 
     */

    if (n == root) 
        return adjust_root(root);


    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */

    min_keys = getIsLeaf(n) ? cut(leaf_order - 1) : cut(node_order) - 1;

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    if (getNumKeys(n) >= min_keys)
        return root;

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor node with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */

    neighbor_index = get_neighbor_index( n );
    k_prime_index = neighbor_index == -2 ? 0 : neighbor_index;
    k_prime = getKeyInNode(getParentPage(n), k_prime_index);
    neighbor = neighbor_index == -2 ? getOffsetInNode(getParentPage(n), 0) : 
        getOffsetInNode(getParentPage(n), neighbor_index);

    capacity = getIsLeaf(n) ? leaf_order : node_order - 1;

    /* Coalescence. */

    if (getNumKeys(neighbor) + getNumKeys(n) < capacity)
        return coalesce_nodes(n, neighbor, neighbor_index, k_prime);

    /* Redistribution. */

    else
        return redistribute_nodes(n, neighbor, neighbor_index, k_prime_index, k_prime);
}



/* Master deletion function.
 */
 int64_t delete( int64_t key) {

    int64_t key_leaf;
    char key_value[120];
    int64_t root = getRootPage();

    if(find(key) == NULL) return 0;

    strcpy(find(key), key_value);
    //key_value = find(key); erase
    key_leaf = find_leaf(root, key);
    if (key_value != NULL && key_leaf != 0) {
        root = delete_entry(key_leaf, key);
    }
    //fdatasync(fd);
    fflush(file_db);
    return root;
}


/*void destroy_tree_nodes(node * root) {
    int i;
    if (root->is_leaf)
        for (i = 0; i < root->num_keys; i++)
            free(root->pointers[i]);
    else
        for (i = 0; i < root->num_keys + 1; i++)
            destroy_tree_nodes(root->pointers[i]);
    free(root->pointers);
    free(root->keys);
    free(root);
}*/


/*int64_t destroy_tree() {
    //fclose(file_db);
    truncate(pathname, 4096);
    //file_db = fopen(pathname, "w+");
    int64_t temp = 0;
    int32_t temp1 = 1;
    fseek(file_db, 0, SEEK_SET);
    fwrite(&temp, 8, 1, file_db);
    fwrite(&temp, 8, 1, file_db);
    fwrite(&temp1, 4, 1, file_db);
    return NULL;
}*/

