
#include "globals.h"

#define MOVE_TO_INT(mv, player)   \
   ((mv).mask_index-1)*g_board_size[(player)]+((mv).array_index-1)

#define TABLE_SET_KEY(table,index,k)                          \
   table[index].key[0] = k[0], table[index].key[1] = k[1],    \
   table[index].key[2] = k[2], table[index].key[3] = k[3]; 

#define TABLE_CMP_KEY(table,index,k)                              \
   (table[index].key[0] == k[0] && table[index].key[1] == k[1] && \
    table[index].key[2] == k[2] && table[index].key[3] == k[3]) 


static inline s32bit
store_entry(Hash_Key x, s32bit value,
            s32bit alpha, s32bit beta,
            u32bit nodes, s32bit depth, Move best, s32bit player)
{
  s32bit index = (x).code;
  
  if(TABLE_CMP_KEY(g_trans_table, index, (x).key)){
    TABLE_SET_KEY(g_trans_table, index, (x).key);
    g_trans_table[index].nodes     =nodes;       
    g_trans_table[index].best_move =MOVE_TO_INT(best,player);
    g_trans_table[index].depth     =depth;       
    g_trans_table[index].whos_turn =player;      
    g_trans_table[index].value     =value;       
    if     (value>=beta) g_trans_table[index].type =LOWER;
    else if(value>alpha) g_trans_table[index].type =EXACT;
    else                 g_trans_table[index].type =UPPER;
    return 1;
  }
  return 0;
}

static inline void
store_entry_force(Hash_Key x, s32bit value,
                  s32bit alpha, s32bit beta,
                  u32bit nodes, s32bit depth, Move best, s32bit player)
{
  s32bit index = (x).code;
  
  TABLE_SET_KEY(g_trans_table, index, (x).key);
  g_trans_table[index].nodes     =nodes;       
  g_trans_table[index].best_move =MOVE_TO_INT(best,player);
  g_trans_table[index].depth     =depth;       
  g_trans_table[index].whos_turn =player;      
  g_trans_table[index].value     =value;       
  if     (value>=beta) g_trans_table[index].type =LOWER;
  else if(value>alpha) g_trans_table[index].type =EXACT;
  else                 g_trans_table[index].type =UPPER;
}


extern void
hashstore(s32bit value, s32bit alpha, s32bit beta,
          u32bit nodes, s32bit depth, Move best, s32bit player)
{
  u32bit a, b, c, d;

  if(store_entry(g_norm_hashkey, value, alpha, beta,
                 nodes, depth, best, player))   return;
  if(store_entry(g_flipV_hashkey, value, alpha, beta,
                 nodes, depth, best, player))   return;
  if(store_entry(g_flipH_hashkey, value, alpha, beta,
                 nodes, depth, best, player))   return;
  if(store_entry(g_flipVH_hashkey, value, alpha, beta,
                 nodes, depth, best, player))   return;

  a = g_trans_table[g_norm_hashkey.code].nodes;
  b = g_trans_table[g_flipV_hashkey.code].nodes;
  c = g_trans_table[g_flipH_hashkey.code].nodes;
  d = g_trans_table[g_flipVH_hashkey.code].nodes;
  
  if(a <= b && a <= c && a <= d){
    if(nodes >= a) store_entry_force(g_norm_hashkey, value, alpha, beta,
                                     nodes, depth, best, player);
  } else if(b <= c && b <= d){
    if(nodes >= b) store_entry_force(g_flipV_hashkey, value, alpha, beta,
                                     nodes, depth, best, player);
  } else if(c <= d){
    if(nodes >= c) store_entry_force(g_flipH_hashkey, value, alpha, beta,
                                     nodes, depth, best, player);
  } else {
    if(nodes >= d) store_entry_force(g_flipVH_hashkey, value, alpha, beta,
                                     nodes, depth, best, player);
  }
}


#define INT_TO_MOVE(mv, int, player)                    \
   (mv).mask_index = ((int)/g_board_size[(player)])+1;  \
   (mv).array_index = ((int)%g_board_size[(player)])+1


static inline s32bit
lookup_entry(Hash_Key x, s32bit *value,
           s32bit *alpha, s32bit *beta,
           s32bit depth_remaining, Move *force_first, s32bit player)
{
  s32bit index = (x).code;
  if(TABLE_CMP_KEY(g_trans_table, index, (x).key)
     && g_trans_table[index].whos_turn == player ) {
    /* found matching entry.*/                      
    
    /* If nothing else we can use this entry to give us a good move. */
    INT_TO_MOVE(*force_first, g_trans_table[index].best_move, player); 
    
    /* use value if depth >= than the depth remaining in our search. */
    if(g_trans_table[index].depth >= depth_remaining) {
      
      /* if the value is exact we can use it. */    
      if(g_trans_table[index].type == EXACT) {      
        *value=g_trans_table[index].value;          
        return 1;                                   
      }                                         
      
      /* if value is a lower bound we can possibly use it. */
      if(g_trans_table[index].type == LOWER) {  
        if(g_trans_table[index].value>=(*beta)){
          *value=g_trans_table[index].value;    
          return 1;
        }
        if(g_trans_table[index].value>(*alpha)){
          *alpha=g_trans_table[index].value;    
        }
        return 0;                                            
      }  
         
      /* if value is a upper bound we can possibly use it. */
      if(g_trans_table[index].type == UPPER) {               
        if(g_trans_table[index].value<=(*alpha)){            
          *value=g_trans_table[index].value;                 
          return 1;                                          
        }
        if(g_trans_table[index].value<(*beta)){              
          *beta=g_trans_table[index].value;                  
        }
        return 0;
      }
    }
  }
  return -1;
}


extern s32bit
hashlookup(s32bit *value, s32bit *alpha, s32bit *beta,
           s32bit depth_remaining, Move *force_first, s32bit player)
{
  s32bit r_value;
  
  r_value = lookup_entry(g_norm_hashkey, value, alpha, beta,
                         depth_remaining, force_first, player);
  if(r_value >= 0) return r_value;

  r_value = lookup_entry(g_flipV_hashkey, value, alpha, beta,
                         depth_remaining, force_first, player);
  if(r_value >= 0) return r_value;

  r_value = lookup_entry(g_flipH_hashkey, value, alpha, beta,
                         depth_remaining, force_first, player);
  if(r_value >= 0) return r_value;

  r_value = lookup_entry(g_flipVH_hashkey, value, alpha, beta,
                         depth_remaining, force_first, player);
  if(r_value >= 0) return r_value;

  return 0;
}
