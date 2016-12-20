// System
#include <stdarg.h>
#include <unistd.h>

// Tools
#include "AutoPtr.h"
#include "GenericException.h"
#include "FUX.h"

// XML library
#ifdef FUX_XERCES
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/URLInputSource.hpp>
XERCES_CPP_NAMESPACE_USE
#else
#include <xmlparse.h>

// Tools
#include "AutoFilePtr.h"
#endif

// TODO: Catch the name of the DTD so that
//       we can print it out in dump()?
//       Unclear how to do that in a SAX
//       environment. The Xerces entity resolver
//       sees the systemID of the <!DOCTYPE ... >
//       entry, but it's not obvious that
//       the parser is in a <!DOCTYPE> tag at that point.
//       Could also be <!ENTITY ...>.


FUX::Element::Element(FUX::Element *parent, const stdString &name)
        : parent(parent), name(name)
{
    if (parent)
        parent->children.push_back(this);
}

FUX::Element::Element(FUX::Element *parent, const stdString &name,
                      const stdString &value)
        : parent(parent), name(name), value(value)
{
    if (parent)
        parent->children.push_back(this);
}

FUX::Element::Element(Element *parent, const char *name,
                      const char *format, ...)
        : parent(parent), name(name)
{
    char buf[200];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    value = buf;    
    if (parent)
        parent->children.push_back(this);
}

FUX::Element::~Element()
{    
    stdList<Element *>::iterator c;
    for (c=children.begin(); c!=children.end(); ++c)
        delete *c;
}

FUX::Element *FUX::Element::find(const char *name)
{    
    stdList<Element *>::iterator c;
    for (c=children.begin(); c!=children.end(); ++c)
        if ((*c)->name == name)
            return *c;
    return 0;
}

FUX::FUX()
    : root(0), current(0), inside_tag(false)
{}

FUX::~FUX()
{
    clear();
}

void FUX::setDoc(Element *doc)
{
    if (root)
        delete root;
    root = doc;
    current = 0;
    inside_tag = false;
}

void FUX::start_tag(void *data, const char *el, const char **attr)
{
    FUX *me = (FUX *)data;
    try
    {
        if (me->root == 0)
            me->root = me->current = new Element(0, el);
        else
            me->current = new Element(me->current, el);
        me->inside_tag = true;
    }
    catch (...)
    {
        throw GenericException(__FILE__, __LINE__,
                               "FUX::start_tag out of memory");
    }
}

void FUX::text_handler(void *data, const char *s, int len)
{
    FUX *me = (FUX *)data;
    if (me->inside_tag)
        me->current->append(s, len);
}

void FUX::end_tag(void *data, const char *el)
{
    FUX *me = (FUX *)data;
    if (!me->current)
        throw GenericException(__FILE__, __LINE__, "FUX: malformed '%s'", el);
    me->current = me->current->getParent();
    me->inside_tag = false;
}

void FUX::indent(FILE *f, int depth)
{
    for (int i=0; i<depth; ++i)
        fprintf(f, "\t");
}

void FUX::dump(FILE *f)
{
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    if (root && DTD.length() > 0)
        fprintf(f, "<!DOCTYPE %s SYSTEM \"%s\">\n",
                root->getName().c_str(), DTD.c_str());
    dump_element(f, root, 0);
}

inline bool all_white_text(const stdString &text)
{
    const char *c = text.c_str();
    while (*c)
    {
        if (strchr(" \t\n\r", *c)==0)
            return false;
        ++c;
    }
    return true;
}

void FUX::dump_element(FILE *f, Element *e, int depth)
{
    if (!e)
        return;
    indent(f, depth);
    if (all_white_text(e->getValue()))
    {
        if (e->getChildren().empty())
        {
            fprintf(f, "<%s/>\n", e->getName().c_str());
            return;
        }
        fprintf(f, "<%s>", e->getName().c_str());
    }
    else
        fprintf(f, "<%s>%s", e->getName().c_str(), e->getValue().c_str());
    if (!e->getChildren().empty())
    {
        fprintf(f, "\n");
        stdList<Element *>::const_iterator c;
        for (c=e->getChildren().begin(); c!=e->getChildren().end(); ++c)
            dump_element(f, *c, depth+1);
        indent(f, depth);
    }
    fprintf(f, "</%s>\n", e->getName().c_str());    
}

