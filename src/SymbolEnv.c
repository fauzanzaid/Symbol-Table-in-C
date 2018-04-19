#include <stdlib.h>
#include <string.h>

#include "HashTable.h"
#include "LinkedList.h"

#include "SymbolEnv.h"


///////////////
// Constants //
///////////////

static const int HASHTABLE_SIZE = 31;


/////////////////////
// Data Structures //
/////////////////////

typedef struct SymbolEnv {
	SymbolEnv_Scope *scp_root_ptr;
	SymbolEnv_Scope *scp_cur_ptr;
	SymbolEnv_Scope *scp_last_child_ptr;

	int memory_allocated;
}SymbolEnv;

typedef struct SymbolEnv_Scope {
	SymbolEnv *env_ptr;

	SymbolEnv_Scope *parent;
	SymbolEnv_Scope *sibling;
	SymbolEnv_Scope *child;

	int nesting;

	char *name;
	int len_name;

	HashTable *tbl_ptr;
	// id_lst maintains the keys of the hashtable
	LinkedList *id_lst_ptr;
}SymbolEnv_Scope;

typedef struct SymbolEnv_Entry {
	SymbolEnv_Scope *scp_ptr;

	char *id;
	int len_id;
	int size;
	void *type_ptr;
	int offset;

	int flag_initialized;
}SymbolEnv_Entry;


////////////////////
// Pvt Prototypes //
////////////////////

static SymbolEnv_Scope* SymbolEnv_Scope_new(SymbolEnv *env_ptr, char *name, int len_name);

static void SymbolEnv_Scope_destroy(SymbolEnv_Scope *scp_ptr);

static SymbolEnv_Entry *SymbolEnv_Entry_new(SymbolEnv_Scope *scp_ptr, char *id, int size, void *type_ptr);

static void SymbolEnv_Entry_destroy(SymbolEnv_Entry *etr_ptr);

static int hash_function(void *key);

static int key_compare(void *key1, void *key2);


//////////////////////////////////
// Constructors and Destructors //
//////////////////////////////////

SymbolEnv *SymbolEnv_new(char *name, int len_name){
	SymbolEnv *env_ptr = malloc( sizeof(SymbolEnv) );

	env_ptr->scp_root_ptr = SymbolEnv_Scope_new(env_ptr, name, len_name);
	env_ptr->scp_cur_ptr = env_ptr->scp_root_ptr;
	env_ptr->scp_last_child_ptr = NULL;

	env_ptr->memory_allocated = 0;

	return env_ptr;
}

void SymbolEnv_destroy(SymbolEnv *env_ptr){
	LinkedList *stack =  LinkedList_new();

	LinkedList_push(stack, env_ptr->scp_root_ptr);

	while( LinkedList_peek(stack) != NULL ){
		SymbolEnv_Scope *current = LinkedList_pop(stack);

		// Push all children on to stack
		SymbolEnv_Scope *child = current->child;
		while(child){
			LinkedList_push(stack, child);
			child = child->sibling;
		}

		SymbolEnv_Scope_destroy(current);
	}
	LinkedList_destroy(stack);

	free(env_ptr);
}

static SymbolEnv_Scope* SymbolEnv_Scope_new(SymbolEnv *env_ptr, char *name, int len_name){
	SymbolEnv_Scope *scp_ptr = malloc( sizeof(SymbolEnv_Scope) );

	scp_ptr->env_ptr = env_ptr;
	scp_ptr->parent = NULL;
	scp_ptr->child = NULL;
	scp_ptr->sibling = NULL;

	scp_ptr->nesting = 0;

	scp_ptr->name = malloc( sizeof(char) * (len_name+1) );
	strncpy(scp_ptr->name, name, len_name);
	scp_ptr->name[len_name] = '\0';
	scp_ptr->len_name = len_name+1;

	scp_ptr->tbl_ptr = HashTable_new(HASHTABLE_SIZE, hash_function, key_compare);
	scp_ptr->id_lst_ptr = LinkedList_new();
}

