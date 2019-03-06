#include <iostream>
#include <iomanip>
#include <vector>
#include <set>

class TrominoGame 
{

public:
    TrominoGame( int k )
    :   m_size{1<<k} ,
        m_board(m_size,std::vector<int>(m_size,0))
    { }

    void print()
    {
        for (int i = 0; i < m_size; i++) {
            for (int j = 0; j < m_size; j++)
                std::cout<<' '<<std::setfill (' ') << std::setw(2)<<m_board[i][j];
            std::cout<<'\n';
        }
    }

    int play(int x,int y)
    {
        tromino(m_size, {x, y}, {0, 0});
    }

private:

    void putTromino( std::set <std::pair<int,int>>& ms )
    {
        for(auto it : ms) 
            m_board[it.first][it.second] = m_index;
        m_index++;
    }

    void processSize2(std::pair<int,int> xy, std::pair<int,int> xy_tile) 
    {
        for (int i = xy_tile.first; i < (2+ xy_tile.first); i++)
            for (int j = xy_tile.second; j < (2 + xy_tile.second); j++)
                if ((i != xy.first) || (j != xy.second))
                    m_board[i][j] = m_index;
        m_index++;
    }
    void tromino(int size, std::pair<int,int> xy, std::pair<int,int> xy_tile)
    {
        if (size == 2) {
            processSize2(xy,xy_tile);
            return;
        }
        size /= 2;
        int x_center = xy_tile.first + size;
        int y_center = xy_tile.second + size;

        std::vector<std::pair<int,int>>
            vec{{x_center - 1, y_center -1}, 
               {x_center - 1, y_center }, 
               {x_center, y_center - 1}, 
               {x_center, y_center}};

        std::set <std::pair<int,int>>
            ms {vec.begin(),
                vec.end()};

        std::vector<std::pair<int,int>>
            vec2{{xy_tile.first,xy_tile.second}, 
                 {xy_tile.first,xy_tile.second + size}, 
                 {xy_tile.first+size,xy_tile.second}, 
                 {xy_tile.first+size,xy_tile.second+size}};

        if (xy.first < x_center)
            if(xy.second < y_center)  
                ms.erase({x_center - 1, y_center -1});
            else 
                ms.erase({x_center - 1, y_center });
        else 
            if(xy.second < y_center)  
                ms.erase({x_center, y_center - 1});
            else 
               ms.erase({x_center,  y_center });

        putTromino(ms);

        if (xy.first < x_center)
            if (xy.second < y_center) {
                 vec[0] = {xy.first, xy.second} ; 
            }else  {
                 vec[1] = {xy.first, xy.second} ; 
            }
        else 
            if(xy.second < y_center)  {
                 vec[2] = {xy.first, xy.second} ; 
            }else {
                 vec[3] = {xy.first, xy.second} ; 
            }

        for (int i = 0 ;i<4 ;i++) 
            tromino(size, vec[i],vec2[i]);
            
    }
    int m_index {1};
    int m_size;
    std::vector<std::vector<int>>  m_board;
};
int main()
{
    TrominoGame try_game(4);
    try_game.play(3,3);
    try_game.print();
}
