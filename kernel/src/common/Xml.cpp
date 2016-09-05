#include "Xml.h"
#include <string.h>

#define Log(...)

struct XmlToken {
    enum Type { Open, Slash, Close, String, Equals, ExclMinMin, MinMinClose, EndOfFile } type;
    rodvin::string text;
    int line, chr;
    XmlToken(Type type, rodvin::string text, int line, int chr) : type(type), text(text), line(line), chr(chr) {}
    XmlToken() {}
};

void next(char c, size_t &index, int &line, int &chr) {
    ++index; 
    ++chr; 
    if (c == '\n') {
        line++; chr = 1;
    }
}

bool IsWhitespace(char c) {
    return (c == ' ' ||
            c == '\t' ||
            c == '\r' || 
            c == '\n');
}

bool IsValidString(char c) {
    return !IsWhitespace(c) &&
           c != '<' &&
           c != '>' &&
           c != '/' &&
           c != '"' &&
           c != '\'' &&
           c != '=';
}

XmlToken XmlTok(const char *file, size_t length, size_t &index, int &line, int &chr) {
    while (index < length && IsWhitespace(file[index])) { 
        next(file[index], index, line, chr);
    }
    if (index == length) return XmlToken(XmlToken::EndOfFile, "", line, chr);
    XmlToken tok;
    switch(file[index]) {
        case '<':
            if (index < length + 1 && file[index+1] == '?') {
                // Some XML tag thingum, ignore
                while (index < length + 1 && (file[index] != '?' || file[index+1] != '>')) {
                    index++;
                }
                index += 2;
                return XmlTok(file, length, index, line, chr); // re-tokenize another token.
            }
            if (index < length + 3 && file[index+1] == '!' && file[index+2] == '-' && file[index+3] == '-') {
                // Parse comment instead
                for (int i = 0; i < 4; i++) next(file[index], index, line, chr); // skip over comment intro
                while (index < length + 2 && !(file[index] == '-' && file[index+1] == '-' && file[index+2] == '>')) {
                    next(file[index], index, line, chr);    // skip over comment
                }
                for (int i = 0; i < 3; i++) next(file[index], index, line, chr); // skip over comment outro
                return XmlTok(file, length, index, line, chr); // re-tokenize another token.
            }
            tok = XmlToken(XmlToken::Open, "<", line, chr);
            next(file[index], index, line, chr);
            return tok;
        case '>':
            tok = XmlToken(XmlToken::Close, ">", line, chr);
            next(file[index], index, line, chr);
            return tok;
        case '=':
            tok = XmlToken(XmlToken::Equals, "=", line, chr);
            next(file[index], index, line, chr);
            return tok;
        case '/':
            tok = XmlToken(XmlToken::Slash, "/", line, chr);
            next(file[index], index, line, chr);
            return tok;
        default:
            int sline = line, schr = chr;
            const char *buf = &file[index];
            if (*buf == '"') {
                buf++;
                next(file[index], index, line, chr);
                while (index < length && file[index] != '"') next(file[index], index, line, chr);
                if (index == length) {
                    tok = XmlToken(XmlToken::EndOfFile, "", sline, schr);
                    return tok;
                }
                tok = XmlToken(XmlToken::String, rodvin::string(buf, &file[index]), sline, schr);
                next(file[index], index, line, chr);
                return tok;
            } else {
                while (index < length && IsValidString(file[index])) next(file[index], index, line, chr);
                return XmlToken(XmlToken::String, rodvin::string(buf, &file[index]), sline, schr);
            }
    }
}

XmlNode *XmlParse(const char *file, size_t length, size_t &index, int &line, int &chr, XmlToken &tok) {
/*    STRING <attrs> SLASH CLOSE OPEN
    STRING <attrs> CLOSE OPEN nesting SLASH STRING CLOSE OPEN
    EXCLMINMIN somethingorother MINMINCLOSE OPEN
    */
    if (tok.type != XmlToken::String) { Log("%d:%d: Expected string after <", line, chr); return NULL; }
    XmlNode *node = new XmlNode();
    node->type = tok.text;
    XmlToken token = XmlTok(file, length, index, line, chr);
    while (token.type == XmlToken::String) {
        XmlToken name = token;
        token = XmlTok(file, length, index, line, chr);
        if (token.type == XmlToken::Equals) {
            token = XmlTok(file, length, index, line, chr);
            if (token.type != XmlToken::String) {
                 { Log("%d:%d: Expected <", line, chr); return NULL; }
            }
            node->attributes.insert(std::make_pair(name.text, token.text));
            token = XmlTok(file, length, index, line, chr);
        } else {
            node->attributes.insert(std::make_pair(name.text, "true"));
        }
    }
    if (token.type == XmlToken::Slash) {
        // do nothing special, really
    } else if (token.type == XmlToken::Close) { 
        token = XmlTok(file, length, index, line, chr);
        if (token.type != XmlToken::Open)  { Log("%d:%d: Expected <", line, chr); delete node; return NULL; }
        token = XmlTok(file, length, index, line, chr);
        while (token.type != XmlToken::Slash) {
            XmlNode *cnode = XmlParse(file, length, index, line, chr, token);
            if (!cnode) return NULL;
            node->children.push_back(cnode);
            token = XmlTok(file, length, index, line, chr);
        }
        token = XmlTok(file, length, index, line, chr);
        if (token.type != XmlToken::String) { Log("%d:%d: Expected string", line, chr); delete node; return NULL; }
        if (strcmp(token.text.c_str(), tok.text.c_str()) != 0) { 
            Log("%d:%d: Mismatching name '%s' in close tag. Matched with '%s' open tag at %d:%d", line, chr, token.text.c_str(), tok.line, tok.chr, tok.text.c_str()); 
            delete node; 
            return NULL; 
        }
    } else {
        Log("%d:%d: Expected >", line, chr); 
        delete node; 
        return NULL; 
    }
    token = XmlTok(file, length, index, line, chr);
    if (token.type != XmlToken::Close) { Log("%d:%d: Expected >", line, chr); delete node; return NULL; }
    token = XmlTok(file, length, index, line, chr);
    if (token.type != XmlToken::Open) { Log("%d:%d: Expected <", line, chr); delete node; return NULL; }
    return node;
}

