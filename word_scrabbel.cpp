
// C++ program to print all valid words that
// are possible using character of array
#include <iostream>
#include <iomanip>
#include <numeric>
#include <cstring>
#include <tuple>
#include <memory>
#include <set>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <iterator>

using namespace std;

// trie Node
struct TrieNode
{
    bool isWord {false};
    vector<TrieNode*> children;
    TrieNode()
    : children(vector<TrieNode*>(26, nullptr))
    { }   
};

class Trie
{
public:
    TrieNode* getRoot(){return root;}
    Trie(vector<string>& words)
    {
        root=new TrieNode();
        for(auto &word : words)
            addWord(word);
    }
    ~Trie() 
    {
        delTree(root);
        delete root ;
    }

    void delTree( TrieNode *root )
    {
        for ( int i = 0; i < 26; ++i ) {
            if ( root->children[i] ) {
                delTree( root->children[i] );
            }
        }
    }
    void addWord(const string& word)
    {
        if(word.empty()) return;
        auto cur = root;
        for(auto c : word) {
            int i = c -'a';
            if(!cur->children[i]) cur->children[i]=new TrieNode();
            cur=cur->children[i];    
        }
        cur->isWord=true;
    }
    TrieNode* searchWord(string prefix) const
    {
        auto curr = root;
        for (auto ch : prefix) {
            curr = curr->children[ch-'a'];
            if (!curr ) return nullptr;
        }
        return curr;
    }
    bool isWord(string word)
    {
       auto trie = searchWord(word);
       return trie ? trie->isWord : false;
    }
    bool isLastNode(TrieNode* root) 
    { 
        for (int i = 0; i < 26; i++) 
            if (root->children[i]) 
                return 0; 
        return 1; 
    } 
    void getDepth(TrieNode* root, string& res,vector<string> &tres)
    {
        if(isLastNode(root)) {
            res.clear();
            return ;
        }
        for (int i = 0; i < root->children.size(); i++) {
            if (root->children[i]) {
                res.push_back('a'+i);
                if(root->children[i]->isWord ) {
                    tres.emplace_back(res);
                }
                getDepth(root->children[i],res,tres);
            }
        }
    }

private:
    TrieNode* root;
};

struct letterValues
{
    static const int points[];
    int operator()(char ch) 
    {
        return points[tolower(ch)-'a'] ;
    }
};
const int letterValues::points[] = {
        1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 
        1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10
    };

struct letterScore
{
    int operator()(string const& word)
    {
        return std::accumulate(
            word.cbegin(), 
            word.cend(), 
            0, 
            [](int total, char a){
                return total + letterValues()(a); }
        );
    }
};

struct WordsLookup
{
    vector<string>& words;
    WordsLookup (vector<string>& inDict)
       : words(inDict)
    { }
    vector<string> startsWith(char p, size_t maxLength)
    {
         vector<string> res;
         copy_if(words.begin(),
                 words.end(), 
                 back_inserter(res),
                 [&](auto& word) {
                 return word.front() == p && word.size() <= maxLength;});
         return res;
    }
    vector<string> endsWith(char p, size_t maxLength)
    {
         vector<string> res;
         copy_if(words.begin(),
                 words.end(), 
                 back_inserter(res),
                 [&](auto& word) {
                 return word.back() == p && word.size() <= maxLength;});
         return res;
    }
    vector<pair<string,int>> contains(string chs, size_t maxLength)
    {
        vector<string> res;
        copy_if(words.begin(),
                words.end(), 
                back_inserter(res),
                [&](auto& word) {
                    unordered_set<char> se(word.begin(), word.end());
                    return std::all_of(
                        chs.begin(), 
                        chs.end(),
                        [&se](auto x) { return se.count(x)==1; }) && 
                        word.size() <= maxLength;});

        vector<pair<string,int>> w_res;
        for(auto it : res) w_res.push_back({it, letterScore()(it)});
        return w_res;
    }
};

struct Found
{
    int xpos;
    int ypos;
    string  word;
    bool vertical;
    int  weight;
    friend ostream& operator<<(ostream&os , Found const& data) 
    {
        return os<<'('<<
               data.xpos<<','<<
               data.ypos<<','<<
               std::boolalpha<<
               !data.vertical<<','<<
               data.word<<')';
    }
};

