#ifndef __HASH__TABLE__H__
#define __HASH__TABLE__H__

  typedef unsigned int hash_code_t;
  typedef struct __hash_table_node_struct_t hash_table_node_t;

  template <typename K, typename V>
  class HashTable {
    public:
      HashTable(const unsigned int initialSize, bool (*_equals)(K key, K input), hash_code_t (*_calculateHashCode)(K key));
      HashTable(bool (*_equals)(K key, K input), hash_code_t (*_calculateHashCode)(K key)): HashTable(16, _equals, _calculateHashCode) {};
      ~HashTable();

      V put(K key, V value);
      V get(K key) const;

      unsigned int size() const;

    private:
      bool (*equals)(K key, K input);
      hash_code_t (*calculateHashCode)(K key);

      unsigned int elementCount;
      hash_table_node_t* nodes;
      unsigned int nodesSize;

      V put(K key, const hash_code_t hashCode, V value);
      bool resize();
  };

#endif