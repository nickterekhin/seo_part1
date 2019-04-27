#include "search_server.h"
#include "iterator_range.h"
#include "profile.h"
#include "stat_profile.h"
#include "parse.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <future>
#include <map>

deque<string> SplitIntoWords(const string &line) {
    istringstream words_input(line);
    return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream& document_input) {
     UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {

       InvertedIndex new_index;
    for (string current_document; getline(document_input, current_document); ) {
        new_index.Add(move(current_document));
    }

    index = move(new_index);
}
void SearchServer::AddQueriesStream(istream &query_input, ostream &search_results_output) {

    Profiler profiler;

    vector<set<DocItem>::iterator> _existing_docs_items(50000);
    set<DocItem> _docsItems;

    for(string current_query; getline(query_input, current_query); )
    {
        _docsItems.clear();
        fill(_existing_docs_items.begin(),_existing_docs_items.end(),_docsItems.end());


         const auto  words = SplitIntoWords(current_query);
        for (const auto &word : words) {


            for (const auto &item : index.Lookup(word)) {

                if(_existing_docs_items[item._id]!=_docsItems.end())
                {
                        auto node = _docsItems.extract(_existing_docs_items[item._id]);
                        node.value()._hits+=item._hits;
                        auto it = _docsItems.insert(move(node.value()));
                        _existing_docs_items[item._id]= it.first;
                }else
                {
                    auto it = _docsItems.insert(item);
                    _existing_docs_items[item._id] = it.first;
                }
            }
        }

        search_results_output << current_query << ':';

        for (auto &[_id,_hits] : HeadEx(_docsItems,5)) {

            search_results_output << " {"
                                  << "docid: " << _id << ", "
                                  << "hitcount: " << _hits << '}';
        }

        search_results_output << endl;

    }
}

void InvertedIndex::Add(const string& document) {

    const size_t docid = docs_id++;

    map<string_view, size_t*> words;
    for (auto& word : SplitIntoWords(document)) {
        auto it = words.find(word);
        if (it == words.end())
        {
            auto& point = index[word];
            point.push_back({docid, 1});
            words[word] = &point[point.size() - 1]._hits;
        }
        else
        {
            *(it->second) = *(it->second) + 1;
        }
    }
}

deque<DocItem> InvertedIndex::Lookup(const string &word) const {

    if (auto it = index.find(word); it != index.end()) {
        return it->second;
    } else {
        return {};
    }
}