class ScrabbleBoard 
{
public:
    friend class ScrabblePlayer;

    ScrabbleBoard(string const& fname, 
                  Trie& trie) 
      :  dict_root {trie},
                    in_fname{fname}
    { 
        loadBoardData();
        word = board_data.back();
        board_data.pop_back();
        findWord(word,"",trie.getRoot());
        out_fname = fname.substr(0, fname.find_last_of('.')) + ".answer";
    }
    string outFile() const
    {
       return out_fname;
    }

    vector<Found> currentWordsH()
    {
        for (int i = 0; i < board_size; i++)
        {
            string word = "";
            for (int j = 0; j < board_size; j++)
            {
                if (board_matrix[i][j] == '-')
                {
                    // assuming that smallest word is 2 chars
                    if (word.length() > 1 && dict_root.searchWord(word))  
                        words_h.push_back({i,j,word,false,0});
                    word = "";
                }
                else {
                    word += board_matrix[i][j];
                }
                vertical = j;
            }
            if (word.length() > 1 && dict_root.searchWord(word)) 
                words_h.push_back({i,vertical,word,false,0});
        }
        return words_h;
    }
    
    vector<Found> currentWordsV()
    {
        for (int i = 0; i < board_size; i++)
        {
            string word = "";
            for (int j = 0; j < board_size; j++)
            {
                if (board_matrix[j][i] == '-')
                {
                    // assuming that smallest word is 2 chars
                    if (word.length() > 1 && dict_root.searchWord(word))
                        words_v.push_back({j,i,word,true,0});
                    word = "";
                }
                else {
                    word += board_matrix[j][i];
                }
                vertical = j;
            }
            if (word.length() > 1  && dict_root.searchWord(word))
                words_v.push_back({vertical,i,word,true,0});
        }
        return words_v;
    }
    
    vector<Found> currentWords()
    {
        auto result = currentWordsH();
        currentWordsV();
        result.insert(result.end(), words_v.begin(), words_v.end());
        return result;
    }
    
    void loadBoardData()
    {
        ifstream f{in_fname};
        if (!f) {
            std::cerr << "Error opening " << in_fname << ": "
                  << strerror(errno) << std::endl;
            exit(1);
        }

        board_data = vector<string> {istream_iterator<string>{f},{}};
        for (int i = 0; i<board_size; i++)
        {
            const char *line = board_data[i].c_str();
            for (int j = 0; j<board_size; j++) 
            {
                board_matrix[i][j] = line[j];
            }
        }
    }
    
    void findWord (string input, string currentWord, TrieNode* node) 
    {
        if (node->isWord)  word_list.push_back(currentWord);
    
        for( int i = 0; i < input.length(); i++ ) {
          auto curr = node->children[input[i]-'a'] ;
          if(curr) {
              findWord( string(input).erase(i,1), currentWord + input[i], curr);
          }
        }
    }

    string player_tiles()
    {
        return word;
    }
private:
    vector<string> board_data;
    vector<Found> words_h;
    vector<Found> words_v;
    static constexpr int board_size{15};
    char board_matrix[board_size][board_size];
    Trie& dict_root;
    int vertical;
    string word;
    string in_fname;
    string out_fname;
    vector<string> word_list;
};

class checkWords
{
public:
    checkWords(string input) 
    {
        for (char ch : input) table[int(ch)] = true;
    }
    bool operator()(unsigned char ch) { return table[ch]; }

private:
    bool table[std::numeric_limits<unsigned char>::max()] {false};
};
class ScrabblePlayer
{
    ScrabbleBoard& board;
    Trie& dict_root;
    string tiles;

public:    

    ScrabblePlayer (ScrabbleBoard& b, Trie& trie) 
    : board(b),dict_root(trie),tiles(b.player_tiles())
    { 
    }
    
