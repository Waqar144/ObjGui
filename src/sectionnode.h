#ifndef SECTIONNODE_H
#define SECTIONNODE_H

#include "section.h"
#include "QString"
#include "QStringList"

class SectionNode
{
public:
    SectionNode(QString sectionName, QStringList addresses, QStringList hex, QStringList ascii);
    void setNext(SectionNode *n);
    SectionNode* getNext();
    Section getSection();
private:
    SectionNode *next;
    Section section;
};

#endif // SECTIONNODE_H