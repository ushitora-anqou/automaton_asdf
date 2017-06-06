#include <iostream>
#include <array>
#include <fstream>
#include <chrono>
#include <thread>
#include <cassert>

bool getLine(std::ifstream& ifs, std::string& input, int& lineno)
{
    if(!ifs) return false;
    std::getline(ifs, input);
    lineno++;
    return !input.empty();
}

void sleepMilli(int duration)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}

inline std::string fok(const std::string& str)
{
    return str;
}

template<class... Args>
inline std::string fok(const std::string& head, Args... args)
{
    return head + fok(args...);
}

using InputCells = std::array<bool, 6>;
using OutputCells = std::array<bool, 2>;

enum {
    WIDTH = 60
};

int calcIndexHash(const InputCells& data)
{
    int index = 0;
    for(int i = 0;i < 6;i++)    index |= (data[i] << i);
    return index;
}

class ConvTable
{
private:
    std::array<OutputCells, 64> data_;

public:
    ConvTable(){}

    void add(const InputCells& in, const OutputCells& out)
    {
        data_.at(calcIndexHash(in)) = out;
    }

    const OutputCells& get(const InputCells& in)
    {
        return data_.at(calcIndexHash(in));
    }

    static ConvTable loadFromFile(const std::string& filename)
    {
        ConvTable table;

        std::ifstream ifs(filename);
        assert(ifs);
        std::string input;
        int lineno = 1;
        while(getLine(ifs, input, lineno) && input.size() == 3){
            InputCells in;
            OutputCells out;

            in[0] = input[0] == '1';
            in[1] = input[1] == '1';
            in[2] = input[2] == '1';
            assert(getLine(ifs, input, lineno) && input.size() == 3);
            in[3] = input[0] == '1';
            in[4] = input[1] == '1';
            in[5] = input[2] == '1';
            assert(getLine(ifs, input, lineno) && input.size() == 1);
            out[0] = input[0] == '1';
            assert(getLine(ifs, input, lineno) && input.size() == 1);
            lineno++;
            out[1] = input[0] == '1';

            table.add(in, out);

            getLine(ifs, input, lineno);
        }

        return std::move(table);
    }
};

std::array<bool, WIDTH * 2> loadInit(const std::string& filename)
{
    std::array<bool, WIDTH * 2> src;
    std::ifstream ifs(filename);
    std::string input[2];
    int lineno = 0;
    assert(getLine(ifs, input[0], lineno) && input[0].size() == WIDTH);
    assert(getLine(ifs, input[1], lineno) && input[1].size() == WIDTH);
    for(int j = 0;j < 2;j++)
        for(int i = 0;i < WIDTH;i++)
            src[i + j * WIDTH] = (input[j][i] == '1');
    return src;
}

int main()
{
    ConvTable convTable = ConvTable::loadFromFile("table.dat");
    std::array<bool, WIDTH * 2> now = loadInit("init.dat"), next;
    next.fill(false);

    while(true){
        for(int y = 0;y < 2;y++){
            for(int x = 0;x < WIDTH;x++)
                std::cout << (now[x + y * WIDTH] ? '~' : ' ') << " " << std::flush;
            std::cout << std::endl;
        }
        std::cout << "------------------------------------------------------------------------------------------------------------------------" << std::endl;

        for(int i = 0;i < WIDTH;i++){
            int a = i - 1, b = i, c = i + 1;
            if(a < 0)        a += WIDTH;
            if(c >= WIDTH)   c -= WIDTH;

            InputCells in;
            in[0] = now[a];
            in[1] = now[b];
            in[2] = now[c];
            in[3] = now[a + WIDTH];
            in[4] = now[b + WIDTH];
            in[5] = now[c + WIDTH];

            OutputCells out = convTable.get(in);
            next[i] = out[0];
            next[i + WIDTH] = out[1];
        }
        std::swap(now, next);
        sleepMilli(1000);
    }
}
