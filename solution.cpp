#include <iostream>
#include <vector>
#include <stack>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <numeric> // needed for accumulate in compute_score
#include <random>  // for proper random tile spawning
#include <iterator>

using namespace std;

void write_board_csv(const vector<vector<int>>& board, bool first) {
    ios_base::openmode mode = ios::app;
    if (first) mode = ios::trunc;
    ofstream fout("game_output.csv", mode);
    if (!fout) return;
    for (int r=0;r<4;r++){
        for (int c=0;c<4;c++){
            fout<<board[r][c];
            if (!(r==3 && c==3)) fout<<",";
        }
    }
    fout<<"\n";
}

void print_board(const vector<vector<int>>& board, bool first) {
    for (const auto &row : board) {
        for (auto val : row) {
            if (val == 0) cout << ".\t";
            else cout << val << "\t";
        }
        cout << "\n";
    }
    cout << endl;
    write_board_csv(board, first);
}

// TODO: use algorithms to spawn properly
void spawn_tile(vector<vector<int>>& board) {
    // Collect empty cells
    vector<pair<int,int>> empties;
    for(int r = 0; r < 4; r++)
        for(int c = 0; c < 4; c++)
            if(board[r][c]==0)
                empties.emplace_back(r,c);

    if(empties.empty())
        return;

    // Randomly choose an empty cell 90 10
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> pick(0, static_cast<int>(empties.size()) - 1);
    std::uniform_int_distribution<int> two_or_four(1, 10);

    auto [r,c] = empties[pick(rng)];
    board[r][c] = (two_or_four(rng) == 1) ? 4 : 2;
}

// Helper: compress nonzeros toward left into a 4 long vector
static vector<int> compress_left(const vector<int>& row){
    vector<int> out;
    out.reserve(4);

    std::copy_if(row.begin(), row.end(), std::back_inserter(out), [](int n){return n != 0;});

    while(out.size() < 4)
        out.push_back(0);

    return out;
}

// Helper: merge equal adjacent tiles (assumes already compressed left)
// After merging, insert zeros to keep length 4, and ensure no doublemerge.
static vector<int> merge_left(vector<int> row){
    for(int i = 0; i < 3; i++){
        if(row[i] != 0 && row[i] == row[i + 1]){
            row[i] *= 2;
            row[i + 1] = 0;
            i++;    // skip next (prevents double merge)
        }
    }

    return compress_left(row);
}

// TODO: implement using STL algorithms
bool move_left(vector<vector<int>>& board){
    bool hasChanged = false;

    for(auto& row : board){
        vector<int> before = row;
        // compress, merge, compress
        vector<int> compressed = compress_left(before);
        vector<int> merged = merge_left(compressed);

        if(merged != before){
            hasChanged = true;
            row.swap(merged);
        }
    }

    return hasChanged;
}

bool move_right(vector<vector<int>>& board){
    bool hasChanged = false;

    for(auto& row : board){
        vector<int> before = row;

        std::reverse(row.begin(), row.end());
        vector<int> compressed = compress_left(row);
        vector<int> merged = merge_left(compressed);
        std::reverse(merged.begin(), merged.end());

        if(merged != before){
            hasChanged = true;
            row.swap(merged);
        }
        else{
            // restore if unchanged
            row.swap(before);
        }
    }

    return hasChanged;
}

bool move_up(vector<vector<int>>& board){
    bool changed = false;

    for(int c = 0; c < 4; c++){
        // extract column
        vector<int> col(4);
        for(int r = 0; r < 4; r++)
            col[r] = board[r][c];

        vector<int> before = col;
        vector<int> compressed = compress_left(col);
        vector<int> merged = merge_left(compressed);

        if(merged != before)
            changed = true;

        // write back
        for(int r = 0; r < 4; r++)
            board[r][c] = merged[r];
    }

    return changed;
}

bool move_down(vector<vector<int>>& board){
    bool changed = false;

    for(int c = 0; c < 4; c++){
        // extract column
        vector<int> col(4);
        for(int r = 0; r < 4; r++)
            col[r] = board[r][c];

        vector<int> before = col;

        // reverse, logic, reverse back
        std::reverse(col.begin(), col.end());
        vector<int> compressed = compress_left(col);
        vector<int> merged = merge_left(compressed);
        std::reverse(merged.begin(), merged.end());

        if(merged != before)
            changed = true;

        // write back
        for(int r = 0; r < 4; r++)
            board[r][c] = merged[r];
    }

    return changed;
}

// TODO: compute score by summing board values
int compute_score(const vector<vector<int>>& board) {
    int total = 0;
    for(const auto& row : board){
        total = std::accumulate(row.begin(), row.end(), total);
    }

    return total;
}

int main(){
    srand(time(nullptr));
    vector<vector<int>> board(4, vector<int>(4,0));
    spawn_tile(board);
    spawn_tile(board);

    stack<vector<vector<int>>> history;
    bool first=true;

    while(true){
        print_board(board, first);
        first=false;
        cout << "Score: " << compute_score(board) << "\n";
        cout<<"Move (w=up, a=left, s=down, d=right), u=undo, q=quit: ";
        char cmd;
        if (!(cin>>cmd)) break;
        if (cmd=='q') break;

        if (cmd=='u') {
            // TODO: Implement undo handling here using the history stack
            if(!history.empty()){
                board = history.top();
                history.pop();
            }
            print_board(board,false);
            continue;
        }

        vector<vector<int>> prev = board;
        bool moved=false;

        // Push prior state in case we move successfully (for undo)
        // We'll only keep it if a move actually changes the board.
        switch (cmd){
            case 'a': 
                moved = move_left(board);
                break;
            case 'd':
                moved = move_right(board); 
                break;
            case 'w':
                moved = move_up(board);
                break;
            case 's': 
                moved = move_down(board);
                break;
            default:
                moved = false; 
                break;
        }

        if (moved) {
            // TODO: push prev to history stack for undo
            history.push(prev);
            spawn_tile(board);
        }
    }
    return 0;
}