#include <stdio.h>
#include <libxml/parser.h>

#define CHUNKSIZE 102400

/* 2 dummy handler, just to have some */

static void 
tDOMstartElement(
    void         *userData, 
    const xmlChar   *name, 
    const xmlChar  **atts
)
{
}

static void 
tDOMendElement (
    void        *userData, 
    const xmlChar  *name
)       
{
}

xmlSAXHandler SAXHandlerStruct = {
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    tDOMstartElement,
    tDOMendElement,
    NULL,
    NULL,
    NULL,
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL, 
    NULL 
};

xmlSAXHandlerPtr SAXHandler = &SAXHandlerStruct;

void parseAndPrintFile(char *filename) {
    int res;
    FILE *f;
    char chars[CHUNKSIZE];

    f = fopen(filename, "r");
    if (f != NULL) {
        xmlParserCtxtPtr ctxt;

        res = fread(chars, 1, 4, f);
        if (res > 0) {
            ctxt = xmlCreatePushParserCtxt(SAXHandler, NULL,
                                           chars, res, filename);
            while ((res = fread(chars, 1, CHUNKSIZE, f)) > 0) {
                xmlParseChunk(ctxt, chars, res, 0);
            }
            xmlParseChunk(ctxt, chars, 0, 1);
            xmlFreeParserCtxt(ctxt);
        }
        fclose(f);
    }
}

int main(int argc, char **argv) {
    int i;
    int files = 0;

    if (argc != 2) {
        fprintf (stderr, "usage: %s <XML File>\n", argv[0]);
        return 1;
    }

    parseAndPrintFile(argv[1]);
    xmlCleanupParser();
    xmlMemoryDump();

    return(0);
}

