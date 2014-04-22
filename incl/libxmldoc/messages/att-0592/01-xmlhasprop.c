/**
 * xmlHasProp:
 * @node:  the node
 * @name:  the attribute name
 *
 * Search an attribute associated to a node
 * This function also looks in DTD attribute declaration for #FIXED or
 * default declaration values unless DTD use has been turned off.
 *
 * Returns the attribute or the attribute declaration or NULL if 
 *         neither was found.
 */
xmlAttrPtr
xmlHasProp(xmlNodePtr node, const xmlChar *name) {
    xmlAttrPtr prop;
    xmlDocPtr doc;

    if ((node == NULL) || (name == NULL)) return(NULL);
    /*
     * Check on the properties attached to the node
     */
    prop = node->properties;
    while (prop != NULL) {
        if (!xmlStrcmp(prop->name, name))  {
	    return(prop);
        }
	prop = prop->next;
    }
    if (!xmlCheckDTD) return(NULL);

    /*
     * Check if there is a default declaration in the internal
     * or external subsets
     */
    doc =  node->doc;
    if (doc != NULL) {
        xmlAttributePtr attrDecl;
        if (doc->intSubset != NULL) {
	    attrDecl = xmlGetDtdAttrDesc(doc->intSubset, node->name, name);
	    if ((attrDecl == NULL) && (doc->extSubset != NULL))
		attrDecl = xmlGetDtdAttrDesc(doc->extSubset, node->name, name);
	    if (attrDecl != NULL)
		return((xmlAttrPtr) attrDecl);
	}
    }
    return(NULL);
}

