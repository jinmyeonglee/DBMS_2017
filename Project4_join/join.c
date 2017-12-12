#include "bptree.h"

int join_table_low(table* t_1, table* t_2, table* t_join) {

  npage *np_1 = get_root(t_1);
  npage *np_2 = get_root(t_2);
  npage *temp;
  jpage *join_result;

  //find the leftmost leaf page => np_1, np_2
  while(B(np_1)-> is_leaf != 1) {
    temp = get_child(t_1, np_1, 0);
    release_page(t_1, np_1);
    np_1 = temp;
    release_page(t_1, temp);
  }
  while(B(np_2)-> is_leaf != 1) {
    temp = get_child(t_2, np_2, 0);
    release_page(t_2, np_2);
    np_2 = temp;
    release_page(t_2, temp);
  }
  //now np_1, np_2 is the leftmost leaf
  int iter_1, iter_2;
  addr right_1, right_2; // these are the right siblings offset.
  right_1 = B(np_1)-> l_sib;
  right_2 = B(np_2)-> l_sib;
  char value_1[120], value_2[120];
  int count;
  count = 0;
  iter_1 = iter_2 = 0;
  join_result = (jpage*)get_npage(t_join, 0); //just use 1 buffer, don't return
  set_dirty_f(join_result);//dirty_bit = 0 it can make mg don't write last page
  while(1) {
      while(B(np_1)-> l_recs[iter_1].k < B(np_2)-> l_recs[iter_2].k) {

        ++iter_1;
        if (iter_1 == B(np_1)-> num_keys) {
          right_1 = B(np_1)-> l_sib;
          if(right_1 == 0) {
            break;
          }
          release_page(t_1, np_1);
          np_1 = get_npage(t_1, right_1);
          iter_1 = 0;
        }
      }
      while(B(np_1)-> l_recs[iter_1].k > B(np_2)-> l_recs[iter_2].k) {

        ++iter_2;
        if(iter_2 == B(np_2)-> num_keys) {
          right_2 = B(np_2)-> l_sib;
          if(right_2 == 0) {
           break;
         }
         release_page(t_2, np_2);
         np_2 = get_npage(t_2, right_2);
         iter_2 = 0;
        }
      }
      //when key_1 == key_2
      while(B(np_1)-> l_recs[iter_1].k == B(np_2)-> l_recs[iter_2].k) {
        memcpy(value_1, B(np_1)-> l_recs[iter_1].v, 120);
        memcpy(value_2, B(np_2)-> l_recs[iter_2].v, 120);

        //push to the buffer until count == 14, and then write to the file
        if(count < 15) {
          write_result(t_join, join_result, count, B(np_1)-> l_recs[iter_1].k
            ,value_1, value_2);
          count++;
        }
        else { //count == 15
          write_result(t_join, join_result, count, B(np_1)-> l_recs[iter_1].k
            ,value_1, value_2);
          count = 0;
        }

        iter_2++; iter_1++;
        if (iter_1 == B(np_1)-> num_keys) {
          right_1 = B(np_1)-> l_sib;
          if(right_1 == 0) {
            break;
          }
          release_page(t_1, np_1);
          np_1 = get_npage(t_1, right_1);
          iter_1 = 0;
        }
        if(iter_2 == B(np_2)-> num_keys) {
          right_2 = B(np_2)-> l_sib;
          if(right_2 == 0) {
            break;
          }
          release_page(t_2, np_2);
          np_2 = get_npage(t_2, right_2);
          iter_2 = 0;
        }

      }

      right_1 = B(np_1)-> l_sib;
      right_2 = B(np_2)-> l_sib;
      //printf("r_1 : %ld, r_2 : %ld\n", right_1, right_2);
      if(right_1 == 0 || right_2 == 0) { //one table reaches the end of leaf
        for(int i = 0; i < count; i++) { // write the rest join results
          printf("%d  ldfs\n", i);
          dprintf(t_join-> bm.fd, "%ld,%s,%ld,%s\n",
          B(join_result)-> join_sets[i].key1, B(join_result)-> join_sets[i].v1,
          B(join_result)-> join_sets[i].key2, B(join_result)-> join_sets[i].v2);
          printf("%ld,%s,%ld,%s\n",
          B(join_result)-> join_sets[i].key1, B(join_result)-> join_sets[i].v1,
          B(join_result)-> join_sets[i].key2, B(join_result)-> join_sets[i].v2);
        }
        break; //escape the infinite loop: while(1)
      }
    }

  release_page(t_join, join_result);
  close_file(t_join);
  return 0;
}

void write_result(table * t, jpage* join_result, int count, int64_t key,
  char* value_1_, char* value_2_) {
    B(join_result)-> join_sets[count].key1 = key;
    memcpy(B(join_result)-> join_sets[count].v1, value_1_, 120);
    B(join_result)-> join_sets[count].key2 = key;
    memcpy(B(join_result)-> join_sets[count].v2, value_2_, 120);

    if(count == 15) {
      for(int i = 0; i < 16; i++) {
        dprintf(t-> bm.fd, "%ld,%s,%ld,%s\n",
        B(join_result)-> join_sets[i].key1, B(join_result)-> join_sets[i].v1,
        B(join_result)-> join_sets[i].key2, B(join_result)-> join_sets[i].v2);
        // printf("%ld,%s,%ld,%s\n",
        // B(join_result)-> join_sets[i].key1, B(join_result)-> join_sets[i].v1,
        // B(join_result)-> join_sets[i].key2, B(join_result)-> join_sets[i].v2);
      }
      //release_page(t, join_result); <- do not release
    }
}