#ifdef FUX_XERCES
// Xerces implementation ---------------------------------------------------

#if 0
class FUXEntityResolver : public EntityResolver
{
public:
    InputSource *resolveEntity(const XMLCh *const publicId,
                               const XMLCh *const systemId);
};

InputSource *FUXEntityResolver::resolveEntity(const XMLCh *const publicId,
                                              const XMLCh *const systemId)
{
    char *s;
    s = XMLString::transcode(publicId);
    printf("Entity, publicId '%s'\n", s);
    XMLString::release(&s);
    s = XMLString::transcode(systemId);
    printf("Entity, systemId '%s'\n", s);
    XMLString::release(&s);
    return 0;
}
#endif

class FUXContentHandler : public DefaultHandler
{
public:
    FUXContentHandler(FUX *fux) : fux(fux) {}
    void startElement(const XMLCh* const uri, const XMLCh* const localname,
                      const XMLCh* const qname,const Attributes& attrs);
    void characters(const XMLCh *const chars, const unsigned int length);
    void endElement(const XMLCh* const uri, const XMLCh* const localname,
                    const XMLCh* const qname);
private:
    FUX *fux;
};

void FUXContentHandler::startElement(const XMLCh* const uri,
                                     const XMLCh* const localname,
                                     const XMLCh* const name,
                                     const Attributes& attrs)
{
    char *s = XMLString::transcode(name);
    fux->start_tag(fux, s, 0);
    XMLString::release(&s);
}

void FUXContentHandler::characters(const XMLCh *const chars,
                                   const unsigned int length)
{
    char buf[500]; // TODO: Loop over chars in case length > sizeof(buf)
    int len = length;
    if (len >= (int)sizeof(buf))
        len = sizeof(buf) - 1;
    if (!XMLString::transcode(chars, buf, len))
        throw GenericException(__FILE__, __LINE__,
                               "FUXContentHandler: Transcode error");
    fux->text_handler(fux, buf, len);
}

void FUXContentHandler::endElement(const XMLCh* const uri,
                                   const XMLCh* const localname,
                                   const XMLCh* const name)
{
    char *s = XMLString::transcode(name);
    fux->end_tag(fux, s);
    XMLString::release(&s);
}

class FUXErrorHandler : public DefaultHandler
{
public:
    FUXErrorHandler(FUX *fux) : fux(fux) {}    
    void warning(const SAXParseException&);
    void error(const SAXParseException&);
    void fatalError(const SAXParseException&);
private:
    FUX *fux;
};

void FUXErrorHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    GenericException e(__FILE__, __LINE__,
                       "XML Warning, Line %zu:\n%s",
                       (size_t)exception.getLineNumber(), message);
    XMLString::release(&message);
    throw e;
}

void FUXErrorHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    GenericException e(__FILE__, __LINE__,
                       "XML Error, Line %zu:\n%s",
                       (size_t)exception.getLineNumber(), message);
    XMLString::release(&message);
    throw e;
}

void FUXErrorHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    GenericException e(__FILE__, __LINE__,
                       "XML Error (fatal), Line %zu:\n%s",
                       (size_t)exception.getLineNumber(), message);
    XMLString::release(&message);
    throw e;
}

