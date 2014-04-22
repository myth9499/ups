#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <stdio.h>

void backtrace (const xmlNodePtr node) {
		int i, indent = 0;
		xmlNodePtr tmp = node;
		do {
				for (i=0; i<indent; i++) { printf("\t"); }
				indent++;
				printf("%s\n", tmp->name);
				tmp = tmp->parent;
		} while ((tmp != NULL) && (tmp->type == XML_ELEMENT_NODE));
}

xmlNodePtr find (const xmlNodePtr node, const char *name) {
		if (node == NULL) return NULL;
		if (strncmp((char *)node->name, name, sizeof(node->name)) == 0) { 
					backtrace(node); // successful
					return node;
	   	}
		if (node->children != NULL) find(node->children, name);
		if (node->next != NULL) find(node->next, name);
		return NULL;
}
		
int main (int argc, char *argv[]) {
		if (argc != 3) {
				printf("usage: %s <node_name> <xml_file>\n", argv[0]);
				exit(-1);
		}
		xmlDocPtr doc = xmlParseFile(argv[2]);
		xmlNodePtr node = xmlDocGetRootElement(doc);
		xmlNodePtr tmp = find(node, argv[1]);
		backtrace(tmp); // fails...
		return 0;
}