static void SymbolEnv_Scope_destroy(SymbolEnv_Scope *scp_ptr){
	if(scp_ptr->name != NULL)
		free(scp_ptr->name);

	LinkedListIterator *itr_ptr = LinkedListIterator_new(scp_ptr->id_lst_ptr);
	LinkedListIterator_move_to_first(itr_ptr);

	char *id = LinkedListIterator_get_item(itr_ptr);
	while(id != NULL){
		SymbolEnv_Entry *etr_ptr = HashTable_get(scp_ptr->tbl_ptr, id);
		SymbolEnv_Entry_destroy(etr_ptr);

		LinkedListIterator_move_to_next(itr_ptr);
		id = LinkedListIterator_get_item(itr_ptr);
	}
	LinkedListIterator_destroy(itr_ptr);

	// Can free id now, as hashtable will no longer be used
	while( LinkedList_peek(scp_ptr->id_lst_ptr) ){
		free( LinkedList_pop(scp_ptr->id_lst_ptr) );
	}

	HashTable_destroy(scp_ptr->tbl_ptr);
	LinkedList_destroy(scp_ptr->id_lst_ptr);

	free(scp_ptr);
}

static SymbolEnv_Entry *SymbolEnv_Entry_new(SymbolEnv_Scope *scp_ptr, char *id, int len_id, void *type_ptr){
	SymbolEnv_Entry *etr_ptr = malloc( sizeof(SymbolEnv_Entry) );

	etr_ptr->scp_ptr = scp_ptr;

	etr_ptr->id = malloc( sizeof(char) * (len_id+1) );
	strncpy(etr_ptr->id, id, len_id);
	etr_ptr->id[len_id] = '\0';
	etr_ptr->len_id = len_id;

	etr_ptr->type_ptr = type_ptr;
	etr_ptr->size = 0;
	etr_ptr->offset = 0;

	etr_ptr->flag_initialized = 0;

	return etr_ptr;
}

static void SymbolEnv_Entry_destroy(SymbolEnv_Entry *etr_ptr){
	if(etr_ptr->type_ptr != NULL)
		SymbolEnv_Type_destroy(etr_ptr->type_ptr);

	free(etr_ptr);
	// id will be freed later, as it will be used by hashtable get
}


///////////
// Scope //
///////////

SymbolEnv_Scope *SymbolEnv_scope_add(SymbolEnv *env_ptr, char *name, int len_name){

	SymbolEnv_Scope *scp_ptr = SymbolEnv_Scope_new(env_ptr, name, len_name);
	// printf("SymbolEnv_scope_add : %s\n", scp_ptr->name);

	scp_ptr->parent = env_ptr->scp_cur_ptr;
	// Add scope to tree
	if(env_ptr->scp_last_child_ptr == NULL){
		// Add scope as left most child
		scp_ptr->sibling = scp_ptr->parent->child;
		scp_ptr->parent->child = scp_ptr;
	}
	else{
		// Add scope to the right of last child
		scp_ptr->sibling = env_ptr->scp_last_child_ptr->sibling;
		env_ptr->scp_last_child_ptr->sibling = scp_ptr;
	}

	scp_ptr->nesting = env_ptr->scp_cur_ptr->nesting + 1;

	// Enter the scope
	env_ptr->scp_cur_ptr = scp_ptr;
	env_ptr->scp_last_child_ptr = NULL;

	return scp_ptr;
}

SymbolEnv_Scope *SymbolEnv_scope_enter(SymbolEnv *env_ptr){
	if(env_ptr->scp_last_child_ptr == NULL){
		if(env_ptr->scp_cur_ptr->child == NULL){
			return NULL;
		}

		else{
			env_ptr->scp_cur_ptr = env_ptr->scp_cur_ptr->child;
			env_ptr->scp_last_child_ptr = NULL;

			return env_ptr->scp_cur_ptr;
		}
	}

	else{
		if(env_ptr->scp_last_child_ptr->sibling == NULL){
			return NULL;
		}

		else{
			env_ptr->scp_cur_ptr = env_ptr->scp_last_child_ptr->sibling;
			env_ptr->scp_last_child_ptr = NULL;

			return env_ptr->scp_cur_ptr;
		}
	}
}