XmlNode *XmlRead(const char *file, size_t length) {
    size_t index = 0;
    int line = 1, chr = 1;
    XmlNode *node = new XmlNode();
    XmlToken token = XmlTok(file, length, index, line, chr);
    if (token.type != XmlToken::Open) { Log("%d:%d: Expected <", line, chr); delete node; return NULL; }
    XmlToken openName = XmlTok(file, length, index, line, chr);
    if (openName.type != XmlToken::String) { Log("%d:%d: Expected string after <", line, chr); delete node; return NULL; }
    node->type = openName.text;
    token = XmlTok(file, length, index, line, chr);
    while (token.type == XmlToken::String) {
        XmlToken name = token;
        token = XmlTok(file, length, index, line, chr);
        if (token.type == XmlToken::Equals) {
            token = XmlTok(file, length, index, line, chr);
            if (token.type != XmlToken::String) {
                 { Log("%d:%d: Expected <", line, chr); delete node; return NULL; }
            }
            node->attributes.insert(std::make_pair(name.text, token.text));
            token = XmlTok(file, length, index, line, chr);
        } else {
            node->attributes.insert(std::make_pair(name.text, "true"));
        }
    }
    if (token.type != XmlToken::Close) { Log("%d:%d: Expected >", line, chr); delete node; return NULL; }
    token = XmlTok(file, length, index, line, chr);
    if (token.type != XmlToken::Open)  { Log("%d:%d: Expected <", line, chr); delete node; return NULL; }
    token = XmlTok(file, length, index, line, chr);
    while (token.type != XmlToken::Slash) {
        XmlNode *cnode = XmlParse(file, length, index, line, chr, token);
        if (!cnode) { 
            delete node; 
            return NULL; 
        }
        node->children.push_back(cnode);
        token = XmlTok(file, length, index, line, chr);
    }
    token = XmlTok(file, length, index, line, chr);
    if (token.type != XmlToken::String) { Log("%d:%d: Expected string", line, chr); delete node; return NULL; }
    if (strcmp(token.text.c_str(), openName.text.c_str()) != 0) { 
        Log("%d:%d: Mismatching name '%s' in close tag. Matched with '%s' open tag at %d:%d", line, chr, token.text.c_str(), openName.line, openName.chr, openName.text.c_str()); 
        delete node; 
        return NULL; 
    }
    token = XmlTok(file, length, index, line, chr);
    if (token.type != XmlToken::Close) { Log("%d:%d: Expected >", line, chr); delete node; return NULL; }
    token = XmlTok(file, length, index, line, chr);
    if (token.type != XmlToken::EndOfFile) { Log("%d:%d: Expected EOF", line, chr); delete node; return NULL; }
    return node;
}

/*
char spacebuf[50] = "                                                 ";
void XmlNode::write(unsigned char *buffer, size_t &len, int depth) {
    if (buffer) {
        sprintf((char *)buffer, "%s<%s", (spacebuf + 49 - depth), type.c_str());
        buffer += strlen((char *)buffer);
        for (std::pair<rodvin::string, rodvin::string> arg : attributes) {
            sprintf((char *)buffer, " %s=\"%s\"", arg.first.c_str(), arg.second.c_str());
            buffer += strlen((char *)buffer);
        }
        if (children.size() == 0) {
            sprintf((char *)buffer, " />\n");
            buffer += strlen((char *)buffer);
        } else {
            sprintf((char *)buffer, ">\n");
            buffer += strlen((char *)buffer);
            for (XmlNode *n : children) {
                n->write(buffer, len, depth+1);
                buffer += strlen((char *)buffer);
            }
            sprintf((char *)buffer, "%s</%s>\n", (spacebuf + 49 - depth), type.c_str());
        }
    } else {
        len += depth + type.size() + 5;
        len += attributes.size() * 4;
        if (children.size()) 
            len += depth + type.size() + 2;

        for (std::pair<rodvin::string, rodvin::string> arg : attributes) {
            len += arg.first.size();
            len += arg.second.size();
        }
        for (XmlNode *child : children) {
            child->write(buffer, len, depth+1);
        }
    }
}

*/
