// -*- c++ -*-

#ifndef _FUX_H_
#define _FUX_H_

// System
#include <stdio.h>
// Tools
#include <ToolsConfig.h>
#include <NoCopy.h>

// Define this one to use the Xerces XML library
// When undefined, we default to the Expat library.
// Also check ChannelArchiver/make.cfg for the required link commands
#define FUX_XERCES

/** \ingroup Tools
 * FUX, the f.u. XML helper class.
 *
 * "f.u." could stand for
 * <UL>
 * <li> fast and lightweight
 * <li> friendly and utilitarian
 * <li> frightingly useless
 * </UL>
 * or whatever you want.
 *
 * In any case, FUX implements XML read
 * and write support.
 * The 'read' part is based on XML libraries,
 * there's a choice of using
 * <UL>
 * <li> Xerces C++, see http://xml.apache.org/index.html
 * <li> Expat, see http://www.libexpat.org.
 * </UL>
 * Since Xerces handles validation and Expat doesn't,
 * the former should be preferred.
 * You pick which one you want to use in the FUX.h header file.
 */
class FUX
{
public:
    /** Constructor. */
    FUX();
    /** Destructor. */
    ~FUX();

    /** One element in the FUX tree. */
    class Element
    {
    public:
        /** Create new, empty element.
         *  <p>
         *  When parent is given, this element is added to the parent.
         */
        Element(Element *parent, const stdString &name);

        /** Create new element with value.
         *  <p>
         *  When parent is given, this element is added to the parent.
         */
        Element(Element *parent, const stdString &name, const stdString &value);
        
        /** Create Element, using printf-style format and args for value.
         *  <p>
         *  When parent is given, this element is added to the parent.
         */
        Element(Element *parent, const char *name, const char *format, ...)
                __attribute__ ((format (printf, 4, 5)));
        
        /** Destructor. */   
        ~Element();

        /** @return Returns the element name. */
        const stdString &getName() const
        {   return name; }

        /** @return Returns the element value. */
        const stdString &getValue() const
        {   return value; }

        /** @return Returns the parent element (may be 0). */
        Element *getParent()
        {   return parent; }
        
        /** @return Returns the list of child elements. */
        stdList<Element *> &getChildren()
        {   return children; }
        
        /** Append text to value. */
        void append(const char *text, size_t len)
        {   value.append(text, len); }
        
        /** @return Returns the first child of given name or 0. */
        Element *find(const char *name);
        
    private:
        PROHIBIT_DEFAULT_COPY(Element);
        Element *parent; ///< Parent element or 0.
        stdString name;  ///< Name of this element.
        stdString value; ///< Value of this element.

        /** List of children. */
        stdList<Element *> children;
    };

    stdString DTD; ///< The DTD. Set for dump().

    /** Parse the given XML file into the FUX tree.
     *
     *  Returns root of the FUX document.
     *  @exception GenericException in case of errors.
     */
    Element *parse(const char *file_name);

    /** Clear/delete the current document. */
    void clear()
    {
        setDoc(0);
    }

    /** Set the document. */
    void setDoc(Element *doc);
    
    /** Add indentation to the file. */
    static void indent(FILE *f, int depth);

    /** Dumps the FUX document.
     *
     *  Elements with values that are pure white space are
     *  printed as empty elements. Tabs are used to indent
     *  the elements according to their hierarchical location
     *  in the document.
     */
    void dump(FILE *f);
private:
    PROHIBIT_DEFAULT_COPY(FUX);
#ifdef FUX_XERCES
    friend class FUXContentHandler;
    friend class FUXErrorHandler;
#endif
    Element *root;
    Element *current;
    bool inside_tag;
    
    static void start_tag(void *data, const char *el, const char **attr);
    static void text_handler(void *data, const char *s, int len);
    static void end_tag(void *data, const char *el);
    void dump_element(FILE *f, Element *e, int depth);
};

#endif