    bool generateCrossWords(Found& entry, vector<string>&res, Found& found)
    {
        vector<Found> final_results;
        WordsLookup  suggestions(board.word_list);
        for(auto it : res) {
           if(it.size()>1) continue; //the current implementation supports suggestion with 1 chars only
           auto p = getOppositeSpace(entry,0);
           auto words_sug = suggestions.contains(it,p.second-p.first);
           auto target =  max_element(
                words_sug.begin(), 
                words_sug.end(), 
                [](const auto &lhs,const auto &rhs){
                    return rhs.second > lhs.second or 
                           (rhs.second ==  lhs.second &&  
                           lhs.first.size() > rhs.first.size());});
           
           size_t adj = target->first.find_last_of(it) ; //check if we need to shift the new word

           int pos1 = entry.vertical ? entry.xpos : entry.ypos;
           int pos2 = entry.vertical ? entry.ypos : entry.xpos;

           pos2 -= adj  ;
           final_results.push_back({pos1,pos2,target->first,entry.vertical,target->second});
        }
        auto target =  max_element(
             final_results.begin(), 
             final_results.end(), 
             [](const auto &lhs,const auto &rhs){
                 return rhs.weight > lhs.weight or 
                     (rhs.weight ==  lhs.weight &&  
                     lhs.word.size() > rhs.word.size());});
        if(final_results.size()) found = *target;
        return final_results.size();
    }
    pair<int,int> getOppositeSpace(Found& entry, int i)
    {
        int l_cnt {0}; //left or top counter
        int r_cnt {0}; //right or down counter

        if(entry.vertical) {
            l_cnt  = entry.ypos + i ;
            r_cnt  = entry.ypos + i;

            while(l_cnt > 0 && board.board_matrix[entry.xpos][l_cnt]  == '-' )  --l_cnt;
            while(r_cnt < 14 && board.board_matrix[entry.xpos][r_cnt] == '-')   ++r_cnt;
        }else {
            l_cnt  = entry.xpos + i ;
            r_cnt  = entry.xpos + i;
            while(l_cnt > 0  && board.board_matrix[l_cnt][entry.ypos] == '-' )  --l_cnt;
            while(r_cnt < 14  && board.board_matrix[r_cnt][entry.ypos] == '-' ) ++r_cnt;
        }
        if(l_cnt) l_cnt-=2;   //the algorithm dosn't join words, ensure 1 position as seperations
        if(r_cnt) r_cnt-=2;
        return {l_cnt,r_cnt};
    }
    bool generateSuggestions(Found& entry, int space,vector<string>&res) 
    {
        auto p1 = dict_root.searchWord(entry.word);

        if(p1 && p1->isWord && !dict_root.isLastNode(p1)) {
            vector<string> result;
            string s;
            dict_root.getDepth(p1, s, result);
            checkWords iv{board.player_tiles()};
            copy_if(result.begin(),
                result.end(),
                back_inserter(res),
                [&,this](auto &data) {
                    return std::all_of(
                    data.begin(), 
                    data.end(),
                    [&iv](auto x) { return iv(x);}
                    ) && int(data.size()) <= space && dict_root.isWord(entry.word+data);});

            return !res.empty();
        }
        return false;
    }
    bool takeTurn(Found& found) 
    {
        for (auto& entry: board.currentWords()) 
        {
            int boundery = entry.vertical ? entry.xpos : entry.ypos;

            if (boundery == board.board_size -1) continue; //edge word

            int remaining_space = board.board_size - boundery;

            vector<string> word_list;
            if(!generateSuggestions(entry ,remaining_space,word_list)) continue; //no match

            return generateCrossWords(entry,word_list, found);
        }
        return false;
    }
};

//Driver program to test above function
int main(int argc, char *argv[])
{                      
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <infile>\n";
        return EXIT_FAILURE;
    }

    string dict_file{"dict.txt"};
    ifstream f{dict_file};
    if (!f) {
        std::cerr << "Error opening " << dict_file << ": "
                  << strerror(errno) << std::endl;
        return 1;
    }
    vector<string> dict{istream_iterator<string>{f},{}};

    Trie root (dict);
    
    ScrabbleBoard board (argv[1],root);

    ScrabblePlayer player1(board, root);

    Found found;

    bool result = player1.takeTurn(found);

     std::ofstream outfile(board.outFile());
    if(result) {
         outfile<<found<<'\n';
    }
    else 
       outfile<<"no more turns"<<endl;
}
