#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <deque>

using namespace std;

struct DocItem{
    size_t _id;
    mutable  size_t _hits;
    bool operator<(const DocItem & other) const
    {

        if(_hits>other._hits)
            return true;
        else if(_hits==other._hits)
            return _id<other._id;

        return false;
    }
    void operator +=(const DocItem & other) {
        _id=other._id;
        _hits+=other._hits;
    }
};

class InvertedIndex {
public:
    InvertedIndex() = default;
    void Add(const string &document);

    deque<DocItem> Lookup(const string &word) const;

private:
    map<string, deque<DocItem>> index;
    size_t docs_id=0;
};

class SearchServer {
public:
  SearchServer()=default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
  InvertedIndex index;
};
