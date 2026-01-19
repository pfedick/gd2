#ifndef INCLUDE_LEVELDESCRIPTION_H_
#define INCLUDE_LEVELDESCRIPTION_H_
#include <map>
#include <ppl7.h>

class LevelDescription
{
  public:
    bool partOfStory;
    bool visibleInLevelSelection;
    int levelSort;
    std::map<ppl7::String, ppl7::String> LevelName;
    std::map<ppl7::String, ppl7::String> Description;
    ppl7::String Author;
    ppl7::ByteArray Thumbnail;

    ppl7::String Filename; // only for Level selection
    bool isCustomLevel;    // only for Level selection

    LevelDescription();
    void clear();
    bool loadFromFile(const ppl7::String &filename);
    void loadFromAssocArray(const ppl7::AssocArray &a);
};

void getLevelList(std::list<LevelDescription> &level_list);

#endif