FUX::Element *FUX::parse(const char *file_name)
{
    clear();
    if (file_name[0] == '\0' or !file_name)
       throw GenericException(__FILE__, __LINE__,
                              "FUX::parse invoked with empty filename");
    try
    {
        XMLPlatformUtils::Initialize();
        //FUXEntityResolver entity_resolver;
        FUXContentHandler content_handler(this);
        FUXErrorHandler   error_handler(this);
        AutoPtr<SAX2XMLReader> parser(XMLReaderFactory::createXMLReader());

        // Very strange notation, but these XMLUni constants
        // are simply the XMLCh strings shown in the following comments.
        // Those URLs matche the SAX2XMLReader documentation,
        // but it's very hard to find the XMLUni members
        // for the URLs without looking at XMLUni.cpp.
        // "http://xml.org/sax/features/validation"
        parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
        // "http://xml.org/sax/features/namespaces"
        parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);
        // "http://xml.org/sax/features/validation"
        parser->setFeature(XMLUni::fgXercesDynamic, true);
        // "http://apache.org/xml/features/validation-error-as-fatal"
        parser->setFeature(XMLUni::fgXercesValidationErrorAsFatal, true);
        //parser->setEntityResolver(&entity_resolver);
        parser->setContentHandler(&content_handler);
        parser->setErrorHandler(&error_handler);

        // This is nuts.
        // I would like to be able to use
        // "../some_dir/file.xml"
        // as well as
        // "http://server/dir/file.xml"
        // In the former case, all files are intepreted
        // relative to where the executable binary is installed
        // (e.g. $EPICS_EXTENSIONS/bin/linux/ArchiveExport)
        // on some systems (OS X/Darwin) while on Linux,
        // it's relative to where we _invoke_ the executable.
        // So in here we preprend the cwd to make it work the
        // same in both cases.
        if (strchr(file_name, ':')  ||  // Assume it starts with file: or http: ...
            file_name[0] == '/')        // It's an absolute path
            parser->parse(file_name);
        else
        {
            char path[1023];
            getcwd(path, sizeof(path)-1);
            strncat(path, "/", sizeof(path)-1);
            strncat(path, file_name, sizeof(path)-1);
            parser->parse(path);
        }
        parser = 0;
        XMLPlatformUtils::Terminate();
    }
    catch (GenericException &e)
    {
        clear();
        XMLPlatformUtils::Terminate();
        throw GenericException(__FILE__, __LINE__,
                               "Error parsing '%s'\n%s", file_name, e.what());
    }
    catch (const XMLException &toCatch)
    {
        clear();
        char* message = XMLString::transcode(toCatch.getMessage());
        GenericException e(__FILE__, __LINE__,
                           "Xerces exception: %s", message);
        XMLString::release(&message);
        XMLPlatformUtils::Terminate();
        throw e;
    }
    catch (const SAXParseException &toCatch)
    {
        clear();
        char* message = XMLString::transcode(toCatch.getMessage());
        GenericException e(__FILE__, __LINE__,
                           "Xerces exception: %s", message);
        XMLString::release(&message);
        XMLPlatformUtils::Terminate();
        throw e;
    }
    catch (...)
    {
        clear();
        throw GenericException(__FILE__, __LINE__, "Unkown Xerces error");
    }  
    return root;
}
// End of Xerces implementation --------------------------------------------
#else

class AutoXML_Parser
{
public:
    AutoXML_Parser()
    {
        p =  XML_ParserCreate(NULL);
        if (! p)
            throw GenericException(__FILE__, __LINE__,
                               "Couldn't allocate memory for parser\n");
    }

    ~AutoXML_Parser()
    {
        XML_ParserFree(p);
        p = 0;
    }

    operator XML_Parser () const
    {
        return p;
    }
private:
    XML_Parser p;
};

// Expat implementation ----------------------------------------------------
FUX::Element *FUX::parse(const char *file_name)
{
    clear();
    bool done = false;
    AutoFilePtr f(file_name, "rt");
    if (!f)
        throw GenericException(__FILE__, __LINE__,
                               "Cannot open '%s'", file_name);
    AutoXML_Parser p;
    XML_SetUserData(p, this);
    XML_SetElementHandler(p, start_tag, end_tag);
    XML_SetCharacterDataHandler(p, text_handler);
    while (!done)
    {
        void *buf = XML_GetBuffer(p, 1000);
        if (!buf)
            throw GenericException(__FILE__, __LINE__, "FUX: No buffer");
        int len = fread(buf, 1, 1000, f);
        if (ferror(f))
            throw GenericException(__FILE__, __LINE__, "FUX: Read error");
        done = feof(f);
        if (! XML_ParseBuffer(p, len, done))
            throw GenericException(__FILE__, __LINE__, 
                                   "FUX: Error at line %d of '%s': %s\n",
                                   XML_GetCurrentLineNumber(p),
                                   file_name,
                                   XML_ErrorString(XML_GetErrorCode(p)));
    }
    return root;
}
// End of Expat implementation ----------------------------------------------
#endif
