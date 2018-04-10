#include <stdlib.h>

#include "HashTable.h"

#include "SymbolEnv.h"


/////////////////////
// Data Structures //
/////////////////////

typedef struct SymbolEnv {

}SymbolEnv;

typedef struct SymbolEnv_Scope {

}SymbolEnv_Scope;

typedef struct SymbolEnv_Entry {

}SymbolEnv_Entry;

typedef struct SymbolEnv_Type {

}SymbolEnv_Type;


////////////////////
// Pvt Prototypes //
////////////////////

static SymbolEnv_Scope* SymbolEnv_Scope_new();

static void SymbolEnv_Scope_destroy(SymbolEnv_Scope *scp_ptr);

static SymbolEnv_Entry *SymbolEnv_Entry_new();

static void SymbolEnv_Entry_destroy(SymbolEnv_Entry *etr_ptr);


//////////////////////////////////
// Constructors and Destructors //
//////////////////////////////////

SymbolEnv *SymbolEnv_new(){

}

void SymbolEnv_destroy(SymbolEnv *env_ptr){

}

void SymbolEnv_Type_destroy(SymbolEnv_Type *type_ptr){

}

static SymbolEnv_Scope* SymbolEnv_Scope_new(){
	
}

static void SymbolEnv_Scope_destroy(SymbolEnv_Scope *scp_ptr){
	
}

static SymbolEnv_Entry *SymbolEnv_Entry_new(){
	
}

static void SymbolEnv_Entry_destroy(SymbolEnv_Entry *etr_ptr){
	
}


///////////
// Scope //
///////////

SymbolEnv_Scope *SymbolEnv_scope_add(SymbolEnv *env_ptr, char *name, int len_name){

}

SymbolEnv_Scope *SymbolEnv_scope_exit(SymbolEnv *env_ptr){

}

SymbolEnv_Scope *SymbolEnv_scope_reset(SymbolEnv *env_ptr){

}

SymbolEnv_Scope *SymbolEnv_scope_get_current(SymbolEnv *env_ptr){

}

SymbolEnv_Scope *SymbolEnv_scope_set_dfs(SymbolEnv *env_ptr){

}

SymbolEnv_Scope *SymbolEnv_scope_set_explicit(SymbolEnv *env_ptr, SymbolEnv_Scope *scp_ptr){

}


/////////////
// Entries //
/////////////

SymbolEnv_Entry *SymbolEnv_entry_add(SymbolEnv *env_ptr, char *id, int len_id, int size, SymbolEnv_Type *type_ptr){

}

SymbolEnv_Entry *SymbolEnv_entry_get_by_id(SymbolEnv *env_ptr, char *id, int len_id){

}

char *SymbolEnv_Entry_get_id(SymbolEnv_Entry *etr_ptr){

}

int SymbolEnv_Entry_get_size(SymbolEnv_Entry *etr_ptr){

}

int SymbolEnv_Entry_set_size(SymbolEnv_Entry *etr_ptr){

}

SymbolEnv_Type *SymbolEnv_Entry_get_type(SymbolEnv_Entry *etr_ptr){

}

SymbolEnv_Scope *SymbolEnv_Entry_get_scope(SymbolEnv_Entry *etr_ptr){

}
