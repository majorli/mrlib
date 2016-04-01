#include <stdlib.h>
#include <pthread.h>

#include "mr_set.h"
#include "mr_catalogue.h"
#include "private_element.h"

#define IS_VALID_CATA(X) (X && X->container && X->type == Catalogue)

typedef struct CataNode {
	char *key;
	char *value;
	struct CataNode *parent;
	Container children;		// using Set<cata_node_t> to keep keys identity and ordered
} cata_node_t, *cata_node_p;

typedef struct {
	long root_size;
	long size;
	long changes;
	Container roots;		// using Set<cata_node_t> to keep keys identity and ordered
	pthread_mutex_t mut;
} cata_t, *cata_p;
