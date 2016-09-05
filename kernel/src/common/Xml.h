#ifndef XML_H
#define XML_H

#include <string>
#include <vectormap>
#include <vector>

class XmlNode {
public:
    ~XmlNode() {
        for (auto it = children.begin(); it != children.end(); ++it) {
            delete *it;
        }
    }
    rodvin::string type;
    vectormap<rodvin::string, rodvin::string> attributes;
    std::vector<XmlNode *> children;
    void write(unsigned char *buffer, size_t &len, int depth = 0);
};

XmlNode *XmlRead(const char *file, size_t length);

template <typename T>
class XmlSerializer {
public:
    static XmlNode toXml(T t);
    static T fromXml(XmlNode *text);
};

template <typename T>
XmlNode *toXml(T t) {
    return XmlSerializer<T>::toXml(t);
}
template <typename T>
T fromXml(XmlNode *value) {
    return XmlSerializer<T>::fromXml(value);
}

template <typename T>
class StringSerializer {
public:
    static rodvin::string toString(T t);
    static T fromString(const rodvin::string &str);
};

template <typename T>
rodvin::string toString(T t) {
    return StringSerializer<T>::toString(t);
}
template <typename T>
T fromString(const rodvin::string &str) {
    return StringSerializer<T>::fromString(str);
}

#endif

