#ifndef INCLUDE_GUARD_1A368492455A426692606488F5BD21F0
#define INCLUDE_GUARD_1A368492455A426692606488F5BD21F0


/////////////////////
// Data Structures //
/////////////////////

/**
 * Opaque struct to hold the tree of symbol table and other information
 */
typedef struct SymbolEnv SymbolEnv;

/**
 * Opaque struct holding the symbol table for its scope
 */
typedef struct SymbolEnv_Scope SymbolEnv_Scope;

/**
 * Opaque struct holding information about a symbol
 */
typedef struct SymbolEnv_Entry SymbolEnv_Entry;


//////////////////////////////////
// Constructors and Destructors //
//////////////////////////////////

/**
 * Allocates space for and returns a pointer to a struct holding an empty tree
 * of symbol tables. The scope is the home scope
 * @return Pointer to struct
 */
SymbolEnv *SymbolEnv_new();

/**
 * Deallocates all internally allocated memory along with freeing all
 * SymbolEnv_Type structs
 * @param env_ptr Pointer to struct
 */
void SymbolEnv_destroy(SymbolEnv *env_ptr);

/**
 * User defined function to deallocate a SymbolEnv_Type struct
 * @param type_ptr Pointer to SymbolEnv_Type struct
 */
void SymbolEnv_Type_destroy(void *type_ptr);

///////////
// Scope //
///////////

/**
 * Creates a child scope and sets the enviroment scope to the new scope
 * @param  env_ptr  Pointer to SymbolEnv struct
 * @param  name     A reference name for the scope. Need not be unique
 * @param  len_name Length of the string
 * @return          Pointer to newly created scope for reference
 */
SymbolEnv_Scope *SymbolEnv_scope_add(SymbolEnv *env_ptr, char *name, int len_name);

/**
 * Changes the environment scope to the parent, if it exists
 * @param  env_ptr Pointer to SymbolEnv struct
 * @return         Pointer to the parent scope, NULL if no parent
 */
SymbolEnv_Scope *SymbolEnv_scope_exit(SymbolEnv *env_ptr);

/**
 * Sets the environment scope to home scope.
 * @param  env_ptr Pointer to SymbolEnv struct
 * @return         Pointer to home scope
 */
SymbolEnv_Scope *SymbolEnv_scope_reset(SymbolEnv *env_ptr);

/**
 * Fetch current environment's scope
 * @param  env_ptr Pointer to SymbolEnv struct
 * @return         POinter to current scope
 */
SymbolEnv_Scope *SymbolEnv_scope_get_current(SymbolEnv *env_ptr);

/**
 * Sets the environment's scope to the next scope which will be visited in
 * triple order traversal. The scope to be set is an unvisited child, or if none
 * exists, the parent, or if parent does not exist, stops. When scope is reset
 * or set explicitly, all children all considered unvisited
 * @param  env_ptr Pointer to SymbolEnv struct
 * @return         Pointer to the next scope which is set, NULL if none exists
 */
SymbolEnv_Scope *SymbolEnv_scope_set_dfs(SymbolEnv *env_ptr);

/**
 * Sets the environment's scope to the scope whose reference pointer is given.
 * All sub scopes are now considered unvisited for dfs traversal
 * @param  env_ptr Pointer to SymbolEnv struct
 * @param  scp_ptr Pointer to the scope which should be set
 * @return         Pointer to the set scope, or NULL if the scope is not valid
 */
SymbolEnv_Scope *SymbolEnv_scope_set_explicit(SymbolEnv *env_ptr, SymbolEnv_Scope *scp_ptr);


/////////////
// Entries //
/////////////

/**
 * Adds a symbol table table entry to the current scope of the environment.
 * @param  env_ptr  Pointer to SymbolEnv struct
 * @param  id       Pointer to the identifier string
 * @param  len_id   Length of string
 * @param  size     Size in bytes of memory to be reserved for the symbol. Can
 * be changed later if it is set to 0
 * @param  type_ptr Pointer to a user allocated SymbolEnv_Type struct for the
 * symbol. The struct will be deallocated by the user defined destructor for the
 * void    struct
 * @return          Returns a pointer to entry created, NULL if an
 * entry with same id already exists.
 */
SymbolEnv_Entry *SymbolEnv_entry_add(SymbolEnv *env_ptr, char *id, int len_id, int size, void *type_ptr);

/**
 * Fetch an entry by it's identifier
 * @param  env_ptr Pointer to SymbolEnv struct
 * @param  id      Pointer to identifier string 
 * @param  len_id  Length of string
 * @return         Returns a pointer to entry if it exists, NULL other wise
 */
SymbolEnv_Entry *SymbolEnv_entry_get_by_id(SymbolEnv *env_ptr, char *id, int len_id);

char *SymbolEnv_Entry_get_id(SymbolEnv_Entry *etr_ptr);

int SymbolEnv_Entry_get_size(SymbolEnv_Entry *etr_ptr);

/**
 * Set the size in bytes to be allocated from the memory for the symbol
 * @param  etr_ptr Pointer to the symbol's entry
 * @return         0 on success. -1 on failure to set size, as it has already
 * been set before
 */
int SymbolEnv_Entry_set_size(SymbolEnv_Entry *etr_ptr);

void *SymbolEnv_Entry_get_type(SymbolEnv_Entry *etr_ptr);

SymbolEnv_Scope *SymbolEnv_Entry_get_scope(SymbolEnv_Entry *etr_ptr);


#endif