SymbolEnv_Scope *SymbolEnv_scope_exit(SymbolEnv *env_ptr){
	if(env_ptr->scp_cur_ptr->parent == NULL)
		return NULL;

	env_ptr->scp_last_child_ptr = env_ptr->scp_cur_ptr;
	env_ptr->scp_cur_ptr = env_ptr->scp_cur_ptr->parent;

	return env_ptr->scp_cur_ptr;
}

SymbolEnv_Scope *SymbolEnv_scope_reset(SymbolEnv *env_ptr){
	env_ptr->scp_cur_ptr = env_ptr->scp_root_ptr;
	env_ptr->scp_last_child_ptr = NULL;

	return env_ptr->scp_cur_ptr;
}

SymbolEnv_Scope *SymbolEnv_scope_get_current(SymbolEnv *env_ptr){
	return env_ptr->scp_cur_ptr;
}

SymbolEnv_Scope *SymbolEnv_scope_get_root(SymbolEnv *env_ptr){
	return env_ptr->scp_root_ptr;
}

SymbolEnv_Scope *SymbolEnv_scope_set_explicit(SymbolEnv *env_ptr, SymbolEnv_Scope *scp_ptr){
	if(scp_ptr->env_ptr != env_ptr)
		return NULL;

	env_ptr->scp_cur_ptr = scp_ptr;
	env_ptr->scp_last_child_ptr = NULL;
}

char* SymbolEnv_Scope_get_name(SymbolEnv_Scope *scp_ptr){
	return scp_ptr->name;
}

SymbolEnv_Entry *SymbolEnv_Scope_entry_get_by_id(SymbolEnv_Scope *scp_ptr, char *id, int len_id){
	// Add null terminator
	char id_2[len_id+1];
	strncpy(id_2, id, len_id);
	id_2[len_id] = '\0';

	return HashTable_get(scp_ptr->tbl_ptr, id_2);
}

LinkedList *SymbolEnv_Scope_get_id_lst(SymbolEnv_Scope *scp_ptr){
	return scp_ptr->id_lst_ptr;
}

int SymbolEnv_Scope_get_nesting_level(SymbolEnv_Scope *scp_ptr){
	return scp_ptr->nesting;
}

SymbolEnv_Scope *SymbolEnv_Scope_get_inorder(SymbolEnv_Scope *scp_ptr){
	if(scp_ptr->child != NULL){
		// Move to child if it exists
		return scp_ptr->child;
	}
	else if(scp_ptr->sibling != NULL){
		// Else, move to sibling if it exists
		return scp_ptr->sibling;
	}
	else{
		// Else, move to the sibling of the first ancestor who has one
		SymbolEnv_Scope *current = scp_ptr;

		while(1){
			if(current->parent == NULL){
				// No such ancestor exists, given node was the rightmost
				return NULL;
			}
			else if(current->parent->sibling == NULL){
				// Immediate parent does not have a sibling, move to next
				// parent
				current = current->parent;
			}
			else{
				// Immediate parent has a sibling
				return current->parent->sibling;
			}
		}
	}
}

SymbolEnv_Scope *SymbolEnv_Scope_get_parent(SymbolEnv_Scope *scp_ptr){
	return scp_ptr->parent;
}


/////////////
// Entries //
/////////////

SymbolEnv_Entry *SymbolEnv_entry_add(SymbolEnv *env_ptr, char *id, int len_id, void *type_ptr){
	SymbolEnv_Scope *scp_ptr = env_ptr->scp_cur_ptr;
	SymbolEnv_Entry *etr_ptr = SymbolEnv_Entry_new(scp_ptr, id, len_id, type_ptr);

	if( HashTable_get(scp_ptr->tbl_ptr, etr_ptr->id) != NULL){
		// Symbol already exists in current scope

		// Need to free id in etr as it will not be added to linked list
		free(etr_ptr->id);
		SymbolEnv_Entry_destroy(etr_ptr);
		return NULL;
	}

	LinkedList_pushback(scp_ptr->id_lst_ptr, etr_ptr->id);
	HashTable_add(scp_ptr->tbl_ptr, etr_ptr->id, etr_ptr);

	return etr_ptr;
}

