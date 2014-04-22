#include <stdio.h>
#include <libxml/parser.h>


int main(int argc, char **argv)
{
    xmlDocPtr doc;
    char buf[1024];
    FILE *f;
    size_t l;

    printf("- From file\n");
    doc = xmlParseFile(argv[1]);
    printf("encoding: %s\ncharset: %d\n",
           doc->encoding, doc->charset);
    xmlFreeDoc(doc);

    f = fopen(argv[1], "r");
    l = fread(buf, 1, 1024, f);
    
    printf("- From memory\n");
    doc = xmlParseMemory(buf, l);
    printf("encoding: %s\ncharset: %d\n",
           doc->encoding, doc->charset);
    xmlFreeDoc(doc);

    return 0;
}

