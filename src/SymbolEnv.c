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
}SymbolEnv;

typedef struct SymbolEnv_Scope {
	SymbolEnv *env_ptr;

	SymbolEnv_Scope *parent;
	SymbolEnv_Scope *sibling;
	SymbolEnv_Scope *child;

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
}SymbolEnv_Entry;


////////////////////
// Pvt Prototypes //
////////////////////

static SymbolEnv_Scope* SymbolEnv_Scope_new(SymbolEnv *env_ptr, char *name, int len_name);

static void SymbolEnv_Scope_destroy(SymbolEnv_Scope *scp_ptr);

static SymbolEnv_Entry *SymbolEnv_Entry_new(SymbolEnv_Scope *scp_ptr, char *id, int len_id, int size, void *type_ptr);

static void SymbolEnv_Entry_destroy(SymbolEnv_Entry *etr_ptr);

static int hash_function(void *key);

static int key_compare(void *key1, void *key2);


//////////////////////////////////
// Constructors and Destructors //
//////////////////////////////////

SymbolEnv *SymbolEnv_new(){
	SymbolEnv *env_ptr = malloc( sizeof(SymbolEnv) );

	env_ptr->scp_root_ptr = SymbolEnv_Scope_new(env_ptr, NULL, 0);
	env_ptr->scp_cur_ptr = env_ptr->scp_root_ptr;
	env_ptr->scp_last_child_ptr = NULL;

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

	while( LinkedList_peek(scp_ptr->id_lst_ptr) != NULL ){
		char *id = LinkedList_pop(scp_ptr->id_lst_ptr);
		SymbolEnv_Entry *etr_ptr = HashTable_get(scp_ptr->tbl_ptr, id);
		SymbolEnv_Entry_destroy(etr_ptr);
	}

	HashTable_destroy(scp_ptr->tbl_ptr);
	LinkedList_destroy(scp_ptr->id_lst_ptr);

	free(scp_ptr);
}

static SymbolEnv_Entry *SymbolEnv_Entry_new(SymbolEnv_Scope *scp_ptr, char *id, int len_id, int size, void *type_ptr){
	SymbolEnv_Entry *etr_ptr = malloc( sizeof(SymbolEnv_Entry) );

	etr_ptr->scp_ptr = scp_ptr;

	etr_ptr->id = malloc( sizeof(char) * (len_id+1) );
	strncpy(etr_ptr->id, id, len_id);
	etr_ptr->id[len_id] = '\0';
	etr_ptr->len_id = len_id;

	etr_ptr->size = size;
	etr_ptr->type_ptr = type_ptr;

	return etr_ptr;
}

static void SymbolEnv_Entry_destroy(SymbolEnv_Entry *etr_ptr){
	free(etr_ptr->id);
	if(etr_ptr->type_ptr != NULL)
		SymbolEnv_Type_destroy(etr_ptr->type_ptr);

	free(etr_ptr);
}


///////////
// Scope //
///////////

SymbolEnv_Scope *SymbolEnv_scope_add(SymbolEnv *env_ptr, char *name, int len_name){
	// printf("SymbolEnv_scope_add : %*s\n", len_name, name);
	SymbolEnv_Scope *scp_ptr = SymbolEnv_Scope_new(env_ptr, name, len_name);

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

	return env_ptr->scp_cur_ptr->parent;
}

SymbolEnv_Scope *SymbolEnv_scope_reset(SymbolEnv *env_ptr){
	env_ptr->scp_cur_ptr = env_ptr->scp_root_ptr;
	env_ptr->scp_last_child_ptr = NULL;

	return env_ptr->scp_cur_ptr;
}

SymbolEnv_Scope *SymbolEnv_scope_get_current(SymbolEnv *env_ptr){
	return env_ptr->scp_cur_ptr->parent;
}

SymbolEnv_Scope *SymbolEnv_scope_set_explicit(SymbolEnv *env_ptr, SymbolEnv_Scope *scp_ptr){
	if(scp_ptr->env_ptr != env_ptr)
		return NULL;

	env_ptr->scp_cur_ptr = scp_ptr;
	env_ptr->scp_last_child_ptr = NULL;
}


/////////////
// Entries //
/////////////

SymbolEnv_Entry *SymbolEnv_entry_add(SymbolEnv *env_ptr, char *id, int len_id, int size, void *type_ptr){
	SymbolEnv_Scope *scp_ptr = env_ptr->scp_cur_ptr;
	SymbolEnv_Entry *etr_ptr = SymbolEnv_Entry_new(scp_ptr, id, len_id, size, type_ptr);

	if( HashTable_get(scp_ptr->tbl_ptr, etr_ptr->id) != NULL){
		// Symbol already exists in current scope
		SymbolEnv_Entry_destroy(etr_ptr);
		return NULL;
	}

	LinkedList_push(scp_ptr->id_lst_ptr, etr_ptr->id);
	HashTable_add(scp_ptr->tbl_ptr, etr_ptr->id, etr_ptr);
}

SymbolEnv_Entry *SymbolEnv_entry_get_by_id(SymbolEnv *env_ptr, char *id, int len_id){

}

char *SymbolEnv_Entry_get_id(SymbolEnv_Entry *etr_ptr){

}

int SymbolEnv_Entry_get_size(SymbolEnv_Entry *etr_ptr){

}

int SymbolEnv_Entry_set_size(SymbolEnv_Entry *etr_ptr){

}

void *SymbolEnv_Entry_get_type(SymbolEnv_Entry *etr_ptr){

}

SymbolEnv_Scope *SymbolEnv_Entry_get_scope(SymbolEnv_Entry *etr_ptr){

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