SymbolEnv_Entry *SymbolEnv_entry_get_by_id(SymbolEnv *env_ptr, char *id, int len_id){
	SymbolEnv_Scope *scp_ptr = env_ptr->scp_cur_ptr;

	// Add null terminator
	char id_2[len_id+1];
	strncpy(id_2, id, len_id);
	id_2[len_id] = '\0';

	while(scp_ptr != NULL){
		SymbolEnv_Entry *etr_ptr = HashTable_get(scp_ptr->tbl_ptr, id_2);

		if(etr_ptr != NULL)
			return etr_ptr;

		scp_ptr = scp_ptr->parent;
	}

	return NULL;
}

int SymbolEnv_entry_set_flag_initialized_by_id(SymbolEnv *env_ptr, char *id, int len_id){
	SymbolEnv_Entry *etr_ptr = SymbolEnv_entry_get_by_id(env_ptr, id, len_id);

	if(etr_ptr == NULL)
		return -1;

	etr_ptr->flag_initialized = 1;

	return 0;
}

int SymbolEnv_entry_get_flag_initialized_by_id(SymbolEnv *env_ptr, char *id, int len_id){
	SymbolEnv_Entry *etr_ptr = SymbolEnv_entry_get_by_id(env_ptr, id, len_id);

	if(etr_ptr == NULL)
		return -1;

	return etr_ptr->flag_initialized;
}

char *SymbolEnv_Entry_get_id(SymbolEnv_Entry *etr_ptr){
	return etr_ptr->id;
}

int SymbolEnv_Entry_get_size(SymbolEnv_Entry *etr_ptr){
	return etr_ptr->size;
}

void SymbolEnv_Entry_set_size(SymbolEnv_Entry *etr_ptr, int size){
	etr_ptr->size = size;
}

void *SymbolEnv_Entry_get_type(SymbolEnv_Entry *etr_ptr){
	return etr_ptr->type_ptr;
}

int SymbolEnv_Entry_get_offset(SymbolEnv_Entry *etr_ptr){
	return etr_ptr->offset;
}

SymbolEnv_Scope *SymbolEnv_Entry_get_scope(SymbolEnv_Entry *etr_ptr){
	return etr_ptr->scp_ptr;
}

void SymbolEnv_Entry_set_flag_initialized(SymbolEnv_Entry *etr_ptr){
	etr_ptr->flag_initialized = 1;
}

int SymbolEnv_Entry_get_flag_initialized(SymbolEnv_Entry *etr_ptr){
	return etr_ptr->flag_initialized;
}



/////////////
// Hashing //
/////////////

static int hash_function(void *key){
	char *string = (char *)key;
	int hash = 5381;
	int c;
	while(c = *string++)
		hash = ((hash << 5) + hash) + c;

	if(hash<0)
		return -hash;
	else
		return hash;
}

static int key_compare(void *key1, void *key2){
	return strcmp((char *)key1, (char *)key2);
}


////////////
// Layout //
////////////

void SymbolEnv_layout_memory(SymbolEnv *env_ptr, int(*get_type_width)(void *) ){
	SymbolEnv_Scope *scp_ptr = env_ptr->scp_root_ptr;
	env_ptr->memory_allocated = 0;

	while(scp_ptr){
		LinkedList *id_lst_ptr = SymbolEnv_Scope_get_id_lst(scp_ptr);

		LinkedListIterator *itr_ptr = LinkedListIterator_new(id_lst_ptr);
		LinkedListIterator_move_to_first(itr_ptr);
		char *id = LinkedListIterator_get_item(itr_ptr);

		while(id){
			SymbolEnv_Entry *etr_ptr = SymbolEnv_Scope_entry_get_by_id(scp_ptr, id, strlen(id));
			void *type_ptr = SymbolEnv_Entry_get_type(etr_ptr);

			int size = get_type_width(type_ptr);
			if(size > 0){
				etr_ptr->offset = env_ptr->memory_allocated;
				etr_ptr->size = size;
				env_ptr->memory_allocated += size;
			}

			LinkedListIterator_move_to_next(itr_ptr);
			id = LinkedListIterator_get_item(itr_ptr);
		}

		LinkedListIterator_destroy(itr_ptr);
		scp_ptr = SymbolEnv_Scope_get_inorder(scp_ptr);
	}
}
