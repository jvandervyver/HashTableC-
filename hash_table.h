#ifndef __HASH__TABLE__H__
#define __HASH__TABLE__H__

  #include <stddef.h>

  typedef struct __hash_table_node_struct_t hash_table_node_t;

  class HashTable {
    public:
      HashTable(): HashTable(16) {};
      HashTable(const size_t initialSize);
      ~HashTable();

      void* put(const char* key, void* value);
      void* get(const char* key) const;

      unsigned int size() const;

    private:
      unsigned int elementCount;
      hash_table_node_t* nodes;
      unsigned int nodesSize;

      bool resize();
  };

#endif