#ifndef __HASH__TABLE__H__
#define __HASH__TABLE__H__

  #include <type_traits>
  #include <limits.h>
  #include <stdlib.h>
  #include <string.h>

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


/************* Implementation due to template *************/

#ifdef __GNUC__
  #ifndef likely
    #define likely(x)       __builtin_expect(!!(x), 1)
  #endif

  #ifndef unlikely
    #define unlikely(x)     __builtin_expect(!!(x), 0)
  #endif
#else
  #ifndef likely
    #define likely(x)       (x)
  #endif

  #ifndef unlikely
    #define unlikely(x)     (x)
  #endif
#endif

struct __hash_table_node_struct_t {
  void* key;
  hash_code_t hashCode;
  void* value;
  hash_table_node_t* next;
};

static hash_table_node_t* mallocNodes(const size_t size);
static hash_table_node_t* mallocNode();
static void freeNodes(hash_table_node_t* nodes, const size_t size);
static size_t calculateTableSize(size_t size);

template <typename K, typename V>
HashTable<K,V>::HashTable(
  const unsigned int initialSize,
  bool (*_equals)(K k1, K k2),
  hash_code_t (*_calculateHashCode)(K key)
) {
  static_assert(std::is_pointer<K>::value, "Key must be a pointer");
  static_assert(std::is_pointer<V>::value, "Value must be a pointer");

  this->elementCount = 0;
  this->nodes = mallocNodes(this->nodesSize = calculateTableSize(initialSize));
  this->equals = _equals;
  this->calculateHashCode = _calculateHashCode;
}

template <typename K, typename V>
HashTable<K,V>::~HashTable() {
  freeNodes(nodes, nodesSize);
}

template <typename K, typename V>
unsigned int HashTable<K,V>::size() const {
  return elementCount;
}

template <typename K, typename V>
V HashTable<K,V>::get(K key) const {
  if (unlikely(key == 0)) {
    return 0;
  }

  hash_table_node_t* node = &nodes[(nodesSize - 1) & calculateHashCode(key)];
  if (node->key == 0) {
    return 0;
  }

  do {
    if (equals((K) node->key, key)) {
      return (V) node->value;
    }

    node = node->next;
  } while (node != 0);

  return 0;
}

template <typename K, typename V>
V HashTable<K,V>::put(K key, V value) {
  if (unlikely(key == 0)) {
    return 0;
  }

  if ((((size_t) elementCount) + 1) >= ((size_t) (nodesSize * 0.75))) {
    if (unlikely(!resize())) {
      return 0;
    }
  }

  ++elementCount;
  return put(key, calculateHashCode(key), value);
}

template <typename K, typename V>
V HashTable<K,V>::put(K key, const hash_code_t hashCode, V value) {
  hash_table_node_t* node = &nodes[(nodesSize - 1) & hashCode];

  if (node->key != 0) {
    while (1) {
      if (equals((K) node->key, key)) {
        break;
      }

      hash_table_node_t* nextNode = node->next;
      if (nextNode != 0) {
        node = nextNode;
      }

      node->next = mallocNode();
      node = node->next;
      break;
    }
  }

  V currentValue = (V) node->value;
  node->value = (void*) value;

  node->key = (void*) key;
  node->hashCode = hashCode;

  return currentValue;
}

template <typename K, typename V>
bool HashTable<K,V>::resize() {
  const size_t prevNodesSize = nodesSize;
  hash_table_node_t* prevNodes = nodes;

  if (1) {
    const size_t newNodesSize = calculateTableSize(prevNodesSize << 1);
    if (unlikely(prevNodesSize >= newNodesSize)) {
      return false;
    } else {
      nodesSize = (unsigned int) newNodesSize;
    }
  }

  nodes = mallocNodes(nodesSize);

  for (size_t index = 0; index < prevNodesSize; ++index) {
    hash_table_node_t* node = &prevNodes[index];
    if (node->key == 0) {
      continue;
    }

    do {
      put((K) node->key, node->hashCode, (V) node->value);
      node = node->next;
    } while (node != 0);
  }

  freeNodes(prevNodes, prevNodesSize);
  return true;
}

static size_t calculateTableSize(size_t size) {
  if (likely(size > 1)) {
    size = size - 1;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;

    if (likely(size < ((UINT_MAX) - 1))) {
      return size + 1;
    } else {
      return ((UINT_MAX) - 1);
    }
  } else {
    return 1;
  }
}

static hash_table_node_t* mallocNodes(const size_t size) {
  const size_t byteSize = size * sizeof(struct __hash_table_node_struct_t);
  hash_table_node_t* nodes = (hash_table_node_t*) malloc(byteSize);
  memset(nodes, 0, byteSize);

  return nodes;
}

static hash_table_node_t* mallocNode() {
  const size_t byteSize = sizeof(struct __hash_table_node_struct_t);
  hash_table_node_t* node = (hash_table_node_t*) malloc(byteSize);
  memset(node, 0, byteSize);

  return node;
}

static void freeNodes(hash_table_node_t* nodes, const size_t size) {
  for (size_t index = 0; index < size; ++index) {
    hash_table_node_t* node = nodes[index].next;
    while (node != 0) {
      hash_table_node_t* nextNode = node->next;
      free(node);
      node = nextNode;
    }
  }

  free(nodes);
}

#endif